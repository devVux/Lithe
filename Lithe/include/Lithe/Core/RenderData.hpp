#pragma once

#include "IDs.hpp"

#include <glm/glm.hpp>
#include <vector>

namespace Lithe {

struct MeshData {
	std::vector<glm::vec3> positions;
	std::vector<uint32_t> indices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;
};

struct MaterialData {
	glm::vec4 color{ 1.0f };
	TextureID albedoTextureID;
};

struct TextureData {
	uint32_t width;
	uint32_t height;
	std::vector<uint8_t> pixels;
};

struct InstanceData {
	glm::mat4 model{ 1.0f };
	MeshID meshID;
	MaterialID materialID;
	TextureID textureID;
};


}
