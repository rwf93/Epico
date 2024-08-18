#version 460

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

layout(binding = 1) uniform sampler2D skybox_texture;

void main() {
    out_color = vec4(texture(skybox_texture, in_uv).rgb, 1.0);
}