#pragma once

#include "Lithe/Core/Window.h"

namespace Lithe {

	struct MouseCoords {
		double x, y;
	};

	class Input {

		public:

			static bool isKeyDown(Keys keyCode) {
				return glfwGetKey((GLFWwindow*) pWindow->native(), keyCode) == GLFW_PRESS;
			}

			static bool isMouseButtonDown(MouseButtons button) {
				return glfwGetMouseButton((GLFWwindow*) pWindow->native(), button) == GLFW_PRESS;
			}

			static MouseCoords mousePos() {
				double x, y;
				glfwGetCursorPos((GLFWwindow*) pWindow->native(), &x, &y);
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