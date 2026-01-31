#include "SurfaceFactory.hpp"

#include <wayland-client.h>
#include <vulkan/vulkan_wayland.h>

namespace Lithe { 
std::expected<VkSurfaceKHR, Error> createSurface(VkInstance instance, ISurface& surface) { }

	VkSurfaceKHR vkSurface;

	VkXlibSurfaceCreateInfoKHR info = {};
	info.sType						= VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	info.dpy						= static_cast<Display*>(surface.native().display);
	info.window						= surface.native().handle.id;
	auto res						= vkCreateXlibSurfaceKHR(instance, &info, nullptr, &vkSurface);
	if (res != VK_SUCCESS)
		return std::unexpected(Error::Unknown);

	return vkSurface;
}
