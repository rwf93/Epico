#version 460

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec4 out_composition;

layout(binding = 0) uniform CompositionData {
    uint gbuffer_selection;
} composition;

layout(binding = 1) uniform sampler2D position_attachment;
layout(binding = 2) uniform sampler2D normal_attachment;
layout(binding = 3) uniform sampler2D albedo_attachment;

void main() {
    switch(composition.gbuffer_selection) {
        case 0:
            out_composition = texture(position_attachment, in_uv);
            break;
        case 1:
            out_composition = texture(normal_attachment, in_uv);
            break;
        case 2:
            out_composition = texture(albedo_attachment, in_uv);
            break;
        default: break;
    }
}