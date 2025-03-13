#pragma once

#include <string>
#include "Utils.h"

#ifdef _WIN32
	#include <windows.h>
	using NativeWindowHandle = HWND;
#elif defined(__APPLE__)
	using NativeWindowHandle = class NSWindow;
#else
	using NativeWindowHandle = void*; // Fallback or Linux-specific type
#endif

namespace Lithe {
	class EventDispatcher;


	class Window {

		class WinImpl;

		public:
			
			using Size = Extent<int32_t>;
			using Pos = Size;

			Window(EventDispatcher* dispatcher, std::string title = "Simple Window", Size size = { 800, 600 }, Pos position = { -1, -1 });
			Window(const Window& other) = delete;
			Window(Window&&) = delete;
			Window& operator=(const Window&) = delete;
			Window& operator=(Window&&) = delete;
			~Window() = default;

			void init(EventDispatcher* dispatcher);

			void processEvents() const;
			
			void show() const;
			void hide() const;
			void resize(Size size);
			void move(Pos pos);
			void rename(const std::string& title);


			[[nodiscard]] virtual NativeWindowHandle handle() const;
			[[nodiscard]] Size size() const;
			[[nodiscard]] Size screenSize() const;
			[[nodiscard]] Size position() const;
			[[nodiscard]] std::string title() const;
							

		private:

			WinImpl* pImpl;

	};

}
