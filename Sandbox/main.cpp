#include <Log.hpp>
#include <EventDispatcher.hpp>
#include <Events/WindowEvents.hpp>
#include <RenderSystem.hpp>
#include <Scene.hpp>
#include <PerspectiveCamera.hpp>
#include <Clock.hpp>
#include <RenderPacket.hpp>
#include <ResourceCache.hpp>
#include <GLFW/glfw3.h>

#include "MyWindow.hpp"
#include "MyInput.hpp"

#include <memory>

using namespace Lithe;

int main() {
	LT_LOG_TRACE("Hello World");

	EventDispatcher dispatcher;
	MyWindow		w;
	if (not w.init(dispatcher, 800, 600, "Window")) {
		LT_LOG_CRITICAL("Could not init window");
		return 1;
	}
	MyInput input(w.glfwHandle());

	using Extension = const char*;

	uint32_t	 glfwExtensionCount = 0;
	const char** glfwExts			= glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<Extension> glfwExtensions(glfwExts, glfwExts + glfwExtensionCount);
#if defined(GLFW_EXPOSE_NATIVE_X11) || defined(GLFW_EXPOSE_NATIVE_WAYLAND)
	glfwExtensions.push_back("VK_KHR_xlib_surface");
#elif defined(GLFW_EXPOSE_NATIVE_WAYLAND)
	glfwExtensions.push_back("VK_KHR_wayland_surface");
#endif

	bool running = true;

  	dispatcher.on<WindowEvents::WindowClosedEvent>([&running](auto &&e) {
		running = false;
		return true;
	});



	ResourceCache cache;
	cache.addMesh({
		.position = {
			{-3.0f, -3.0f, -1.0f},
			{ 3.0f, -3.0f, -1.0f},
			{ 3.0f,  3.0f, -1.0f},
			{-3.0f,  3.0f, -1.0f},
			{-3.0f, -3.0f,  1.0f},
			{ 3.0f, -3.0f,  1.0f},
			{ 3.0f,  3.0f,  1.0f},
			{-3.0f,  3.0f,  1.0f}
		},
		.indices = {
			0,1,2, 2,3,0,
			4,5,6, 6,7,4,
			0,4,7, 7,3,0,
			1,5,6, 6,2,1,
			3,2,6, 6,7,3,
			0,1,5, 5,4,0
		}
	});

	RenderSystem system;
	if (not system.init(w, {glfwExtensions.begin(), glfwExtensions.end()})) {
		LT_LOG_CRITICAL("Could not init renderer");
		return 1;
	}

	StaticRenderPacket statics;
	statics.meshes.push_back(0);
	system.uploadStaticData(statics, cache);

	DynamicRenderPacket dynamics;
	dynamics.camera = glm::mat4(1.0f);

	Scene scene;
	scene.setCamera(
		std::make_unique<PerspectiveCamera>(
			glm::vec3(0.0f, 0.0f, 2.0f),
			glm::vec3(0.0f, 0.0f, -1.0f)
		)
	);



	Clock clock(60);
	double acc = 0.0;

	while (running) {
		acc += clock.timeSinceLastUpdate();

		while (acc >= clock.tickInterval()) {
			scene.update(clock.tickInterval(), input);
			acc -= clock.tickInterval();
		}

		dynamics.camera = scene.camera();
		system.render(dynamics, cache);
		w.update(dispatcher);
	}


	LT_LOG_TRACE("Bye world");
}
