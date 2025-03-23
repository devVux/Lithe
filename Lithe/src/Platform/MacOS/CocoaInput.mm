#include "pch.h"
#include "Input.h"

#include <Carbon/Carbon.h>
#include <AppKit/AppKit.h>

namespace Lithe {

    static constexpr uint16_t getKeyCode(Key key) {
        switch (key) {
            default: return static_cast<uint8_t>(Key::UNKNOWN);

            case Key::SPACE: return kVK_Space;
            case Key::APOSTROPHE: return kVK_ANSI_Quote;
            case Key::COMMA: return kVK_ANSI_Comma;
            case Key::MINUS: return kVK_ANSI_Minus;
            case Key::PERIOD: return kVK_ANSI_Period;
            case Key::SLASH: return kVK_ANSI_Slash;
            case Key::_0: return kVK_ANSI_0;
            case Key::_1: return kVK_ANSI_1;
            case Key::_2: return kVK_ANSI_2;
            case Key::_3: return kVK_ANSI_3;
            case Key::_4: return kVK_ANSI_4;
            case Key::_5: return kVK_ANSI_5;
            case Key::_6: return kVK_ANSI_6;
            case Key::_7: return kVK_ANSI_7;
            case Key::_8: return kVK_ANSI_8;
            case Key::_9: return kVK_ANSI_9;
            case Key::SEMICOLON: return kVK_ANSI_Semicolon;
            case Key::EQUAL: return kVK_ANSI_Equal;
            case Key::A: return kVK_ANSI_A;
            case Key::B: return kVK_ANSI_B;
            case Key::C: return kVK_ANSI_C;
            case Key::D: return kVK_ANSI_D;
            case Key::E: return kVK_ANSI_E;
            case Key::F: return kVK_ANSI_F;
            case Key::G: return kVK_ANSI_G;
            case Key::H: return kVK_ANSI_H;
            case Key::I: return kVK_ANSI_I;
            case Key::J: return kVK_ANSI_J;
            case Key::K: return kVK_ANSI_K;
            case Key::L: return kVK_ANSI_L;
            case Key::M: return kVK_ANSI_M;
            case Key::N: return kVK_ANSI_N;
            case Key::O: return kVK_ANSI_O;
            case Key::P: return kVK_ANSI_P;
            case Key::Q: return kVK_ANSI_Q;
            case Key::R: return kVK_ANSI_R;
            case Key::S: return kVK_ANSI_S;
            case Key::T: return kVK_ANSI_T;
            case Key::U: return kVK_ANSI_U;
            case Key::V: return kVK_ANSI_V;
            case Key::W: return kVK_ANSI_W;
            case Key::X: return kVK_ANSI_X;
            case Key::Y: return kVK_ANSI_Y;
            case Key::Z: return kVK_ANSI_Z;
            case Key::LEFT_BRACKET: return kVK_ANSI_LeftBracket;
            case Key::BACKSLASH: return kVK_ANSI_Backslash;
            case Key::RIGHT_BRACKET: return kVK_ANSI_RightBracket;
            case Key::GRAVE_ACCENT: return kVK_ANSI_Grave;
            case Key::ESCAPE: return kVK_Escape;
            case Key::ENTER: return kVK_Return;
            case Key::TAB: return kVK_Tab;
            case Key::BACKSPACE: return kVK_Delete;
            case Key::INSERT: return kVK_Help; // macOS has no Insert key; mapped to Help
            case Key::DEL: return kVK_ForwardDelete;
            case Key::RIGHT: return kVK_RightArrow;
            case Key::LEFT: return kVK_LeftArrow;
            case Key::DOWN: return kVK_DownArrow;
            case Key::UP: return kVK_UpArrow;
            case Key::PAGE_UP: return kVK_PageUp;
            case Key::PAGE_DOWN: return kVK_PageDown;
            case Key::HOME: return kVK_Home;
            case Key::END: return kVK_End;
            case Key::CAPS_LOCK: return kVK_CapsLock;
            case Key::F1: return kVK_F1;
            case Key::F2: return kVK_F2;
            case Key::F3: return kVK_F3;
            case Key::F4: return kVK_F4;
            case Key::F5: return kVK_F5;
            case Key::F6: return kVK_F6;
            case Key::F7: return kVK_F7;
            case Key::F8: return kVK_F8;
            case Key::F9: return kVK_F9;
            case Key::F10: return kVK_F10;
            case Key::F11: return kVK_F11;
            case Key::F12: return kVK_F12;
            case Key::F13: return kVK_F13;
            case Key::F14: return kVK_F14;
            case Key::F15: return kVK_F15;
            case Key::F16: return kVK_F16;
            case Key::F17: return kVK_F17;
            case Key::F18: return kVK_F18;
            case Key::F19: return kVK_F19;
            case Key::KP_0: return kVK_ANSI_Keypad0;
            case Key::KP_1: return kVK_ANSI_Keypad1;
            case Key::KP_2: return kVK_ANSI_Keypad2;
            case Key::KP_3: return kVK_ANSI_Keypad3;
            case Key::KP_4: return kVK_ANSI_Keypad4;
            case Key::KP_5: return kVK_ANSI_Keypad5;
            case Key::KP_6: return kVK_ANSI_Keypad6;
            case Key::KP_7: return kVK_ANSI_Keypad7;
            case Key::KP_8: return kVK_ANSI_Keypad8;
            case Key::KP_9: return kVK_ANSI_Keypad9;
            case Key::KP_DECIMAL: return kVK_ANSI_KeypadDecimal;
            case Key::KP_DIVIDE: return kVK_ANSI_KeypadDivide;
            case Key::KP_MULTIPLY: return kVK_ANSI_KeypadMultiply;
            case Key::KP_SUBTRACT: return kVK_ANSI_KeypadMinus;
            case Key::KP_ADD: return kVK_ANSI_KeypadPlus;
            case Key::KP_ENTER: return kVK_ANSI_KeypadEnter;
            case Key::KP_EQUAL: return kVK_ANSI_KeypadEquals;
            case Key::LEFT_SHIFT: return kVK_Shift;
            case Key::LEFT_CONTROL: return kVK_Control;
            case Key::LEFT_ALT: return kVK_Option;
            case Key::LEFT_SUPER: return kVK_Command;
            case Key::RIGHT_SHIFT: return kVK_RightShift;
            case Key::RIGHT_CONTROL: return kVK_RightControl;
            case Key::RIGHT_ALT: return kVK_RightOption;
            case Key::RIGHT_SUPER: return kVK_RightCommand;
            case Key::MENU: return kVK_Function;
        }
    }

