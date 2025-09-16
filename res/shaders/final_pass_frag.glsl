#version 440

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D gBufferAlbedo;     // Your gAttachments[2]

void main() {
    vec4 albedo = texture(gBufferAlbedo, TexCoord);
    FragColor = vec4(albedo.rgb, 1.0);
}