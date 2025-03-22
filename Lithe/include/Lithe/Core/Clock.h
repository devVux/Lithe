#pragma once

#include <chrono>
#include <cassert>

namespace Lithe {

	namespace Time {

		using Timestep = double;

		class Clock {
			
			public:

				using ClockType = std::chrono::high_resolution_clock;
				using TimePoint = ClockType::time_point;
				using Duration = std::chrono::duration<Timestep>;

				// @param tickInterval: updates per second
				Clock(uint32_t tickInterval = 10): mStart(ClockType::now()), mLastUpdate(mStart), mTickInterval(Duration(1.0 / tickInterval > 0 ? 1.0 / tickInterval : 1.0)) { }
				
				// Set the tick interval in seconds
				void setTickInterval(Timestep interval) {
					mTickInterval = Duration(interval);
				}

				// Get the tick interval in seconds
				Timestep tickInterval() const {
					return mTickInterval.count();
				}

				// Get the elapsed time in seconds since the last tick
				Timestep timeSinceLastUpdate() {
					auto now = ClockType::now();
					Duration duration = now - mLastUpdate;
					mLastUpdate = now;
					return duration.count();
				}

				// Get the total elapsed time in seconds since the clock started
				Timestep elapsed() const {
					return Duration(ClockType::now() - mStart).count();
				}


			private:
				
				TimePoint mStart;
				TimePoint mLastUpdate;
				Duration mTickInterval;

		};

	};

}
