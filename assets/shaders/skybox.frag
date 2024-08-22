#version 460

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

layout(binding = 1) uniform sampler2D skybox_texture;

void mainImage(out vec4 O, vec2 I)
{
    vec2 R = vec2(1280, 762),
         U = 1.69 * ( I+I - R ) / R.y;
    U.x++;
    float v = .5, d = v, i;
    for( ; i++ < 1e2; )
        v *= U.x *  (1. - v),
        i > 50. ?  d = min(d, abs(U.y - v)) : d;

    O = vec4(.005 / d);
}


void main() {
    out_color = 1 - vec4(texture(skybox_texture, in_uv).rgb, 1.0);
}