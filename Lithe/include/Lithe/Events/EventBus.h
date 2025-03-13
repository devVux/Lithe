#pragma once

#include <thread>
#include <atomic>
#include <chrono>

#include "EventDispatcher.h"

namespace Lithe {

	class EventBus {
		
		public:

			explicit EventBus(EventDispatcher& dispatcher): mDispatcher(dispatcher), mRunning(false) { }

			~EventBus() {
				stop();
			}

			void start() {
				mRunning = true;
				mEventThread = std::thread([this]() {
					while (mRunning) {
						mDispatcher.processQueue();
						std::this_thread::sleep_for(std::chrono::milliseconds(100));
					}
				});
			}

			void stop() {
				mRunning = false;
				if (mEventThread.joinable())
					mEventThread.join();
			}

		private:
			
			EventDispatcher& mDispatcher;
			std::atomic<bool> mRunning;
			std::thread mEventThread;

	};


}