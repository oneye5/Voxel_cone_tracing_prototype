#version 440

layout(binding = 0, rgba8) uniform image3D voxelTex0; // Pos.xyz + Metallic
layout(binding = 1, rgba8) uniform image3D voxelTex1; // Normal.xyz + Smoothness
layout(binding = 2, rgba8) uniform image3D voxelTex2; // Albedo.rgb + EmissiveFactor

uniform int   uVoxelRes;
uniform float uVoxelWorldSize;
uniform int   uRenderMode; // 0 = voxelize, 1 = gbuffer

layout(location = 0) out vec4 gPosition; // world position.xyz + metallic
layout(location = 1) out vec4 gNormal;   // world normal.xyz + smoothness
layout(location = 2) out vec4 gAlbedo;   // albedo.rgb + emissiveFactor
layout(location = 3) out vec4 gEmissive; // emissive.rgb + spare channel

in vec3 worldPos;
in vec3 normal;
out vec4 fragColor;

struct MaterialData {
    vec3 pos, nrm, alb, emi; // world position, world normal, albedo, emissive color
    float mtl, smoothness, emiFac; // metalic, smoothness, emissive factor (strength of emission)
};

void writeRenderInfo(MaterialData m) {
    if (uRenderMode == 0) {
        vec3 vpos = (m.pos + uVoxelWorldSize * 0.5) / uVoxelWorldSize;
        if (any(lessThan(vpos, vec3(0))) || any(greaterThan(vpos, vec3(1)))) return;
        ivec3 texCoord = ivec3(vpos * float(uVoxelRes - 1));

        imageStore(voxelTex0, texCoord, vec4(vpos, m.mtl));
        imageStore(voxelTex1, texCoord, vec4(normalize(m.nrm) * 0.5 + 0.5, m.smoothness));
        imageStore(voxelTex2, texCoord, vec4(m.alb, m.emiFac));
    }
    else {
        gPosition = vec4(m.pos, m.mtl);
        gNormal = vec4(normalize(m.nrm), m.smoothness);
        gAlbedo = vec4(m.alb, m.emiFac);
        gEmissive = vec4(m.emi, 0);
    }
}

void main() {
    MaterialData m;
    m.pos = worldPos;
    m.nrm = normal;
    m.alb = vec3(1);
    m.emi = vec3(0);
    m.mtl = 0;
    m.smoothness = 1;
    m.emiFac = 0;

    writeRenderInfo(m);
    fragColor = vec4(1); // Optional debug color
}
