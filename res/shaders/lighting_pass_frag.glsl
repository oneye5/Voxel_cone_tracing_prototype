#version 440

in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D gBufferPosition;
uniform sampler2D gBufferNormal;
uniform sampler2D gBufferAlbedo;    
uniform sampler2D gBufferEmissive;
uniform int uDebugIndex;


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

    // if debug ===============
    if(uDebugIndex == 1) { // pos
        FragColor = vec4(worldPos, 1.0);
    } else if (uDebugIndex == 2) { // metalic
        FragColor = vec4(metallic, metallic, metallic, 1.0);	
    } else if (uDebugIndex == 3) { // norm
        FragColor = vec4(worldNormal, 1.0);
    } else if (uDebugIndex == 4) { // smooth
        FragColor = vec4(smoothness, smoothness, smoothness, 1.0);
    } else if (uDebugIndex == 5) { // albedo
        FragColor = vec4(albedo, 1.0);
    } else if (uDebugIndex == 6) { // emissive
        FragColor = vec4(emissiveFactor,emissiveFactor,emissiveFactor, 1.0);
    }  else if (uDebugIndex == 7) { // emissive rgb
        FragColor = vec4(emissiveRgb, 1.0);
    }  else if (uDebugIndex == 8) { // spare
        FragColor = vec4(spare, spare, spare, 1.0);
    }
    if(uDebugIndex !=0) {
	return;
    }

    FragColor = vec4(worldNormal, 1.0);
}