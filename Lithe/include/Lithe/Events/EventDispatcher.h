#pragma once

#include <Lithe/Events/Event.h>
#include <Lithe/Core/Log.h>

#include <iostream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <functional>
#include <typeindex>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

namespace Lithe {

	template<typename E>
	concept IsEvent = std::is_base_of<Event, E>::value;



	class EventDispatcher {

		friend class EventDispatcherTest;

		private:

			struct EventWrapperBase {
				virtual ~EventWrapperBase() = default;
				virtual void dispatch(EventDispatcher& dispatcher) const = 0;
			};

			template <class E>
			struct EventWrapper: public EventWrapperBase {
			
				public:
			
					explicit EventWrapper(E event): mEvent(std::move(event)) { }
					void dispatch(EventDispatcher& dispatcher) const override {
						dispatcher.dispatch(mEvent);
					}

				private:

					E mEvent;
			};

		
		public:

			// Subscribe using lambdas or free functions
			template<typename E>
			requires IsEvent<E>
			void subscribe(std::function<void(const E&)> callback) {
				auto eventId = getEventTypeId<E>();

				std::lock_guard<std::mutex> lock(mCallbacksMutex);
				mCallbacks[eventId].push_back([callback](const Event& e) {
					callback(static_cast<const E&>(e));
				});
			}

			// Subscribe using member functions
			template<typename T, typename E>
			requires IsEvent<E>
			void subscribe(T* instance, void(T::*memberFn)(const E&)) {
				auto eventId = getEventTypeId<E>();

				std::lock_guard<std::mutex> lock(mCallbacksMutex);
				mCallbacks[eventId].push_back([instance, memberFn](const Event& e) {
					(instance->*memberFn)(static_cast<const E&>(e));
				});
			}

			// Enqueue an event for processing
			template<typename E>
			requires IsEvent<E>
			void enqueue(const E& event) {
				std::lock_guard<std::mutex> lock(mQueueMutex);
				mEventQueue.push(std::make_shared<EventWrapper<E>>(event));
			}

			void processQueue() {
				while (true) {
					std::shared_ptr<EventWrapperBase> event;
					{
						std::lock_guard<std::mutex> lock(mQueueMutex);
						if (mEventQueue.empty()) {
							break;
						}
						event = std::move(mEventQueue.front());
						mEventQueue.pop();
					}
					event->dispatch(*this);
				}
			}


		public:

			// Immediate dispatch of an event (thread-safe)
			template<typename E>
			requires IsEvent<E>
			void dispatch(const E& event) {
				auto eventId = getEventTypeId<E>();
				std::vector<std::function<void(const Event&)>> callbacksCopy;

				{
					std::lock_guard<std::mutex> lock(mCallbacksMutex);
					if (mCallbacks.contains(eventId))
						callbacksCopy = mCallbacks[eventId]; // Copy mCallbacks to avoid locking during execution
				}

				for (const auto& callback : callbacksCopy)
					try { 
						callback(event);
					} catch (const std::exception& e) {
						Lithe::Log::ERR("Exception in event callback: {}", e.what());
					} catch (...) {
						Lithe::Log::ERR("Unknown exception in event callback");
					}
			}

			// Type-safe event ID generation
			template<typename E>
			requires IsEvent<E>
			static std::size_t getEventTypeId() {
				static std::size_t id = nextEventTypeId++;
				return id;
			}


		private:

			static inline std::size_t nextEventTypeId = 0;

			std::unordered_map<std::size_t, std::vector<std::function<void(const Event&)>>> mCallbacks;
			std::mutex mCallbacksMutex;

			std::queue<std::shared_ptr<EventWrapperBase>> mEventQueue;
			std::mutex mQueueMutex;

	};


}