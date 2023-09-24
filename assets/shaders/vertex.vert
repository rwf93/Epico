#version 460

#include "primitives.h"

layout(binding = 0) uniform EGlobalData {
    mat4 view;
    mat4 projection;
} global;

layout(std140, set = 1, binding = 0) readonly buffer EObjectBuffer {
    EObjectData objects[];
} object_buffer;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec3 in_normal;

layout(location = 0) out vec3 out_color;
layout(location = 2) out vec3 out_normal;

void main() {
    mat4 model_matrix = object_buffer.objects[gl_BaseInstance].model;

    gl_Position = global.projection * global.view * model_matrix * vec4(in_position, 1.0);
    out_color = in_color;
    out_normal = in_normal;
}