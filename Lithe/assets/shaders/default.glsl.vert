#version 420 core

layout(location = 0) in vec2 position;

layout(std140, binding = 0) uniform CameraBuffer {
    mat4 uViewProjection;
};

layout(std140, binding = 1) uniform EntityBuffer {
    mat4 uTransforms[100];
    vec4 uColors[100];
};

out vec4 vColor;

void main() {
    vColor = uColors[gl_InstanceID];

    mat4 model = uTransforms[gl_InstanceID];
    gl_Position = uViewProjection * model * vec4(position, 0.0, 1.0);
}
