#version 460

layout(location = 0) in vec3 in_vertex;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec3 out_vertex;
layout(location = 1) out vec3 out_color;

layout(binding = 0) uniform SceneData {
    mat4 view;
    mat4 projection;
    mat4 model;
    vec3 color;
} scene;

struct StorageData {
    mat4 model;
};

layout(std140, set = 0, binding = 1) readonly buffer StorageDataUniform {
    StorageData objects[];
} storage;

void main() {
    gl_Position = scene.projection * scene.view * storage.objects[gl_BaseInstance].model * vec4(in_vertex, 1.0);
    out_vertex = in_vertex;
    out_color = scene.color;
}