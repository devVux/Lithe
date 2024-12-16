#pragma once

#include <string>
#include <GLFW/glfw3.h>

namespace Lithe {
	
	struct Event {
		virtual ~Event() = default;
	};

	namespace WindowEvents {

		struct WindowResizeEvent: public Event {
			WindowResizeEvent(int width, int height): width(width), height(height) { }

			const int width;
			const int height;
		};

		struct WindowCloseEvent: public Event {
			WindowCloseEvent() = default;
		};

		struct FramebufferResizeEvent: public Event {
			FramebufferResizeEvent(int width, int height): width(width), height(height) { }

			const int width;
			const int height;
		};

	}

	namespace KeyEvents {

		struct KeyEvent: public Event {
			
			protected:

				KeyEvent(int keycode, int scancode, int mods): 
					keyCode(keycode), scanCode(scancode), modifiers(mods) { }

			public:

				const int keyCode;
				const int scanCode;
				const int modifiers;

		};

		struct KeyPressedEvent: public KeyEvent {
			KeyPressedEvent(int keycode, int scancode, int mods): KeyEvent(keycode, scancode, mods) { }
		};

		struct KeyReleasedEvent: public KeyEvent {
			KeyReleasedEvent(int keycode, int scancode, int mods): KeyEvent(keycode, scancode, mods) { }
		};

		struct KeyRepeatEvent: public KeyEvent {
			KeyRepeatEvent(int keycode, int scancode, int mods): KeyEvent(keycode, scancode, mods) { }
		};

	}

	namespace MouseEvents {

		struct MouseButtonEvent: public Event {
			
			protected:
				
				MouseButtonEvent(int buttoncode, int mods): button(buttoncode), modifiers(mods) { }

			public:
			
				const int button;
				const int modifiers;

		};

		struct MouseButtonPressedEvent: public MouseButtonEvent {
			MouseButtonPressedEvent(int button, int mods): MouseButtonEvent(button, mods) { }
		};

		struct MouseButtonReleasedEvent: public MouseButtonEvent {
			MouseButtonReleasedEvent(int button, int mods): MouseButtonEvent(button, mods) { }
		};

		struct MouseScrollEvent: public Event {
			MouseScrollEvent(double xOffset, double yOffset): xOffset(xOffset), yOffset(yOffset) { }

			const double xOffset;
			const double yOffset;
		};

		struct MouseMovedEvent: public Event {
			MouseMovedEvent(double xPos, double yPos): xPos(xPos), yPos(yPos) { }

			const double xPos;
			const double yPos;
		};

	}

	enum MouseButtons {
		BUTTON_1 = 0,
		BUTTON_2 = 1,
		BUTTON_3 = 2,
		BUTTON_4 = 3,
		BUTTON_5 = 4,
		BUTTON_6 = 5,
		BUTTON_7 = 6,
		BUTTON_8 = 7,
		BUTTON_LAST = GLFW_MOUSE_BUTTON_8,
		BUTTON_LEFT = GLFW_MOUSE_BUTTON_1,
		BUTTON_RIGHT = GLFW_MOUSE_BUTTON_2,
		BUTTON_MIDDLE = GLFW_MOUSE_BUTTON_3
	};

}