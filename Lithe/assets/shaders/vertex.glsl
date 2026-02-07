#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTextureCoord;

layout(location = 1) flat out uint vMaterialIndex;
layout(location = 2) flat out uint vTextureIndex;
layout(location = 3) out vec2 vTextureCoord;


// Set 0 - Per frame
layout(set = 0, binding = 0) uniform CameraUBO {
	mat4 viewProjection;
};


// Set 1 - Per material in fragment


// Set 2 - Per object
struct InstanceData {
    mat4 model;
    uint materialIndex;
    uint textureIndex;
};

layout(std430, set = 2, binding = 0) readonly buffer ObjectSSBO {
    InstanceData objects[];
};


void main() {
    gl_Position = viewProjection * objects[gl_InstanceIndex].model * vec4(inPosition, 1.0);

	vMaterialIndex = objects[gl_InstanceIndex].materialIndex;
	vTextureIndex = objects[gl_InstanceIndex].textureIndex;
	vTextureCoord = inTextureCoord;
}
