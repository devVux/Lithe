#include "pch.h"
#include "Window.h"

#include "Log.h"

#include "Event.h"
#include "EventDispatcher.h"
#include "Utils.h"

#include <winnt.h>
#include <winuser.h>
#include <windows.h>
#include <windowsx.h>

namespace Lithe {

	
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		Window* impl = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

		if (impl)
			impl->dispatchEvent(uMsg, wParam, lParam);

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	
	Window::Window(EventDispatcher* pDispatcher, std::string title, Size size, Pos pos, bool centered): 
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

		switch (uMsg) {
			// TODO: implement other events
			// https://learn.microsoft.com/en-us/windows/win32/inputdev

			default:
				Log::TRACE("{} Yet to be implemented", uMsg);
				break;

			case WM_CLOSE:
				pDispatcher->enqueue<WindowEvents::WindowCloseEvent>();
				break;

			case WM_MOUSEMOVE:
				pDispatcher->enqueue<MouseEvents::MouseMovedEvent>(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				break;

			case WM_LBUTTONDOWN:
				pDispatcher->enqueue<MouseEvents::MouseButtonPressedEvent>(MouseButtons::BUTTON_1 /* LOWORD(lParam), HIWORD(lParam) */);
				break;

			case WM_LBUTTONUP:
				pDispatcher->enqueue<MouseEvents::MouseButtonReleasedEvent>(MouseButtons::BUTTON_1 /* LOWORD(lParam), HIWORD(lParam)*/);
				break;

			case WM_RBUTTONDOWN:
				pDispatcher->enqueue<MouseEvents::MouseButtonPressedEvent>(MouseButtons::BUTTON_2 /* LOWORD(lParam), HIWORD(lParam)*/);
				break;

			case WM_RBUTTONUP:
				pDispatcher->enqueue<MouseEvents::MouseButtonReleasedEvent>(MouseButtons::BUTTON_2 /* LOWORD(lParam), HIWORD(lParam)*/);
				break;

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
	Window::Size Window::size() const {
		RECT rect;
		GetClientRect(pNativeHandle, &rect);
		return { rect.right - rect.left, rect.bottom - rect.top };
	}
	void Window::move(Window::Pos pos) {
		SetWindowPos(pNativeHandle, NULL, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
	Window::Size Window::position() const {
		RECT rect;
		GetWindowRect(pNativeHandle, &rect);
		return { rect.left, rect.top };
	}
	Window::Size Window::screenSize() const {
		RECT rect;
		GetClientRect(GetDesktopWindow(), &rect);
		return { rect.right - rect.left, rect.bottom - rect.top };
	}

}
