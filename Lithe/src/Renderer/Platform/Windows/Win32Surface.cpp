#include "SurfaceFactory.hpp"

#include <windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

namespace Lithe::SurfaceFactory {

std::expected<VkSurfaceKHR, E> createSurface(VkInstance instance, ISurface& surface) {
	VkSurfaceKHR vkSurface;
	VkWin32SurfaceCreateInfoKHR info = {};
	info.sType						 = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	info.hinstance					 = GetModuleHandle(NULL);
	info.hwnd						 = static_cast<HWND>(surface.native().handle.ptr);
	auto res						 = vkCreateWin32SurfaceKHR(instance, &info, nullptr, &vkSurface);

	if (res != VK_SUCCESS)
		return std::unexpected(E::Unknown);

	return vkSurface;
}
}
