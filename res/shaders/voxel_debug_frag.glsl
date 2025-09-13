#version 430

out vec4 fragColor;
uniform sampler3D uVoxelTex;
uniform float uSlice; // 0.0 to 1.0

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(uVoxelTex, 0).xy);
    vec3 texCoord = vec3(uv, uSlice);
    fragColor = texture(uVoxelTex, texCoord);
}
