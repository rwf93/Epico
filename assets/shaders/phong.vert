#version 460

#include "primitives.h"

layout(binding = 0) uniform ECameraData {
    mat4 view;
    mat4 proj;
} camera;

layout(std140, set = 1, binding = 0) readonly buffer EObjectBuffer {
    EObjectData objects[];
} object_buffer;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec3 in_normal;

layout(location = 0) out vec3 out_color;
layout(location = 2) out vec3 out_normal;

layout(location = 3) out vec3 out_view;
layout(location = 4) out vec3 out_light;

void main() {
    mat4 model_matrix = object_buffer.objects[gl_BaseInstance].model;

    gl_Position = camera.proj * camera.view * model_matrix * vec4(in_position, 1.0);
    out_color = in_color;
    out_normal = in_normal;

    vec4 pos = model_matrix * vec4(in_position, 1.0);
    out_normal = mat3(model_matrix) * in_normal;
    vec3 lpos = mat3(model_matrix) * vec3(0, 0, 0);

    out_light = lpos - pos.xyz;
    out_view = -pos.xyz;
}