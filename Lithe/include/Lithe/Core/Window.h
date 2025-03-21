#pragma once

#include "Utils.h"
#include "InternalTypes.h"

#include <string>

namespace Lithe {
	class EventDispatcher;

	class Window {

		public:
			

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
				void dispatchEvent(NSNotificationName eventType, id sender);
			#else
			#endif

		private:

			EventDispatcher* pDispatcher;
			NativeWindowHandle pNativeHandle;
			bool mCentered;

	};

}
