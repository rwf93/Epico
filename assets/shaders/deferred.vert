#version 460
#extension GL_ARB_shading_language_include: require

layout(location = 0) in vec3 in_vertex;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tangent;
layout(location = 3) in vec2 in_uv;

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec3 out_tangent;
layout(location = 3) out vec2 out_uv;

#include "../../assets/shaders/renderdefs.h"
UNIFORM_BLOCK(0, SceneData) scene = SceneData_block.data;

layout(std140, set = 0, binding = 1) readonly buffer StorageDataUniform {
    StorageData objects[];
} storage;

void main() {
    StorageData object = storage.objects[gl_InstanceIndex];
    gl_Position = scene.projection * scene.view * object.model * vec4(in_vertex, 1.0);
    mat3 normal = transpose(inverse(mat3(object.model)));

    out_position = vec3(object.model * vec4(in_vertex, 1.0));
    out_normal = normal * normalize(in_normal);
    out_tangent = normal * normalize(in_tangent);
    out_uv = in_uv;
}