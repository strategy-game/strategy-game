#version 330 compatibility
precision lowp float;

out vec4 f_color;
in vec2 v_texcoord;

uniform sampler2D atlas;

uniform float pxRange;

float screenPxRange() {
    vec2 uv = v_texcoord;
    uv.y = 1. - uv.y;
    vec2 unitRange = vec2(pxRange)/vec2(textureSize(atlas, 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(uv);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() {
    vec2 uv = v_texcoord;
    uv.y = 1. - uv.y;
    vec3 msd = texture(atlas, uv).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = screenPxRange()*(sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    f_color = vec4(0., 0., 0., opacity);
}
