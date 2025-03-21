#include "pch.h"
#include "Input.h"

#include "Window.h"

#include <windows.h>
#include <utility>

namespace Lithe {

	static constexpr uint16_t getKeyCode(Key key) {
		switch (key) {
			default: return static_cast<uint8_t>(Key::UNKNOWN);

			case Key::SPACE: return VK_SPACE;
			case Key::APOSTROPHE: return VK_OEM_7;
			case Key::COMMA: return VK_OEM_COMMA;
			case Key::MINUS: return VK_OEM_MINUS;
			case Key::PERIOD: return VK_OEM_PERIOD;
			case Key::SLASH: return VK_OEM_2;
			case Key::_0: return '0';
			case Key::_1: return '1';
			case Key::_2: return '2';
			case Key::_3: return '3';
			case Key::_4: return '4';
			case Key::_5: return '5';
			case Key::_6: return '6';
			case Key::_7: return '7';
			case Key::_8: return '8';
			case Key::_9: return '9';
			case Key::SEMICOLON: return VK_OEM_1;
			case Key::EQUAL: return VK_OEM_PLUS;
			case Key::A: return 'A';
			case Key::B: return 'B';
			case Key::C: return 'C';
			case Key::D: return 'D';
			case Key::E: return 'E';
			case Key::F: return 'F';
			case Key::G: return 'G';
			case Key::H: return 'H';
			case Key::I: return 'I';
			case Key::J: return 'J';
			case Key::K: return 'K';
			case Key::L: return 'L';
			case Key::M: return 'M';
			case Key::N: return 'N';
			case Key::O: return 'O';
			case Key::P: return 'P';
			case Key::Q: return 'Q';
			case Key::R: return 'R';
			case Key::S: return 'S';
			case Key::T: return 'T';
			case Key::U: return 'U';
			case Key::V: return 'V';
			case Key::W: return 'W';
			case Key::X: return 'X';
			case Key::Y: return 'Y';
			case Key::Z: return 'Z';
			case Key::LEFT_BRACKET: return VK_OEM_4;
			case Key::BACKSLASH: return VK_OEM_5;
			case Key::RIGHT_BRACKET: return VK_OEM_6;
			case Key::GRAVE_ACCENT: return VK_OEM_3;
			case Key::WORLD_1: return VK_OEM_8;
			case Key::WORLD_2: return VK_OEM_102;
			case Key::ESCAPE: return VK_ESCAPE;
			case Key::ENTER: return VK_RETURN;
			case Key::TAB: return VK_TAB;
			case Key::BACKSPACE: return VK_BACK;
			case Key::INSERT: return VK_INSERT;
			case Key::DEL: return VK_DELETE;
			case Key::RIGHT: return VK_RIGHT;
			case Key::LEFT: return VK_LEFT;
			case Key::DOWN: return VK_DOWN;
			case Key::UP: return VK_UP;
			case Key::PAGE_UP: return VK_PRIOR;
			case Key::PAGE_DOWN: return VK_NEXT;
			case Key::HOME: return VK_HOME;
			case Key::END: return VK_END;
			case Key::CAPS_LOCK: return VK_CAPITAL;
			case Key::SCROLL_LOCK: return VK_SCROLL;
			case Key::NUM_LOCK: return VK_NUMLOCK;
			case Key::PRINT_SCREEN: return VK_SNAPSHOT;
			case Key::PAUSE: return VK_PAUSE;
			case Key::F1: return VK_F1;
			case Key::F2: return VK_F2;
			case Key::F3: return VK_F3;
			case Key::F4: return VK_F4;
			case Key::F5: return VK_F5;
			case Key::F6: return VK_F6;
			case Key::F7: return VK_F7;
			case Key::F8: return VK_F8;
			case Key::F9: return VK_F9;
			case Key::F10: return VK_F10;
			case Key::F11: return VK_F11;
			case Key::F12: return VK_F12;
			case Key::F13: return VK_F13;
			case Key::F14: return VK_F14;
			case Key::F15: return VK_F15;
			case Key::F16: return VK_F16;
			case Key::F17: return VK_F17;
			case Key::F18: return VK_F18;
			case Key::F19: return VK_F19;
			case Key::F20: return VK_F20;
			case Key::F21: return VK_F21;
			case Key::F22: return VK_F22;
			case Key::F23: return VK_F23;
			case Key::F24: return VK_F24;
			case Key::KP_0: return VK_NUMPAD0;
			case Key::KP_1: return VK_NUMPAD1;
			case Key::KP_2: return VK_NUMPAD2;
			case Key::KP_3: return VK_NUMPAD3;
			case Key::KP_4: return VK_NUMPAD4;
			case Key::KP_5: return VK_NUMPAD5;
			case Key::KP_6: return VK_NUMPAD6;
			case Key::KP_7: return VK_NUMPAD7;
			case Key::KP_8: return VK_NUMPAD8;
			case Key::KP_9: return VK_NUMPAD9;
			case Key::KP_DECIMAL: return VK_DECIMAL;
			case Key::KP_DIVIDE: return VK_DIVIDE;
			case Key::KP_MULTIPLY: return VK_MULTIPLY;
			case Key::KP_SUBTRACT: return VK_SUBTRACT;
			case Key::KP_ADD: return VK_ADD;
			case Key::KP_ENTER: return VK_RETURN;
			case Key::KP_EQUAL: return VK_OEM_PLUS;
			case Key::LEFT_SHIFT: return VK_SHIFT;
			case Key::LEFT_CONTROL: return VK_CONTROL;
			case Key::LEFT_ALT: return VK_MENU;
			case Key::LEFT_SUPER: return VK_LWIN;
			case Key::RIGHT_SHIFT: return VK_SHIFT;
			case Key::RIGHT_CONTROL: return VK_CONTROL;
			case Key::RIGHT_ALT: return VK_MENU;
			case Key::RIGHT_SUPER: return VK_RWIN;
			case Key::MENU: return VK_APPS;
		}
	}

