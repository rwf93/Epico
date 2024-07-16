#version 450

layout(location = 0) in vec3 in_vertex;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec3 frag_color;

void main() {
    gl_Position = vec4(in_vertex, 1.0);
    frag_color = in_color;
}