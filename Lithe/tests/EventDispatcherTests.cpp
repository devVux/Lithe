#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <chrono>

#include <Lithe/Events/EventDispatcher.h>

struct TestEvent: public Lithe::Event {
	TestEvent(int value): value(value) { }
	int value;
};

struct AnotherTestEvent: public Lithe::Event {
	AnotherTestEvent(std::string msg): message(std::move(msg)) { }
	std::string message;
};
struct YetAnotherTestEvent: public Lithe::Event { };

// Test fixture
class EventDispatcherTest: public ::testing::Test {
	
	protected:

		Lithe::EventDispatcher dispatcher;

		void SetUp() override { }
		void TearDown() override { }

		template <typename E>
		std::size_t getEventTypeIdForTest() {
			return Lithe::EventDispatcher::getEventTypeId<E>();
		}

};



TEST_F(EventDispatcherTest, EventTypeIdConsistency) {
	auto id1 = getEventTypeIdForTest<TestEvent>();
	auto id2 = getEventTypeIdForTest<AnotherTestEvent>();
	auto id3 = getEventTypeIdForTest<YetAnotherTestEvent>();
	auto id4 = getEventTypeIdForTest<TestEvent>();  // Should be same as id1

	EXPECT_NE(id1, id2);
	EXPECT_NE(id1, id3);
	EXPECT_NE(id2, id3);

	EXPECT_EQ(id1, id4);
}

TEST_F(EventDispatcherTest, EventTypeIdConsistencyAcrossInvocations) {
	auto id1 = getEventTypeIdForTest<TestEvent>();
	auto id2 = getEventTypeIdForTest<AnotherTestEvent>();

	auto id3 = getEventTypeIdForTest<TestEvent>();
	auto id4 = getEventTypeIdForTest<AnotherTestEvent>();

	EXPECT_EQ(id1, id3);
	EXPECT_EQ(id2, id4);
}

TEST_F(EventDispatcherTest, DifferentEventTypeIds) {
	auto id1 = getEventTypeIdForTest<TestEvent>();
	auto id2 = getEventTypeIdForTest<AnotherTestEvent>();
	auto id3 = getEventTypeIdForTest<YetAnotherTestEvent>();

	EXPECT_NE(id1, id2);
	EXPECT_NE(id1, id3);
	EXPECT_NE(id2, id3);
}

TEST_F(EventDispatcherTest, BasicSubscribeAndDispatch) {
	int callCount = 0;
	int receivedValue = 0;

	dispatcher.subscribe<TestEvent>([&callCount, &receivedValue](const TestEvent& e) {
		callCount++;
		receivedValue = e.value;
	});

	auto id = getEventTypeIdForTest<TestEvent>();

	dispatcher.enqueue(TestEvent(42));
	dispatcher.processQueue();

	EXPECT_EQ(id, 0);
	EXPECT_EQ(callCount, 1);
	EXPECT_EQ(receivedValue, 42);
}



TEST_F(EventDispatcherTest, MultipleSubscribers) {
	int count1 = 0, count2 = 0;

	dispatcher.subscribe<TestEvent>([&](const TestEvent&) { count1++; });
	dispatcher.subscribe<TestEvent>([&](const TestEvent&) { count2++; });

	dispatcher.enqueue(TestEvent(1));
	dispatcher.processQueue();

	EXPECT_EQ(count1, 1);
	EXPECT_EQ(count2, 1);
}

TEST_F(EventDispatcherTest, DifferentEventTypes) {
	int testEventCount = 0;
	int anotherEventCount = 0;

	dispatcher.subscribe<TestEvent>([&](const TestEvent&) { testEventCount++; });
	dispatcher.subscribe<AnotherTestEvent>([&](const AnotherTestEvent&) { anotherEventCount++; });

	dispatcher.enqueue(TestEvent(1));
	dispatcher.enqueue(AnotherTestEvent("test"));
	dispatcher.processQueue();

	EXPECT_EQ(testEventCount, 1);
	EXPECT_EQ(anotherEventCount, 1);
}

// Test member function subscription
class EventListener {
	public:
	MOCK_METHOD(void, onTestEvent, (const TestEvent&));
	MOCK_METHOD(void, onAnotherEvent, (const AnotherTestEvent&));
};

TEST_F(EventDispatcherTest, MemberFunctionSubscription) {
	EventListener listener;

	EXPECT_CALL(listener, onTestEvent(::testing::_))
		.Times(1);

	dispatcher.subscribe(&listener, &EventListener::onTestEvent);
	dispatcher.enqueue(TestEvent(1));
	dispatcher.processQueue();
}

TEST_F(EventDispatcherTest, QueueOrder) {
	std::vector<int> received;

	dispatcher.subscribe<TestEvent>([&](const TestEvent& e) {
		received.push_back(e.value);
	});

	dispatcher.enqueue(TestEvent(1));
	dispatcher.enqueue(TestEvent(2));
	dispatcher.enqueue(TestEvent(3));
	dispatcher.processQueue();

	EXPECT_THAT(received, ::testing::ElementsAre(1, 2, 3));
}

TEST_F(EventDispatcherTest, ThreadSafety) {
	static const int NUM_EVENTS = 1000;
	std::atomic<int> receivedCount(0);
	std::atomic<int> enqueuedCount(0);

	dispatcher.subscribe<TestEvent>([&](const TestEvent&) {
		receivedCount++;
	});

	// Spawn multiple threads to enqueue events
	std::vector<std::thread> threads;
	for (int i = 0; i < 4; i++) {
		threads.emplace_back([&]() {
			for (int j = 0; j < NUM_EVENTS; j++) {
				dispatcher.enqueue(TestEvent(j));
				enqueuedCount++;
			}
		});
	}

	// Process queue in main thread
	std::thread processingThread([&]() {
		while (receivedCount < enqueuedCount) {
			dispatcher.processQueue();
			std::this_thread::yield();
		}
	});

	// Join all threads
	for (auto& thread : threads) {
		thread.join();
	}
	processingThread.join();

	EXPECT_EQ(receivedCount, 4 * NUM_EVENTS);
}


TEST_F(EventDispatcherTest, UnsubscribedEventType) {
	// Subscribe to TestEvent but not AnotherTestEvent
	int callCount = 0;
	dispatcher.subscribe<TestEvent>([&](const TestEvent&) { callCount++; });

	// Enqueue both types
	dispatcher.enqueue(TestEvent(1));
	dispatcher.enqueue(AnotherTestEvent("test"));
	dispatcher.processQueue();

	EXPECT_EQ(callCount, 1);  // Only TestEvent should trigger callback
}

TEST_F(EventDispatcherTest, CallbackExceptionSafety) {
	bool secondCallbackExecuted = false;

	// First callback throws
	dispatcher.subscribe<TestEvent>([](const TestEvent&) {
		throw std::runtime_error("Test exception");
	});

	// Second callback should still execute
	dispatcher.subscribe<TestEvent>([&](const TestEvent&) {
		secondCallbackExecuted = true;
	});

	dispatcher.enqueue(TestEvent(1));
	EXPECT_NO_THROW(dispatcher.processQueue());
	EXPECT_TRUE(secondCallbackExecuted);
}

