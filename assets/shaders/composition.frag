#version 460

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec4 out_composition;

layout(binding = 0) uniform sampler2D position_attachment;

void main() {
    out_composition = texture(position_attachment, in_uv);
}