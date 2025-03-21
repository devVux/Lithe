#pragma once

#include "Event.h"
#include "Utils.h"

namespace Lithe {
	class Window;

	class Input {

		public:

			static void setWindow(SharedPtr<Window> window) {
				pWindow = std::move(window);
			}

			static bool isKeyUp(Key code) { 
				return isKey(code, State::RELEASED);
			}
			static bool isKeyDown(Key code) {  
				return isKey(code, State::PRESSED);
			}

			static bool isMouseUp(Button button) {
				return isMouse(button, State::RELEASED);
			}
			static bool isMouseButtonDown(Button button) {
				return isMouse(button, State::PRESSED);
			}

			static MousePos mousePos();


			static bool isKey(Key code, State state);
			static bool isMouse(Button button, State state);

		private:

			static inline SharedPtr<Lithe::Window> pWindow { nullptr };
		
	};
}
