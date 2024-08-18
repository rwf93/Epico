#version 460

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tangent;
layout(location = 3) in vec2 in_uv;

layout(binding = 0) uniform SceneData {
    mat4 view;
    mat4 projection;
} scene;

layout(location = 0) out vec2 out_uv;

void main() {
    gl_Position = scene.projection * scene.view * mat4(1) * vec4(in_position * 4, 1.0);
    out_uv = in_uv;
}