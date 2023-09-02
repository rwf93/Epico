#version 460

layout(binding = 0) uniform ECameraData {
    mat4 view;
    mat4 proj;
} camera;

struct EObjectData {
    mat4 model;
};

layout(std140, set = 1, binding = 0) readonly buffer EObjectBuffer {
    EObjectData objects[];
} object_buffer;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec3 out_color;

void main() {
    mat4 model_matrix = object_buffer.objects[gl_BaseInstance].model;

    gl_Position = camera.proj * camera.view * model_matrix * vec4(in_position, 1.0);
    out_color = in_color;
}