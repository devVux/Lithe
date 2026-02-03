#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTextCoord;

layout(binding = 0) uniform UBO {
    mat4 viewProjection;
} ubo;

void main() {
    gl_Position = ubo.viewProjection * vec4(inPosition, 1.0);
}