	static constexpr uint8_t getMouseButtonCode(Button button) {
		switch (button) {
			default: return static_cast<uint8_t>(Button::UNKNOWN);

			case Button::_1: return VK_LBUTTON;
			case Button::_2: return VK_RBUTTON;
			case Button::_3: return VK_MBUTTON;
			case Button::_4: return VK_XBUTTON1;
			case Button::_5: return VK_XBUTTON2;
		}
	}

	static constexpr uint16_t KEY_PRESSED_MASK = 0x8000;
	static constexpr uint16_t KEY_RELEASED_MASK = 0x0001;

	MousePos Input::mousePos() {
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(pWindow->handle(), &pt);
		return MousePos(pt.x, pt.y);
	}


	bool Input::isKey(Key code, State state) {		
		const uint16_t keyState = GetAsyncKeyState(getKeyCode(code));

		switch (state) {
			case Lithe::State::PRESSED:
				return (keyState & KEY_PRESSED_MASK) != 0;
			case Lithe::State::RELEASED:
				return (keyState & KEY_PRESSED_MASK) == 0;
			case Lithe::State::REPEATED:
				return (keyState & KEY_PRESSED_MASK) != 0 && (keyState & KEY_RELEASED_MASK) != 0;
			default:
				return false;
		}
	}
	
	bool Input::isMouse(Button button, State state) {
		const uint16_t buttonState = GetAsyncKeyState(getMouseButtonCode(button));

		switch (state) {
			case Lithe::State::PRESSED:
				return (buttonState & KEY_PRESSED_MASK) != 0;
			case Lithe::State::RELEASED:
				return (buttonState & KEY_PRESSED_MASK) == 0;
			case Lithe::State::REPEATED:
				return (buttonState & KEY_PRESSED_MASK) != 0 && (buttonState & KEY_RELEASED_MASK) != 0;
			default:
				return false;
		}
	}

}
