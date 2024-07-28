#version 450

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_texcoord;

layout(location = 3) in vec3 in_view;
layout(location = 4) in vec3 in_light;

layout(location = 0) out vec4 out_color;

void main() {
	vec3 color = vec3(mix(in_color, vec3(dot(vec3(0.2126,0.7152,0.0722), in_color)), 0.65));

	vec3 ambient = color * vec3(1.0);
	vec3 N = normalize(in_normal);
	vec3 L = normalize(in_light);
	vec3 V = normalize(in_view);
	vec3 R = reflect(-L, N);
	vec3 diffuse = max(dot(N, L), 0.0) * color;
	vec3 specular = pow(max(dot(R, V), 0.0), 16.0) * vec3(0.75);
	out_color = vec4(ambient + diffuse * 1.75 + specular, 1.0);

	float intensity = dot(N,L);
	float shade = 1.0;
	shade = intensity < 0.5 ? 0.75 : shade;
	shade = intensity < 0.35 ? 0.6 : shade;
	shade = intensity < 0.25 ? 0.5 : shade;
	shade = intensity < 0.1 ? 0.25 : shade;

	out_color.rgb = in_color * 3.0 * shade;
}