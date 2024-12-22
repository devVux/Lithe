#version 420 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;

layout(std140, binding = 0) uniform CameraBuffer {
    mat4 uViewProjection;
};

out vec4 vColor;

void main() {
    vColor = color;
    gl_Position = uViewProjection * vec4(position, 0.0, 1.0);
}