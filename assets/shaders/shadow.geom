#version 460

#define MAX_LIGHTS 4

layout (triangles, invocations = MAX_LIGHTS) in;
layout (triangle_strip, max_vertices = 3) out;

void main() {
}