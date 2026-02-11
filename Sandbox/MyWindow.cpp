#include "MyWindow.hpp"

#include "EventDispatcher.hpp"
#include "ISurface.hpp"

#include <Events/KeyEvents.hpp>
#include <Events/MouseEvents.hpp>
#include <Events/WindowEvents.hpp>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

using namespace Lithe;

namespace {

void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
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

void onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	auto* dispatcher = reinterpret_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));

	if (action == GLFW_PRESS)
		dispatcher->dispatch(MouseEvents::MouseButtonPressedEvent(static_cast<Button>(button), static_cast<Key>(mods)));
	else
		dispatcher->dispatch(
			MouseEvents::MouseButtonReleasedEvent(static_cast<Button>(button), static_cast<Key>(mods))
		);
}

void onMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	auto* dispatcher = reinterpret_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
	dispatcher->dispatch(
		MouseEvents::MouseWheelEvent({static_cast<uint32_t>(xoffset), static_cast<uint32_t>(yoffset)})
	);
}

void onCursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	auto* dispatcher = reinterpret_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
	dispatcher->dispatch(MouseEvents::MouseMovedEvent({static_cast<uint32_t>(xpos), static_cast<uint32_t>(ypos)}));
}

void onSetWindowSizeCallback(GLFWwindow* window, int width, int height) {
	auto dispatcher = reinterpret_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
	dispatcher->dispatch(
		WindowEvents::WindowResizedEvent({static_cast<uint32_t>(width), static_cast<uint32_t>(height)})
	);
}

void onWindowCloseCallback(GLFWwindow* window) {
	auto* dispatcher = reinterpret_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
	dispatcher->dispatch(WindowEvents::WindowClosedEvent {});
}

} // namespace

bool MyWindow::init(EventDispatcher& dispatcher, uint32_t width, uint32_t height, std::string title) noexcept {

#ifdef GLFW_EXPOSE_NATIVE_WIN32
	glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WIN32);
#elif defined(GLFW_EXPOSE_NATIVE_X11)
	glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#elif defined(GLFW_EXPOSE_NATIVE_WAYLAND)
	glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
#endif

	if (!glfwInit())
		return false;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	pWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

	if (!pWindow)
		return false;

	glfwSetWindowUserPointer(pWindow, static_cast<Lithe::EventDispatcher*>(&dispatcher));

	glfwSetKeyCallback(pWindow, onKeyCallback);
	glfwSetMouseButtonCallback(pWindow, onMouseButtonCallback);
	glfwSetScrollCallback(pWindow, onMouseScrollCallback);
	glfwSetCursorPosCallback(pWindow, onCursorPosCallback);
	glfwSetWindowSizeCallback(pWindow, onSetWindowSizeCallback);
	glfwSetWindowCloseCallback(pWindow, onWindowCloseCallback);

	return true;
}

MyWindow::~MyWindow() noexcept {
	glfwDestroyWindow(pWindow);
	glfwTerminate();
}

Lithe::NativeHandle MyWindow::native() const noexcept {
#ifdef GLFW_EXPOSE_NATIVE_WIN32
	return {.handle = glfwGetWin32Window(pWindow)};
#elif GLFW_EXPOSE_NATIVE_X11
	return {.handle = {.id = glfwGetX11Window(pWindow)}, .display = glfwGetX11Display()};
#elif GLFW_EXPOSE_NATIVE_WAYLAND
	return {.handle = glfwGetWaylandWindow(pWindow), .display = glfwGetWaylandDisplay()};
#endif

	return {};
}

void MyWindow::update(EventDispatcher&) const noexcept {
	if (!glfwWindowShouldClose(pWindow))
		glfwPollEvents();
}

Lithe::Size MyWindow::size() const noexcept {
	int width  = 0;
	int height = 0;

	glfwGetWindowSize(pWindow, &width, &height);

	return {.width = static_cast<uint32_t>(width), .height = static_cast<uint32_t>(height)};
}
