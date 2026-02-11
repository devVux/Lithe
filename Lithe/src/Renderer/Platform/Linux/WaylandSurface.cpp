#include "SurfaceFactory.hpp"

// clang-format off
#include <wayland-client.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_wayland.h>
// clang-format on

namespace Lithe::SurfaceFactory {
std::expected<VkSurfaceKHR, E> createSurface(VkInstance instance, ISurface& surface) {

	VkSurfaceKHR vkSurface;

	VkWaylandSurfaceCreateInfoKHR info {
		.sType	 = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
		.display = static_cast<wl_display*>(surface.native().display),
		.surface = static_cast<wl_surface*>(surface.native().handle.ptr),
	};

	auto res = vkCreateWaylandSurfaceKHR(instance, &info, nullptr, &vkSurface);
	if (res != VK_SUCCESS)
		return std::unexpected(E::Unknown);

	return vkSurface;
}

} // namespace Lithe::SurfaceFactory
