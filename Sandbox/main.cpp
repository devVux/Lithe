#include "MyInput.hpp"
#include "MyWindow.hpp"

#include <Clock.hpp>
#include <EventDispatcher.hpp>
#include <Events/WindowEvents.hpp>
#include <GLFW/glfw3.h>
#include <Log.hpp>
#include <PerspectiveCamera.hpp>
#include <RenderPacket.hpp>
#include <RenderSystem.hpp>
#include <ResourceCache.hpp>
#include <Scene.hpp>
#include <glm/ext/matrix_transform.hpp>
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
#if defined(GLFW_EXPOSE_NATIVE_X11)
	glfwExtensions.push_back("VK_KHR_xlib_surface");
#elif defined(GLFW_EXPOSE_NATIVE_WAYLAND)
	glfwExtensions.push_back("VK_KHR_wayland_surface");
#endif

	bool running = true;

	dispatcher.on<WindowEvents::WindowClosedEvent>([&running](auto&& e) {
		running = false;
		return true;
	});

	// clang-format off
	ResourceCache cache;
	cache.addMesh({
		.positions = {
			{-3.0f, -3.0f,  1.0f},
			{ 3.0f, -3.0f,  1.0f},
			{ 3.0f,  3.0f,  1.0f},
			{-3.0f,  3.0f,  1.0f},
			{-3.0f, -3.0f, -1.0f},
			{ 3.0f, -3.0f, -1.0f},
			{ 3.0f,  3.0f, -1.0f},
			{-3.0f,  3.0f, -1.0f}
		},
		.indices = {
			0, 1, 2, 2, 3, 0,
			4, 7, 6, 6, 5, 4,
			1, 5, 6, 6, 2, 1,
			7, 4, 0, 0, 3, 7,
			3, 2, 6, 6, 7, 3,
			0, 4, 5, 5, 1, 0
		}
	});

	cache.addMaterial({.color = glm::vec4(1.0f), .albedoTextureID = 0});

	cache.addTexture({
		.width = 1, .height = 1, .pixels = {255, 255, 255, 255}
	});

	// clang-format on

	RenderSystem system;
	if (not system.init(dispatcher, w, {glfwExtensions.begin(), glfwExtensions.end()})) {
		LT_LOG_CRITICAL("Could not init renderer");
		return 1;
	}

	StaticRenderPacket statics;
	statics.instances.push_back({.model = glm::mat4(1.0f), .meshID = 0, .materialID = 0, .textureID = 0});
	statics.instances.push_back(
		{.model = glm::translate(glm::mat4(1.0f), glm::vec3(5, -3, 0)), .meshID = 0, .materialID = 0, .textureID = 0}
	);
	system.uploadStaticData(statics, cache);

	DynamicRenderPacket dynamics;
	dynamics.camera = glm::mat4(1.0f);

	Scene scene;
	scene.setCamera(std::make_unique<PerspectiveCamera>(glm::vec3(0.0f, -5.0f, 2.0f), glm::vec3(0.0f, 0.0f, -1.0f)));

	Clock  clock(60);
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
