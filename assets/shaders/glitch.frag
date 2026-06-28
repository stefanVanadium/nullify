// Hack mode glitch overlay — row displacement + RGB split + violet tint
// intensity 0=none, 1=full; time drives animation
#version 130

uniform sampler2D texture;
uniform float     intensity;
uniform float     time;

float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    vec2 uv = gl_TexCoord[0].xy;

    // Horizontal row shift on random scan lines
    float scanNoise = rand(vec2(floor(uv.y * 80.0), floor(time * 8.0)));
    float rowShift  = (scanNoise > 0.92)
        ? (rand(vec2(uv.y, time)) - 0.5) * 0.06 * intensity
        : 0.0;

    vec2 d = uv + vec2(rowShift, 0.0);

    // RGB split
    float ca = intensity * 0.006;
    vec4 col;
    col.r = texture2D(texture, d + vec2(ca, 0.0)).r;
    col.g = texture2D(texture, d).g;
    col.b = texture2D(texture, d - vec2(ca, 0.0)).b;
    col.a = texture2D(texture, d).a;

    // Violet (#AA00FF) tint — hack identity colour
    col.rgb = mix(col.rgb, vec3(0.667, 0.0, 1.0), intensity * 0.22);

    // Subtle brightness flicker
    float flicker = rand(vec2(time * 20.0, 0.0)) * 0.08 * intensity;
    col.rgb *= (1.0 + flicker);

    gl_FragColor = col;
}
