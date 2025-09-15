#version 440
layout(binding = 0, rgba8) uniform image3D voxelTexture;

uniform int uVoxelRes;
uniform float uVoxelWorldSize;

in vec3 worldPos;
out vec4 fragColor;

void voxelizeFragment(vec3 worldPos, vec4 color) {
    vec3 voxelPos = (worldPos + uVoxelWorldSize * 0.5) / uVoxelWorldSize;
    if (any(lessThan(voxelPos, vec3(0.0))) || any(greaterThan(voxelPos, vec3(1.0)))) return;
    ivec3 texCoord = ivec3(voxelPos * float(uVoxelRes - 1));
    imageStore(voxelTexture, texCoord, color);
}

void main() {
	voxelizeFragment(worldPos, vec4(1.0));
	fragColor = vec4(1);
}

