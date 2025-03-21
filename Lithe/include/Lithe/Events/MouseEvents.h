#pragma once

#include "Event.h"

namespace Lithe::MouseEvents {

	struct MouseButtonEvent: public Event {
		
		protected:
			
			MouseButtonEvent(Button button, Key mods): button(button), modifiers(mods) { }
			MouseButtonEvent(const MouseButtonEvent&) = default;
			MouseButtonEvent(MouseButtonEvent&&) = default;

		public:
		
			const Button button;
			const Key modifiers;

	};

	struct MouseButtonPressedEvent: public MouseButtonEvent {
		MouseButtonPressedEvent(Button button, Key mods = Key::UNKNOWN): MouseButtonEvent(button, mods) { }
		MouseButtonPressedEvent(const MouseButtonPressedEvent&) = default;
		MouseButtonPressedEvent(MouseButtonPressedEvent&&) = default;
	};

	struct MouseButtonReleasedEvent: public MouseButtonEvent {
		MouseButtonReleasedEvent(Button button, Key mods = Key::UNKNOWN): MouseButtonEvent(button, mods) { }
		MouseButtonReleasedEvent(const MouseButtonReleasedEvent&) = default;
		MouseButtonReleasedEvent(MouseButtonReleasedEvent&&) = default;
	};

	struct MouseWheelEvent: public Event {
		MouseWheelEvent(const MousePos& p): pos(p) { }
		MouseWheelEvent(const MouseWheelEvent&) = default;
		MouseWheelEvent(MouseWheelEvent&&) = default;

		const MousePos pos;
	};

	struct MouseMovedEvent: public Event {
		MouseMovedEvent(MousePos p): pos(p) { }
		MouseMovedEvent(const MouseMovedEvent&) = default;
		MouseMovedEvent(MouseMovedEvent&&) = default;

		const MousePos pos;
	};

	struct MouseHoveredEvent: public Event {
		MouseHoveredEvent(MousePos offsets): offset(offsets) { }
		MouseHoveredEvent(const MouseHoveredEvent&) = default;
		MouseHoveredEvent(MouseHoveredEvent&&) = default;

		const MousePos offset;
	};


}
