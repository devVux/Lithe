#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <chrono>

#include <EventDispatcher.h>

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

