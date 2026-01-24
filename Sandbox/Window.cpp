#include "Window.hpp"

#include "EventDispatcher.hpp"
#include "ISurface.hpp"

#include <Events/KeyEvents.hpp>
#include <Events/MouseEvents.hpp>
#include <Events/WindowEvents.hpp>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <expected>

using namespace Lithe;

static void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	auto dispatcher = reinterpret_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));

	switch (action) {
		case GLFW_PRESS:
			dispatcher->dispatch(
				KeyEvents::KeyPressedEvent(static_cast<Key>(key), static_cast<Key>(scancode), static_cast<Key>(mods))
			);
			break;
		case GLFW_RELEASE:
			dispatcher->dispatch(
				KeyEvents::KeyReleasedEvent(static_cast<Key>(key), static_cast<Key>(scancode), static_cast<Key>(mods))
			);
			break;
		case GLFW_REPEAT:
			dispatcher->dispatch(
				KeyEvents::KeyRepeatEvent(static_cast<Key>(key), static_cast<Key>(scancode), static_cast<Key>(mods))
			);
			break;
	}
}

static void onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	auto* dispatcher = reinterpret_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));

	if (action == GLFW_PRESS)
		dispatcher->dispatch(MouseEvents::MouseButtonPressedEvent(static_cast<Button>(button), static_cast<Key>(mods)));
	else
		dispatcher->dispatch(
			MouseEvents::MouseButtonReleasedEvent(static_cast<Button>(button), static_cast<Key>(mods))
		);
}

static void onMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	auto* dispatcher = reinterpret_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
	dispatcher->dispatch(
		MouseEvents::MouseWheelEvent({static_cast<uint32_t>(xoffset), static_cast<uint32_t>(yoffset)})
	);
}

static void onCursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	auto* dispatcher = reinterpret_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
	dispatcher->dispatch(MouseEvents::MouseMovedEvent({static_cast<uint32_t>(xpos), static_cast<uint32_t>(ypos)}));
}

static void onSetWindowSizeCallback(GLFWwindow* window, int width, int height) {
	auto dispatcher = reinterpret_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
	dispatcher->dispatch(
		WindowEvents::WindowResizedEvent({static_cast<uint32_t>(width), static_cast<uint32_t>(height)})
	);
}

static void onWindowCloseCallback(GLFWwindow* window) {
	auto* dispatcher = reinterpret_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
	dispatcher->dispatch(WindowEvents::WindowClosedEvent {});
}

std::expected<std::monostate, std::string>
Window::init(EventDispatcher& dispatcher, uint32_t width, uint32_t height, std::string title) noexcept {
	glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);

	if (!glfwInit())
		return std::unexpected {"Could not init glfw"};

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	pWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

	wl_display* display = glfwGetWaylandDisplay();
	if (!display || !pWindow)
		return std::unexpected {"Error on window creation"};

	glfwSetWindowUserPointer(pWindow, static_cast<Lithe::EventDispatcher*>(&dispatcher));

	glfwSetKeyCallback(pWindow, onKeyCallback);
	glfwSetMouseButtonCallback(pWindow, onMouseButtonCallback);
	glfwSetScrollCallback(pWindow, onMouseScrollCallback);
	glfwSetCursorPosCallback(pWindow, onCursorPosCallback);
	glfwSetWindowSizeCallback(pWindow, onSetWindowSizeCallback);
	glfwSetWindowCloseCallback(pWindow, onWindowCloseCallback);

	return {};
}

Window::~Window() noexcept {
	glfwDestroyWindow(pWindow);
	glfwTerminate();
}

Lithe::NativeHandle Window::handle() const noexcept {
	return static_cast<void*>(glfwGetWaylandWindow(pWindow));
}

Lithe::NativeHandle Window::display() const noexcept {
#ifdef LT_WAYLAND
	return static_cast<void*>(glfwGetWaylandDisplay());
#elif defined(LT_X11)
	return static_cast<void*>(glfwGetX11Display());
#endif
}

void Window::update(EventDispatcher&) const noexcept {
	if (!glfwWindowShouldClose(pWindow))
		glfwPollEvents();
}
