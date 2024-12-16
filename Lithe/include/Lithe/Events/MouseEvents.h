#pragma once

#include <Lithe/Events/Event.h>

namespace Lithe {

	class MouseButtonEvent: public Event {

		public:

			int button() const { return mButton; }

		protected:

			MouseButtonEvent(int button): mButton(button) { }


		protected:

			int mButton;

	};


	class MouseButtonPressedEvent: public MouseButtonEvent {

		public:

			MouseButtonPressedEvent(int button): MouseButtonEvent(button) { }


			virtual std::string name() const override {
				std::stringstream ss;
				ss << "MouseButtonPressedEvent: " << mButton;
				return ss.str();
			}

			virtual EventType type() const override {
				return EventType::MouseButtonPressed;
			}

	};


	class MouseButtonReleasedEvent: public MouseButtonEvent {

		public:

			MouseButtonReleasedEvent(int button): MouseButtonEvent(button) { }


			virtual std::string name() const override {
				std::stringstream ss;
				ss << "MouseButtonReleasedEvent: " << mButton;
				return ss.str();
			}

			virtual EventType type() const override {
				return EventType::MouseButtonReleased;
			}

	};


	class MouseMovedEvent: public Event {

		public:

			MouseMovedEvent(int x, int y): x(x), y(y) { }

			virtual std::string name() const override {
				std::stringstream ss;
				ss << "MouseMovedEvent: (" << x << ", " << y << ")";
				return ss.str();
	
			}

			virtual EventType type() const override {
				return EventType::MouseMoved;
			}


			const int x;
			const int y;

	};


	class MouseScrolledEvent: public Event {

		public:

			MouseScrolledEvent(float xOff, float yOff): mXOffset(xOff), mYOffset(yOff) { }

			virtual std::string name() const override {
				std::stringstream ss;
				ss << "MouseScrolledEvent: (" << mXOffset << ", " << mYOffset << ")";
				return ss.str();
			}

			virtual EventType type() const override {
				return EventType::MouseScrolled;
			}


		private:

			const uint32_t mXOffset;
			const uint32_t mYOffset;

	};

}