#version 450
#extension GL_EXT_nonuniform_qualifier : enable


layout(location = 0) out vec4 outColor;

layout(location = 1) flat in uint vMaterialIndex;
layout(location = 2) flat in uint vTextureIndex;
layout(location = 3) in vec2 vTextureCoord;


// Set 1 - Per material
struct MaterialData {
	vec4 color;
	uint albedoTextureIndex;
};

layout(set = 1, binding = 0) uniform sampler globalSampler;
layout(set = 1, binding = 1) uniform texture2D textures[];
	
layout(set = 1, binding = 2) readonly buffer MaterialUBO {
	MaterialData materials[];
};


void main() {
	MaterialData mat = materials[vMaterialIndex];

	vec4 matColor = texture(sampler2D(textures[nonuniformEXT(mat.albedoTextureIndex)], globalSampler), vTextureCoord) * mat.color;
	vec4 objectText = texture(sampler2D(textures[nonuniformEXT(vTextureIndex)], globalSampler), vTextureCoord);
	outColor = matColor * objectText; // plus lighting computation later

}
