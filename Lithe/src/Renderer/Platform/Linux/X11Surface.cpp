#include "SurfaceFactory.hpp"

#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>

namespace Lithe { 
	std::expected<VkSurfaceKHR, Error> createSurface(VkInstance instance, ISurface& surface) { }

		VkSurfaceKHR vkSurface;

		VkWaylandSurfaceCreateInfoKHR info {
			.sType	 = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
			.display = static_cast<wl_display*>(surface.native().display),
			.surface = static_cast<wl_surface*>(surface.native().handle.ptr),
		};

		auto res = vkCreateWaylandSurfaceKHR(instance, &info, nullptr, &vkSurface);
		if (res != VK_SUCCESS)
			return std::unexpected(Error::Unknown);

		return vkSurface;
}
