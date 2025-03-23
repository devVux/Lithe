#pragma once

#include "Event.h"

namespace Lithe::KeyEvents {

	struct KeyEvent: public Event {
		
		protected:

			KeyEvent(Key keycode, Key scancode, Key mods):
				keyCode(keycode), scanCode(scancode), modifiers(mods) { }
			KeyEvent(const KeyEvent&) = default;
			KeyEvent(KeyEvent&&) = default;

		public:

			const Key keyCode;
			const Key scanCode;
			const Key modifiers;

	};

	struct KeyPressedEvent: public KeyEvent {
		KeyPressedEvent(Key keycode, Key scancode = Key::UNKNOWN, Key mods = Key::UNKNOWN): KeyEvent(keycode, scancode, mods) { }
		KeyPressedEvent(const KeyPressedEvent&) = default;
		KeyPressedEvent(KeyPressedEvent&&) = default;
	};

	struct KeyReleasedEvent: public KeyEvent {
		KeyReleasedEvent(Key keycode, Key scancode = Key::UNKNOWN, Key mods = Key::UNKNOWN): KeyEvent(keycode, scancode, mods) { }
		KeyReleasedEvent(const KeyReleasedEvent&) = default;
		KeyReleasedEvent(KeyReleasedEvent&&) = default;
	};
	
	struct KeyTypedEvent: public KeyEvent {
		KeyTypedEvent(Key keycode, Key scancode = Key::UNKNOWN, Key mods = Key::UNKNOWN): KeyEvent(keycode, scancode, mods) { }
		KeyTypedEvent(const KeyTypedEvent&) = default;
		KeyTypedEvent(KeyTypedEvent&&) = default;
	};

	struct KeyRepeatEvent: public KeyEvent {
		KeyRepeatEvent(Key keycode, Key scancode = Key::UNKNOWN, Key mods = Key::UNKNOWN): KeyEvent(keycode, scancode, mods) { }
		KeyRepeatEvent(const KeyRepeatEvent&) = default;
		KeyRepeatEvent(KeyRepeatEvent&&) = default;
	};


}
