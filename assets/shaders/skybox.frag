#version 460
#extension GL_GOOGLE_include_directive: require

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

#include "../../assets/shaders/renderdefs.h"
UNIFORM_BLOCK(0, SceneData) scene = SceneData_block.data;

layout(binding = 1) uniform sampler2D skybox_texture;

void main() {
    out_color = vec4(texture(skybox_texture, in_uv).rgb, 1.0);
}