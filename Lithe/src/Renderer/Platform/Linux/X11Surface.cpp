#include "SurfaceFactory.hpp"

#include <X11/Xlib.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xlib.h>

namespace Lithe::SurfaceFactory {
std::expected<VkSurfaceKHR, E> createSurface(VkInstance instance, ISurface& surface) {

	VkSurfaceKHR vkSurface;

	VkXlibSurfaceCreateInfoKHR info = {};
	info.sType						= VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	info.dpy						= static_cast<Display*>(surface.native().display);
	info.window						= surface.native().handle.id;
	auto res						= vkCreateXlibSurfaceKHR(instance, &info, nullptr, &vkSurface);
	if (res != VK_SUCCESS)
		return std::unexpected(E::Unknown);

	return vkSurface;
}

} // namespace Lithe::SurfaceFactory
