// Bloom + optional chromatic aberration — applied to scene RenderTexture
// Inputs: scene texture, per-pixel texel size, CA intensity (0-1)
#version 130

uniform sampler2D texture;
uniform vec2      texelSize;    // vec2(1/width, 1/height)
uniform float     caIntensity;  // 0=none, 1=full RGB split

void main() {
    vec2 uv = gl_TexCoord[0].xy;

    // Chromatic aberration: shift red right, blue left
    vec4 col;
    if (caIntensity > 0.001) {
        float ca = caIntensity * 0.008;
        col.r = texture2D(texture, uv + vec2(ca, 0.0)).r;
        col.g = texture2D(texture, uv).g;
        col.b = texture2D(texture, uv - vec2(ca, 0.0)).b;
        col.a = 1.0;
    } else {
        col = texture2D(texture, uv);
    }

    // 12-tap bloom: sample neighbours, accumulate glow from bright pixels
    vec3 bloom = vec3(0.0);
    float r = 3.0;
    vec2 offsets[12];
    offsets[0]  = vec2(-r,      0.0);
    offsets[1]  = vec2( r,      0.0);
    offsets[2]  = vec2( 0.0,   -r);
    offsets[3]  = vec2( 0.0,    r);
    offsets[4]  = vec2(-r,     -r);
    offsets[5]  = vec2( r,     -r);
    offsets[6]  = vec2(-r,      r);
    offsets[7]  = vec2( r,      r);
    offsets[8]  = vec2(-r*2.0,  0.0);
    offsets[9]  = vec2( r*2.0,  0.0);
    offsets[10] = vec2( 0.0,   -r*2.0);
    offsets[11] = vec2( 0.0,    r*2.0);

    for (int i = 0; i < 12; i++) {
        vec3 s = texture2D(texture, uv + offsets[i] * texelSize).rgb;
        float brightness = dot(s, vec3(0.2126, 0.7152, 0.0722));
        bloom += s * max(0.0, brightness - 0.55) * 2.0;
    }

    gl_FragColor = vec4(col.rgb + bloom * 0.35, col.a);
}
