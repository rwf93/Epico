#version 460

layout(location = 0) in vec3 in_vertex;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec3 out_vertex;
layout(location = 1) out vec3 out_color;


void main() {
    gl_Position = vec4(in_vertex, 1.0);
    out_vertex = in_vertex;
    out_color = in_color;
}