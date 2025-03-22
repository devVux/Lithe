#include "pch.h"
#include "Window.h"

#include "Utils.h"
#include "WindowEvents.h"
#include "KeyEvents.h"
#include "MouseEvents.h"
#include "Input.h"

#include <winnt.h>
#include <winuser.h>
#include <windows.h>
#include <windowsx.h>

namespace Lithe {

	static constexpr Key getKeyFromWindowsCode(uint8_t keyCode) {
		switch (keyCode) {
			case VK_SPACE: return Key::SPACE;
			case VK_OEM_7: return Key::APOSTROPHE;
			case VK_OEM_COMMA: return Key::COMMA;
			case VK_OEM_MINUS: return Key::MINUS;
			case VK_OEM_PERIOD: return Key::PERIOD;
			case VK_OEM_2: return Key::SLASH;
			case '0': return Key::_0;
			case '1': return Key::_1;
			case '2': return Key::_2;
			case '3': return Key::_3;
			case '4': return Key::_4;
			case '5': return Key::_5;
			case '6': return Key::_6;
			case '7': return Key::_7;
			case '8': return Key::_8;
			case '9': return Key::_9;
			case VK_OEM_1: return Key::SEMICOLON;
			case VK_OEM_PLUS: return Key::EQUAL;
			case 'A': return Key::A;
			case 'B': return Key::B;
			case 'C': return Key::C;
			case 'D': return Key::D;
			case 'E': return Key::E;
			case 'F': return Key::F;
			case 'G': return Key::G;
			case 'H': return Key::H;
			case 'I': return Key::I;
			case 'J': return Key::J;
			case 'K': return Key::K;
			case 'L': return Key::L;
			case 'M': return Key::M;
			case 'N': return Key::N;
			case 'O': return Key::O;
			case 'P': return Key::P;
			case 'Q': return Key::Q;
			case 'R': return Key::R;
			case 'S': return Key::S;
			case 'T': return Key::T;
			case 'U': return Key::U;
			case 'V': return Key::V;
			case 'W': return Key::W;
			case 'X': return Key::X;
			case 'Y': return Key::Y;
			case 'Z': return Key::Z;
			case VK_OEM_4: return Key::LEFT_BRACKET;
			case VK_OEM_5: return Key::BACKSLASH;
			case VK_OEM_6: return Key::RIGHT_BRACKET;
			case VK_OEM_3: return Key::GRAVE_ACCENT;
			case VK_OEM_8: return Key::WORLD_1;
			case VK_OEM_102: return Key::WORLD_2;
			case VK_ESCAPE: return Key::ESCAPE;
			case VK_RETURN: return Key::ENTER;
			case VK_TAB: return Key::TAB;
			case VK_BACK: return Key::BACKSPACE;
			case VK_INSERT: return Key::INSERT;
			case VK_DELETE: return Key::DEL;
			case VK_RIGHT: return Key::RIGHT;
			case VK_LEFT: return Key::LEFT;
			case VK_DOWN: return Key::DOWN;
			case VK_UP: return Key::UP;
			case VK_PRIOR: return Key::PAGE_UP;
			case VK_NEXT: return Key::PAGE_DOWN;
			case VK_HOME: return Key::HOME;
			case VK_END: return Key::END;
			case VK_CAPITAL: return Key::CAPS_LOCK;
			case VK_SCROLL: return Key::SCROLL_LOCK;
			case VK_NUMLOCK: return Key::NUM_LOCK;
			case VK_SNAPSHOT: return Key::PRINT_SCREEN;
			case VK_PAUSE: return Key::PAUSE;
			case VK_F1: return Key::F1;
			case VK_F2: return Key::F2;
			case VK_F3: return Key::F3;
			case VK_F4: return Key::F4;
			case VK_F5: return Key::F5;
			case VK_F6: return Key::F6;
			case VK_F7: return Key::F7;
			case VK_F8: return Key::F8;
			case VK_F9: return Key::F9;
			case VK_F10: return Key::F10;
			case VK_F11: return Key::F11;
			case VK_F12: return Key::F12;
			case VK_F13: return Key::F13;
			case VK_F14: return Key::F14;
			case VK_F15: return Key::F15;
			case VK_F16: return Key::F16;
			case VK_F17: return Key::F17;
			case VK_F18: return Key::F18;
			case VK_F19: return Key::F19;
			case VK_F20: return Key::F20;
			case VK_F21: return Key::F21;
			case VK_F22: return Key::F22;
			case VK_F23: return Key::F23;
			case VK_F24: return Key::F24;
			case VK_NUMPAD0: return Key::KP_0;
			case VK_NUMPAD1: return Key::KP_1;
			case VK_NUMPAD2: return Key::KP_2;
			case VK_NUMPAD3: return Key::KP_3;
			case VK_NUMPAD4: return Key::KP_4;
			case VK_NUMPAD5: return Key::KP_5;
			case VK_NUMPAD6: return Key::KP_6;
			case VK_NUMPAD7: return Key::KP_7;
			case VK_NUMPAD8: return Key::KP_8;
			case VK_NUMPAD9: return Key::KP_9;
			case VK_DECIMAL: return Key::KP_DECIMAL;
			case VK_DIVIDE: return Key::KP_DIVIDE;
			case VK_MULTIPLY: return Key::KP_MULTIPLY;
			case VK_SUBTRACT: return Key::KP_SUBTRACT;
			case VK_ADD: return Key::KP_ADD;
			case VK_SHIFT: return Key::LEFT_SHIFT; // and RIGHT_SHIFT
			case VK_CONTROL: return Key::LEFT_CONTROL; // and RIGHT_CONTROL
			case VK_MENU: return Key::LEFT_ALT; // and RIGHT_ALT
			case VK_LWIN: return Key::LEFT_SUPER;
			case VK_RWIN: return Key::RIGHT_SUPER;
			case VK_APPS: return Key::MENU;
			default: return Key::UNKNOWN;
		}
	}

	

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		Window* impl = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

