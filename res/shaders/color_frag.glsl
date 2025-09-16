#version 440
layout(binding = 0, rgba8) uniform image3D voxelTexture;

uniform int uVoxelRes;
uniform float uVoxelWorldSize;
uniform int uRenderMode; // 0 = write voxels, 1 = write to gbuffer

// g-buffer outputs (multiple render targets)
layout (location = 0) out vec4 gPosition;    // world position.xyz + metallic
layout (location = 1) out vec4 gNormal;      // world normal.xyz + smoothness
layout (location = 2) out vec4 gAlbedo;      // albedo.rgb + emissiveFactor
layout (location = 3) out vec4 gEmissive;    // emissive.rgb + spare channel

in vec3 worldPos;
out vec4 fragColor;

void voxelizeFragment(vec3 worldPos, vec4 color) {
    vec3 voxelPos = (worldPos + uVoxelWorldSize * 0.5) / uVoxelWorldSize;
    if (any(lessThan(voxelPos, vec3(0.0))) || any(greaterThan(voxelPos, vec3(1.0)))) return;
    ivec3 texCoord = ivec3(voxelPos * float(uVoxelRes - 1));
    imageStore(voxelTexture, texCoord, color);
}
void writeToGBuffer(vec3 worldPosition, 
                   vec3 worldNormal, 
                   vec3 albedo, 
                   vec3 emissive, 
                   float metallic, 
                   float smoothness, 
                   float emissiveFactor,
                   float spareChannel) {
    // G-Buffer 0: world position.xyz + metallic
    gPosition = vec4(worldPosition, metallic);
    // G-Buffer 1: world normal.xyz + smoothness
    gNormal = vec4(worldNormal, smoothness);
    // G-Buffer 2: albedo.rgb + emissiveFactor
    gAlbedo = vec4(albedo, emissiveFactor);
    // G-Buffer 3: emissive.rgb + spare channel
    gEmissive = vec4(emissive, spareChannel);
}
void writeRenderInfo(vec3 worldPosition, 
                   vec3 worldNormal, 
                   vec4 albedo, 
                   vec3 emissive, 
                   float metallic, 
                   float smoothness, 
                   float emissiveFactor,
                   float spareChannel) {
	if(uRenderMode == 0) {
		voxelizeFragment(worldPosition, albedo);
	} else {
		writeToGBuffer(worldPosition, worldNormal, albedo.xyz, emissive, metallic, smoothness, emissiveFactor, spareChannel);
	}
}

void main() {
	writeRenderInfo(worldPos, vec3(1.0), vec4(1.0), vec3(1.0), 1, 1, 1, 1);
	fragColor = vec4(1);
}
