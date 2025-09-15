#version 440

layout(location = 0) in vec3 aPosition;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
uniform int uVoxelRes;
uniform float uVoxelWorldSize;

out vec3 worldPos;

void main() {
    worldPos = (uModelMatrix * vec4(aPosition, 1.0)).xyz;
    gl_Position = uProjectionMatrix * uViewMatrix * worldPosition;
}