		if (impl)
			impl->dispatchEvent(uMsg, wParam, lParam);

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	
	Window::Window(SharedPtr<EventDispatcher> pDispatcher, std::string title, Size size, Pos pos, bool centered): 
		pDispatcher(pDispatcher), mCentered(centered) {

		HINSTANCE instance = GetModuleHandle(nullptr);

				// Register window class
		const char CLASS_NAME[] = "Sample Window Class";

		WNDCLASS wc = { 0 };
		wc.lpfnWndProc = WindowProc;
		wc.hInstance = instance;
		wc.lpszClassName = CLASS_NAME;
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);

		if (!RegisterClass(&wc))
			Log::FATAL("Could not register window class");

		// Create window
		pNativeHandle = CreateWindowEx(
			0, CLASS_NAME, title.data(), WS_OVERLAPPEDWINDOW,
			pos.x, pos.y, size.width, size.height,
			NULL, NULL, instance, this
		);

		if (pNativeHandle == NULL) {
			DWORD error = GetLastError();
			Log::FATAL("Could not create window, error code: {}", error);
		}

		// Register window pointer with the handle
		SetWindowLongPtr(pNativeHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	
		if (centered) {
			Size screen = screenSize();

			Size center(
				(screen.width - size.width) / 2,
				(screen.height - size.height) / 2
			);

			move(center);
		}

		show();

	}

	Window::~Window() {
		if (pNativeHandle) {
			DestroyWindow(pNativeHandle);
			PostQuitMessage(0);
			pNativeHandle = NULL;
		}
	}

