#pragma once

#include "IDs.hpp"

#include <vector>
#include <glm/glm.hpp>

namespace Lithe {

// TODO: use function instead of single error-prone inserts
struct StaticRenderPacket {
	std::vector<glm::mat4> transforms;
	std::vector<MeshID> meshes;
};

struct DynamicRenderPacket {
	glm::mat4 camera;
};

}
