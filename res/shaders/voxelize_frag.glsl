#version 440
layout(binding = 0, rgba8) uniform image3D voxelTexture;

uniform int uVoxelRes;
uniform float uVoxelWorldSize;

in vec3 worldPos;

void main() {
    // Convert world position to voxel coordinates [0, 1]
    vec3 voxelPos = (worldPos + uVoxelWorldSize * 0.5) / uVoxelWorldSize;
    
    // Debug: Check if we're getting fragments
    // This should write to console or debug buffer in real implementation
    
    // Check bounds - if outside, skip
    if (any(lessThan(voxelPos, vec3(0.0))) || any(greaterThan(voxelPos, vec3(1.0)))) {
        return; // Outside bounds
    }
    
    // Convert to texture coordinates
    ivec3 texCoord = ivec3(voxelPos * float(uVoxelRes - 1)); // Note: -1 to avoid out of bounds
    
    // Write solid red voxel for debugging
    vec4 voxelData = vec4(1.0, 0.0, 0.0, 1.0); // Solid red
    imageStore(voxelTexture, texCoord, voxelData);
}