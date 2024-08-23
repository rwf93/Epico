#version 460
#extension GL_ARB_shading_language_include: require

#define MAX_LIGHTS 4

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec4 out_composition;

#include "../../assets/shaders/renderdefs.h"
UNIFORM_BLOCK(0, CompositionData) composition = CompositionData_block.data;

layout(std140, set = 0, binding = 1) readonly buffer LightDataUniform {
    LightData lights[];
} lightdata;

layout(binding = 2) uniform sampler2D position_attachment;
layout(binding = 3) uniform sampler2D normal_attachment;
layout(binding = 4) uniform sampler2D albedo_attachment;

UNIFORM_BLOCK(5, SceneData) scene = SceneData_block.data;

void main() {

    vec3 position = texture(position_attachment, in_uv).rgb;
    vec3 normal = texture(normal_attachment, in_uv).rgb;
    vec4 albedo = texture(albedo_attachment, in_uv);

    vec3 lighting = albedo.rgb * 0.0;
    vec3 view_direction = normalize(composition.camera_position.xyz - position);

    for(int i = 0; i < MAX_LIGHTS; i++) {
        LightData light = lightdata.lights[i];

        vec3 L = light.position.xyz - position;
        float dist = length(L);

        vec3 V = composition.camera_position.xyz - position;
        V = normalize(V);
        L = normalize(L);

        float atten = light.radius / (pow(dist, 2.0) + 1.0);
        vec3 N = normalize(normal);
        float NdotL = max(0.0, dot(N, L));

        vec3 diff = light.color * albedo.rgb * NdotL * atten;

        vec3 R = reflect(-L, N);
        float NdotR = max(0.0, dot(R, V));
        vec3 spec = light.color * albedo.a * pow(NdotR, 16.0) * atten;

        lighting += diff + spec;
    }

    switch(composition.gbuffer_selection) {
        case 0:
            out_composition.rgb = position;
            break;
        case 1:
            out_composition.rgb = normal;
            break;
        case 2:
            out_composition.rgb = albedo.rgb;
            break;
        case 3:
            out_composition.rgb = albedo.aaa;
            break;
        case 4:
            out_composition.rgb = lighting;
        default: break;
    }
}