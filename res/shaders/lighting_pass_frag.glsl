#version 440

in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D gBufferPosition;
uniform sampler2D gBufferNormal;
uniform sampler2D gBufferAlbedo;    
uniform sampler2D gBufferEmissive;


void main() {
    // unpack channels
    vec3 worldPos = texture(gBufferPosition, texCoord).xyz;
    float metallic = texture(gBufferPosition, texCoord).w;

    vec3 worldNormal = texture(gBufferNormal, texCoord).xyz;
    float smoothness = texture(gBufferNormal, texCoord).w;

    vec3 albedo = texture(gBufferAlbedo, texCoord).xyz;
    float emissiveFactor = texture(gBufferAlbedo, texCoord).w;

    vec3 emissiveRgb = texture(gBufferEmissive, texCoord).xyz;
    float spare = texture(gBufferEmissive, texCoord).w;

    FragColor = vec4(worldNormal, 1.0);
}