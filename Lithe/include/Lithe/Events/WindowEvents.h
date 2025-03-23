#pragma once

#include <Event.h>

namespace Lithe::WindowEvents {

	struct WindowResizedEvent: public Event {
		WindowResizedEvent(Size s, bool r = false): size(s), resizing(r) { }\
		WindowResizedEvent(const WindowResizedEvent&) = default;
		WindowResizedEvent(WindowResizedEvent&&) = default;

		const Size size;
		bool resizing;
	};
	
	struct WindowMovedEvent: public Event {
		WindowMovedEvent(Size s, bool m = false): size(s), moving(m) { }
		WindowMovedEvent(const WindowMovedEvent&) = default;
		WindowMovedEvent(WindowMovedEvent&&) = default;

		const Size size;
		bool moving;
	};

	struct WindowClosedEvent: public Event { };
	
	struct WindowFocusedEvent: public Event {
		WindowFocusedEvent(bool focus): focused(focus) { }
		WindowFocusedEvent(const WindowFocusedEvent&) = default;
		WindowFocusedEvent(WindowFocusedEvent&&) = default;

		const bool focused;
	};
	
	struct WindowMaximizedEvent: public Event { };

	struct WindowMinimizedEvent: public Event { };

	struct WindowRestoredEvent: public Event { };
	
}