	void Window::processEvents() const { 
		MSG msg = { 0 };
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT)
				return;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	void Window::dispatchEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) {

		auto enqueue = [&](auto&& event) {
			pDispatcher->enqueue<std::decay_t<decltype(event)>>(std::forward<decltype(event)>(event));
		};

		// TODO: implement other events
		// https://learn.microsoft.com/en-us/windows/win32/inputdev
		switch (uMsg) {
			default: break; //Log::TRACE("{} Yet to be implemented", uMsg); break;

			{
				using namespace MouseEvents;
				
				// TODO: implement events for other Button buttons
				case WM_MOUSEMOVE:      enqueue(MouseMovedEvent(MousePos(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))); break;
				case WM_LBUTTONDOWN:    enqueue(MouseButtonPressedEvent(Button::_1)); break;
				case WM_LBUTTONUP:      enqueue(MouseButtonReleasedEvent(Button::_1)); break;
				case WM_RBUTTONDOWN:    enqueue(MouseButtonPressedEvent(Button::_2)); break;
				case WM_RBUTTONUP:      enqueue(MouseButtonReleasedEvent(Button::_2)); break;
				case WM_MBUTTONDOWN:    enqueue(MouseButtonPressedEvent(Button::_3)); break;
				case WM_MBUTTONUP:      enqueue(MouseButtonReleasedEvent(Button::_3)); break;
				case WM_MOUSEHOVER:     enqueue(MouseHoveredEvent(MousePos(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))); break;
				case WM_MOUSEHWHEEL:    enqueue(MouseWheelEvent(MousePos(GET_WHEEL_DELTA_WPARAM(wParam), 0))); break;
				case WM_MOUSEWHEEL:     enqueue(MouseWheelEvent(MousePos(0, GET_WHEEL_DELTA_WPARAM(wParam)))); break;
			}

			{
				using namespace KeyEvents;

				case WM_KEYDOWN:        enqueue(KeyPressedEvent(getKeyFromWindowsCode(wParam))); break;
				case WM_KEYUP:          enqueue(KeyReleasedEvent(getKeyFromWindowsCode(wParam))); break;
				case WM_CHAR:           enqueue(KeyTypedEvent(getKeyFromWindowsCode(wParam))); break;
			}

			{
				using namespace WindowEvents;

				case WM_CLOSE:          enqueue(WindowClosedEvent()); break;
				case WM_SETFOCUS:       enqueue(WindowFocusedEvent(true)); break;
				case WM_KILLFOCUS:      enqueue(WindowFocusedEvent(false)); break;
				case WM_SIZE:           enqueue(WindowResizedEvent(Size(LOWORD(lParam), HIWORD(lParam)))); break;
				case WM_SIZING:         enqueue(WindowResizedEvent(Size(LOWORD(lParam), HIWORD(lParam)), true)); break;
				case WM_MOVE:           enqueue(WindowMovedEvent(Size(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))); break;
				case WM_MOVING:         enqueue(WindowMovedEvent(Size(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)), true)); break;
				case WM_ACTIVATE:       enqueue(WindowFocusedEvent(wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)); break;

				// System Commands
				case WM_SYSCOMMAND:
					{
						switch (wParam & 0xfff0) {
							default: break;
							case SC_MAXIMIZE:   enqueue(WindowMaximizedEvent()); break;
							case SC_MINIMIZE:	enqueue(WindowMinimizedEvent()); break;
							case SC_RESTORE:    enqueue(WindowRestoredEvent()); break;
							case SC_CLOSE:      enqueue(WindowClosedEvent()); break;
							case SC_MOVE:       enqueue(WindowMovedEvent(Size(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))); break;
							case SC_SIZE:       enqueue(WindowResizedEvent(Size(LOWORD(lParam), HIWORD(lParam)))); break;
						}
						break;
					}

				// Additional Useful Events
				//case WM_ENTERSIZEMOVE:  enqueue(WindowResizeMoveStartedEvent); break;
				//case WM_EXITSIZEMOVE:   enqueue(WindowResizeMoveEndedEvent); break;
				//case WM_DPICHANGED:     enqueue(WindowDPIChangedEvent, LOWORD(wParam), HIWORD(wParam)); break;
				//case WM_GETMINMAXINFO:  enqueue(WindowGetMinMaxInfoEvent, reinterpret_cast<MINMAXINFO*>(lParam)); break;
				//case WM_ERASEBKGND:     enqueue(WindowEraseBackgroundEvent, reinterpret_cast<HDC>(wParam)); break;
				//case WM_PAINT:          enqueue(WindowPaintEvent); break;
				//case WM_DESTROY:        enqueue(WindowDestroyedEvent); break;
				//case WM_SHOWWINDOW:     enqueue(WindowVisibilityChangedEvent, wParam == TRUE); break;
				//case WM_ACTIVATEAPP:    enqueue(WindowActivationChangedEvent, wParam == TRUE); break;

			}

		}


	}


	bool Window::shouldClose() const { return !IsWindow(pNativeHandle); }
	void Window::show() const { ShowWindow(pNativeHandle, SW_SHOW); }
	void Window::hide() const { ShowWindow(pNativeHandle, SW_HIDE); }
	void Window::rename(const std::string& title) { SetWindowText(pNativeHandle, title.c_str()); }

	std::string Window::title() const {
		char buffer[256];
		GetWindowText(pNativeHandle, buffer, sizeof(buffer));
		return std::string(buffer);
	}
	void Window::resize(Size size) {
		SetWindowPos(pNativeHandle, NULL, 0, 0, size.width, size.height, SWP_NOMOVE | SWP_NOZORDER);
	}
	Size Window::size() const {
		RECT rect;
		GetClientRect(pNativeHandle, &rect);
		return { rect.right - rect.left, rect.bottom - rect.top };
	}
	void Window::move(Pos pos) {
		SetWindowPos(pNativeHandle, NULL, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
	Size Window::position() const {
		RECT rect;
		GetWindowRect(pNativeHandle, &rect);
		return { rect.left, rect.top };
	}
	Size Window::screenSize() const {
		RECT rect;
		GetClientRect(GetDesktopWindow(), &rect);
		return { rect.right - rect.left, rect.bottom - rect.top };
	}

}
