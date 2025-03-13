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

	class Window::WinImpl {

		public:

			HWND handle;
			HINSTANCE instance;
			EventDispatcher* dispatcher;

			WinImpl(EventDispatcher* disp, std::string_view title, Window::Size size, Window::Pos pos): dispatcher(disp) {
				instance = GetModuleHandle(nullptr);

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
				handle = CreateWindowEx(
					0, CLASS_NAME, title.data(), WS_OVERLAPPEDWINDOW,
					pos.x, pos.y, size.width, size.height,
					NULL, NULL, instance, this
				);

				if (handle == NULL) {
					DWORD error = GetLastError();
					Log::FATAL("Could not create window, error code: {}", error);
				}

				// Register window pointer with the handle
				SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

			}

			~WinImpl() {
				if (handle) {
					DestroyWindow(handle);
					handle = NULL;
				}
			}

			void processEvents() {
				MSG msg = { 0 };
				if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
					if (msg.message == WM_QUIT)
						return;

					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}

			static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
				WinImpl* impl = reinterpret_cast<WinImpl*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

				if (impl) {
					// Let the instance handle the event first
					impl->dispatchEvent(uMsg, wParam, lParam);
				}

				// Default handling for unprocessed messages
				switch (uMsg) {
					case WM_CLOSE:
						DestroyWindow(hwnd);
						return 0;

					case WM_DESTROY:
						PostQuitMessage(0);
						return 0;
				}

				return DefWindowProc(hwnd, uMsg, wParam, lParam);
			}

			bool shouldClose() const { return !IsWindow(handle); }
			void show() const { ShowWindow(handle, SW_SHOW); }
			void hide() const { ShowWindow(handle, SW_HIDE); }
			void rename(const std::string& title) { SetWindowText(handle, title.c_str()); }

			std::string title() const {
				char buffer[256];
				GetWindowText(handle, buffer, sizeof(buffer));
				return std::string(buffer);
			}
			void resize(Window::Size size) {
				SetWindowPos(handle, NULL, 0, 0, size.width, size.height, SWP_NOMOVE | SWP_NOZORDER);
			}
			Extent<long> size() const {
				RECT rect;
				GetClientRect(handle, &rect);
				return { rect.right - rect.left, rect.bottom - rect.top };
			}
			void move(Window::Pos pos) {
				SetWindowPos(handle, NULL, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
			}
			Extent<long> position() const {
				RECT rect;
				GetWindowRect(handle, &rect);
				return { rect.left, rect.top };
			}
			Extent<long> screenSize() const {
				RECT rect;
				GetClientRect(GetDesktopWindow(), &rect);
				return { rect.right - rect.left, rect.bottom - rect.top };
			}


			void dispatchEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) {
			
				switch(uMsg) {
					// TODO: implement other events
					// https://learn.microsoft.com/en-us/windows/win32/inputdev
				
					default:
						Log::TRACE("{} Yet to be implemented", uMsg);
						break;
					
					case WM_CLOSE:
						dispatcher->enqueue<WindowEvents::WindowCloseEvent>();
						break;

					case WM_MOUSEMOVE:
						dispatcher->enqueue<MouseEvents::MouseMovedEvent>(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
						break;

					case WM_LBUTTONDOWN:
						dispatcher->enqueue<MouseEvents::MouseButtonPressedEvent>(MouseButtons::BUTTON_1 /* LOWORD(lParam), HIWORD(lParam) */);
						break;

					case WM_LBUTTONUP:
						dispatcher->enqueue<MouseEvents::MouseButtonReleasedEvent>(MouseButtons::BUTTON_1 /* LOWORD(lParam), HIWORD(lParam)*/ );
						break;

					case WM_RBUTTONDOWN:
						dispatcher->enqueue<MouseEvents::MouseButtonPressedEvent>(MouseButtons::BUTTON_2 /* LOWORD(lParam), HIWORD(lParam)*/);
						break;

					case WM_RBUTTONUP:
						dispatcher->enqueue<MouseEvents::MouseButtonReleasedEvent>(MouseButtons::BUTTON_2 /* LOWORD(lParam), HIWORD(lParam)*/);
						break;

				}

			}

	};
	
	Window::Window(EventDispatcher* dispatcher, std::string title, Size size, Pos pos):
		pImpl(new WinImpl(dispatcher, title, size, pos)) {
		if (pos.x == -1 && pos.y == -1) {
			Size screen = pImpl->screenSize();
			Size window = pImpl->size();

			Size center(
				(screen.width - window.width) / 2,
				(screen.height - window.height) / 2
			);

			pImpl->move(center);
		}

		pImpl->show();
	}

	void Window::processEvents() const { pImpl->processEvents(); }

	void Window::show() const { pImpl->show(); }  
	void Window::hide() const { pImpl->hide(); }
	void Window::resize(Size size) { pImpl->resize(size); }
	void Window::move(Pos pos) { pImpl->move(pos); }
	void Window::rename(const std::string& title) { pImpl->rename(title); }
	
	NativeWindowHandle Window::handle() const { return pImpl->handle; }
	Window::Size Window::size() const { return pImpl->size(); }
	Window::Size Window::screenSize() const { return pImpl->screenSize(); }
	Window::Size Window::position() const { return pImpl->position(); }
	std::string Window::title() const { return pImpl->title(); }

}
