#version 460

layout(location = 0) in vec3 in_vertex;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec4 out_position;
layout(location = 1) out vec3 out_albedo;

layout(binding = 2) uniform sampler2D missing_texture;

void main() {
    out_position = vec4(in_vertex, 1.0);
    out_albedo = texture(missing_texture, in_uv).rgb;
}