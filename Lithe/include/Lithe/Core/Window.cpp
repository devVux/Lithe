#include <Lithe/Core/Window.h>

#include <Lithe/Core/Log.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <LLGL/Platform/NativeHandle.h>
#include <Events/Event.h>

namespace Lithe {

	static void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		auto dispatcher = reinterpret_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));

		switch (action) {
			case GLFW_PRESS:
				dispatcher->enqueue(KeyEvents::KeyPressedEvent(key, scancode, mods));
				break;
			case GLFW_RELEASE:
				dispatcher->enqueue(KeyEvents::KeyReleasedEvent(key, scancode, mods));
				break;
			case GLFW_REPEAT:
				dispatcher->enqueue(KeyEvents::KeyRepeatEvent(key, scancode, mods));
				break;
		}
	}

	static void onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		auto dispatcher = reinterpret_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));

		if (action == GLFW_PRESS) {
			dispatcher->enqueue(MouseEvents::MouseButtonPressedEvent(button, mods));
		} else {
			dispatcher->enqueue(MouseEvents::MouseButtonReleasedEvent(button, mods));
		}
	}

	static void onMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
		auto dispatcher = reinterpret_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
		dispatcher->enqueue(MouseEvents::MouseScrollEvent(xoffset, yoffset));
	}

	static void onCursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
		auto dispatcher = reinterpret_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
		dispatcher->enqueue(MouseEvents::MouseMovedEvent(xpos, ypos));
	}

	static void onSetWindowSizeCallback(GLFWwindow* window, int width, int height) {
		auto dispatcher = reinterpret_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
		dispatcher->enqueue(WindowEvents::WindowResizeEvent(width, height));
	}

	static void onWindowCloseCallback(GLFWwindow* window) {
		auto dispatcher = reinterpret_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
		dispatcher->enqueue(WindowEvents::WindowCloseEvent());
	}

	static void onSetFramebufferSizeCallback(GLFWwindow* window, int width, int height) {
		auto dispatcher = reinterpret_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
		dispatcher->enqueue(WindowEvents::FramebufferResizeEvent(width, height));
	}



	void Window::init(std::shared_ptr<EventDispatcher> dispatcher) {
		glfwSetWindowUserPointer(pWindow, reinterpret_cast<void*>(dispatcher.get()));

		glfwSetKeyCallback(pWindow, onKeyCallback);
		glfwSetMouseButtonCallback(pWindow, onMouseButtonCallback);
		glfwSetScrollCallback(pWindow, onMouseScrollCallback);
		glfwSetCursorPosCallback(pWindow, onCursorPosCallback);
		glfwSetWindowSizeCallback(pWindow, onSetWindowSizeCallback);
		glfwSetWindowCloseCallback(pWindow, onWindowCloseCallback);
		glfwSetFramebufferSizeCallback(pWindow, onSetFramebufferSizeCallback);
	}

	Window::~Window() {
		glfwDestroyWindow(pWindow);
	}

	bool Lithe::Window::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) {
		if (nativeHandle != nullptr && nativeHandleSize == sizeof(LLGL::NativeHandle)) {
			auto* handle = reinterpret_cast<LLGL::NativeHandle*>(nativeHandle);
			
			#if defined(__APPLE__)
				handle->window = glfwGetCocoaWindow(pWindow);
			#elif defined(_WIN32)
				handle->window = glfwGetWin32Window(pWindow);
			#elif defined(__linux__)
				handle->window = glfwGetX11Window(pWindow);
			#endif

			return true;
		}
		return false;
	}

	bool Window::AdaptForVideoMode(LLGL::Extent2D* resolution, bool* fullscreen) {
		mSize = *resolution;
		glfwSetWindowSize(pWindow, mSize.width, mSize.height);
		return true;
	}

	void Window::ResetPixelFormat() {
		glfwDestroyWindow(pWindow);
		pWindow = CreateGLFWWindow();
	}
	
	GLFWwindow* Window::CreateGLFWWindow() {
		auto window = glfwCreateWindow(mSize.width, mSize.height, mTitle.c_str(), nullptr, nullptr);
		if (!window)
			Lithe::Log::FATAL("Could not create the window");

		return window;
	}

}