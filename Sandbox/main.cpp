#include "EventDispatcher.hpp"
#include "Events/WindowEvents.hpp"
#include "RenderSystem.hpp"
#include "Window.hpp"

#include <Log.hpp>

using namespace Lithe;

int main() {
  LT_LOG_TRACE("Hello World");

  EventDispatcher dispatcher;
  Window w;
  w.init(dispatcher, 800, 600, "Window");

  using Extension = const char *;

  uint32_t glfwExtensionCount = 0;
  const char **glfwExts =
      glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<Extension> glfwExtensions(glfwExts,
                                        glfwExts + glfwExtensionCount);
  glfwExtensions.push_back("VK_KHR_wayland_surface");

  bool running = true;

  dispatcher.on<WindowEvents::WindowClosedEvent>([&running](auto &&e) {
    running = false;
    return true;
  });

  RenderSystem system;
  system.init(w, {glfwExtensions.begin(), glfwExtensions.end()});

  while (running) {
    system.render();
    w.update(dispatcher);
  }

  LT_LOG_TRACE("Bye world");
}
