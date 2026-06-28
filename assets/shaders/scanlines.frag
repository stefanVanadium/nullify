// Subtle CRT scanlines overlay — drawn as a full-screen additive/alpha quad
#version 130

void main() {
    // Every 3rd row is slightly darkened
    float line = mod(gl_FragCoord.y, 3.0);
    float alpha = (line < 1.0) ? 0.07 : 0.0;
    gl_FragColor = vec4(0.0, 0.0, 0.0, alpha);
}
