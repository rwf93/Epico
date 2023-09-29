#version 460

#include "primitives.h"

layout(binding = 0) uniform EGlobalData {
	mat4 view;
	mat4 projection;
} global;

layout(std140, set = 1, binding = 0) readonly buffer EObjectBuffer {
	EObjectData objects[];
} object_buffer;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec3 in_normal;
layout(location = 3) in vec2 in_texcoord;

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec3 out_texcoord;

layout(location = 3) out vec3 out_view;
layout(location = 4) out vec3 out_light;

void main() {
	EObjectData object = object_buffer.objects[gl_BaseInstance];

	vec4 pos = object.model * vec4(in_position, 1.0);
	vec3 lpos = mat3(object.model) * vec3(0.0, 2.0, 1.0); // position

	out_color = in_color;
	out_normal = mat3(object.model) * in_normal;
	out_texcoord = vec3(in_texcoord, object.texture_index);

	out_light = lpos - pos.xyz;
	out_view = -pos.xyz;

	gl_Position = global.projection * global.view * object.model * vec4(in_position, 1.0);
}