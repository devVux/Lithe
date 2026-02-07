#pragma once

#include "RenderData.hpp"

#include <vector>
#include <glm/glm.hpp>

namespace Lithe {

struct StaticRenderPacket {
	std::vector<InstanceData> instances;
};

struct DynamicRenderPacket {
	glm::mat4 camera;
};

}
