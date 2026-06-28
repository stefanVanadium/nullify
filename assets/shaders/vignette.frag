// Screen-space vignette — darkens edges, intensifies at low HP
#version 130

uniform float intensity; // 0.6 default, up to 1.5 at low HP

void main() {
    // gl_TexCoord[0].xy is (0,0)-(1,1) on the overlay quad
    vec2  uv   = gl_TexCoord[0].xy;
    vec2  c    = vec2(0.5, 0.5);
    float dist = length(uv - c) * 1.6;
    float v    = dist * dist * intensity;
    v = clamp(v, 0.0, 1.0);
    gl_FragColor = vec4(0.0, 0.0, 0.0, v);
}
