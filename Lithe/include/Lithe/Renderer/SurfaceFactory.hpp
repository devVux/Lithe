#pragma once

#include "ErrorType.hpp"
#include "ForwardDecls.hpp"
#include "ISurface.hpp"

#include <expected>

namespace Lithe::SurfaceFactory {

std::expected<VkSurfaceKHR, E> createSurface(VkInstance instance, ISurface& surface);

}
