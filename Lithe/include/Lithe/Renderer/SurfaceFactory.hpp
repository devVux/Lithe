#pragma once

#include "ErrorType.hpp"
#include "ISurface.hpp"
#include "ForwardDecls.hpp"

#include <expected>

namespace Lithe::SurfaceFactory {

	std::expected<VkSurfaceKHR, DefaultError> createSurface(VkInstance instance, ISurface& surface);

}
