#version 460

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tangent;
layout(location = 3) in vec2 in_uv;

layout(location = 0) out vec4 out_position;
layout(location = 1) out vec4 out_normal;
layout(location = 2) out vec4 out_albedo;

layout(binding = 2) uniform sampler2D in_albedo_texture;
layout(binding = 3) uniform sampler2D in_normal_texture;

void main() {
    out_position = vec4(in_position, 1.0);

    vec3 N = normalize(in_normal);
    vec3 T = normalize(in_tangent);
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);
    vec3 tangent_normalized = TBN * normalize(texture(in_normal_texture, in_uv).xyz * 2.0 - vec3(1.0));

    out_normal = vec4(tangent_normalized, 1.0);
    out_albedo = texture(in_albedo_texture, in_uv);
}