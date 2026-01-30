#include "EventDispatcher.hpp"
#include "Events/WindowEvents.hpp"
#include "MyWindow.hpp"
#include "RenderSystem.hpp"

#include <GLFW/glfw3.h>
#include <Log.hpp>

using namespace Lithe;

int main() {
	LT_LOG_TRACE("Hello World");

	EventDispatcher dispatcher;
	MyWindow		w;
	if (not w.init(dispatcher, 800, 600, "Window")) {
		LT_LOG_CRITICAL("Could not init window");
		return 1;
	}

	using Extension = const char*;

	uint32_t	 glfwExtensionCount = 0;
	const char** glfwExts			= glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<Extension> glfwExtensions(glfwExts, glfwExts + glfwExtensionCount);
#if defined(GLFW_EXPOSE_NATIVE_X11) || defined(GLFW_EXPOSE_NATIVE_WAYLAND)
	glfwExtensions.push_back("VK_KHR_xlib_surface");
#endif

	bool running = true;

  	dispatcher.on<WindowEvents::WindowClosedEvent>([&running](auto &&e) {
		running = false;
		return true;
	});

	RenderSystem system;
	if (not system.init(w, {glfwExtensions.begin(), glfwExtensions.end()})) {
		LT_LOG_CRITICAL("Could not init renderer");
		return 1;
	}

	while (running) {
		system.render();
		w.update(dispatcher);
	}

	LT_LOG_TRACE("Bye world");
}
