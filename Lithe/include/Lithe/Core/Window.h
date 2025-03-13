#pragma once

#include <string>
#include "Utils.h"

#ifdef _WIN32
	#include <wtypes.h>
	using NativeWindowHandle = struct HWND__*;
#elif defined(__APPLE__)
	using NativeWindowHandle = class NSWindow;
#else
	using NativeWindowHandle = void*; // Fallback or Linux-specific type
#endif

namespace Lithe {
	class EventDispatcher;

	class Window {

		public:
			
			using Size = Extent<long>;
			using Pos = Size;

			Window(EventDispatcher* dispatcher, std::string title = "Simple Window", Size size = { 800, 600 }, Pos position = { 0, 0 }, bool centered = true);
			Window(const Window& other) = delete;
			Window(Window&&) = delete;
			Window& operator=(const Window&) = delete;
			Window& operator=(Window&&) = delete;
			~Window();

			
			void processEvents() const;
			void show() const;
			void hide() const;
			void resize(Size size);
			void move(Pos pos);
			void rename(const std::string& title);


			[[nodiscard]] NativeWindowHandle handle() const { return pNativeHandle; }
			[[nodiscard]] Size size() const;
			[[nodiscard]] Size screenSize() const;
			[[nodiscard]] Size position() const;
			[[nodiscard]] std::string title() const;
			[[nodiscard]] bool shouldClose() const;
							

			#ifdef _WIN32
				void dispatchEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);
			#elif defined(__APPLE__)
			#else
			#endif

		private:

			EventDispatcher* pDispatcher;
			NativeWindowHandle pNativeHandle;
			bool mCentered;

	};

}
