#version 460
#extension GL_ARB_shading_language_include: require

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tangent;
layout(location = 3) in vec2 in_uv;

#include "../../assets/shaders/renderdefs.h"
UNIFORM_BLOCK(0, SceneData) scene = SceneData_block.data;

layout(location = 0) out vec2 out_uv;

void main() {
    vec4 temp_position = scene.projection * scene.view * vec4(in_position * 1e4, 1.0);
    gl_Position = temp_position.xyww;
    out_uv = in_uv;
}