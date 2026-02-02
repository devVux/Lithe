#pragma once

#include "IDs.hpp"

#include <glm/glm.hpp>
#include <vector>

namespace Lithe {

struct MeshData {
	std::vector<glm::vec3> position;
	std::vector<glm::vec3> normal;
	std::vector<glm::vec2> uv;
	std::vector<uint32_t> indices;
};

}
