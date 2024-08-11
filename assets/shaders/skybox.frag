#version 460

layout(location = 1) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D skybox_texture;

void main() {
    out_color = texture(skybox_texture, in_uv);
}