    static constexpr uint8_t getMouseButtonCode(Button button) {
        switch (button) {
            default: return static_cast<uint8_t>(Button::UNKNOWN);
            case Button::_1: return 0; // Left
            case Button::_2: return 1;  // Right
            case Button::_3: return 2;  // Middle
            case Button::_4: return 3;  // X1
            case Button::_5: return 4;  // X2
        }
    }

    MousePos Input::mousePos() {
        NSPoint mouseLoc = [NSEvent mouseLocation];
        NSWindow* window = (__bridge NSWindow*)pWindow->handle();
        NSPoint windowLoc = [window convertRectFromScreen:NSMakeRect(mouseLoc.x, mouseLoc.y, 0, 0)].origin;
        NSRect windowFrame = [window contentRectForFrameRect:window.frame];
        
        // Flip Y-coordinate (Cocoa origin is bottom-left)
        float y = windowFrame.size.height - windowLoc.y;
        return MousePos(windowLoc.x, y);
    }

    bool Input::isKey(Key code, State state) {
        CGKeyCode keyCode = getKeyCode(code);
        bool isPressed = CGEventSourceKeyState(kCGEventSourceStateCombinedSessionState, keyCode);

        switch (state) {
            case State::PRESSED: return isPressed;
            case State::RELEASED: return !isPressed;
            // REPEATED not directly pollable; return pressed as a placeholder
            case State::REPEATED: return isPressed;
            default: return false;
        }
    }

    bool Input::isMouse(Button button, State state) {
        NSUInteger mask = [NSEvent pressedMouseButtons];
        uint8_t buttonCode = getMouseButtonCode(button);
        bool isPressed = (mask & (1 << buttonCode)) != 0;

        switch (state) {
            case State::PRESSED: return isPressed;
            case State::RELEASED: return !isPressed;
            // REPEATED not applicable for mouse buttons
            case State::REPEATED: return false;
            default: return false;
        }
    }

}
