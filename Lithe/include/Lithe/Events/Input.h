#pragma once

#include "Lithe/Core/Window.h"
#include "Lithe/Events/Event.h"

namespace Lithe {

	struct MouseCoords {
		double x, y;
	};

	class Input {

		public:

			static bool isKeyDown(Keys keyCode) {
				return false; //return glfwGetKey(pWindow->handle(), keyCode) == GLFW_PRESS;
			}

			static bool isMouseButtonDown(MouseButtons button) {
				return false; //return glfwGetMouseButton(pWindow->handle(), button) == GLFW_PRESS;
			}

			static MouseCoords mousePos() {
				double x, y;
				//glfwGetCursorPos(pWindow->handle(), &x, &y);
				return { x, y };
			}

			static void setInput(Lithe::Window* window) {
				if (window != nullptr)
					pWindow = window;
			}

		private:

			static inline Lithe::Window* pWindow { nullptr };
		
	};


}
