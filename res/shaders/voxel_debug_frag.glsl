#version 430

out vec4 FragColor;
uniform sampler3D voxelTex0; // Pos.xyz + Metallic
uniform sampler3D voxelTex1; // Normal.xyz + Smoothness
uniform sampler3D voxelTex2; // Albedo.rgb + EmissiveFactor

uniform float uSlice; // 0.0 to 1.0
uniform int uDebugIndex; 

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(voxelTex0, 0).xy);
    vec3 texCoord = vec3(uv, uSlice);

    if(uDebugIndex == 1) { // pos
	vec4 result = texture(voxelTex0, texCoord);
	FragColor = vec4(result.xyz, 1.0);
    } else if(uDebugIndex == 2) { // metallic
	vec4 result = texture(voxelTex0, texCoord);
	FragColor = vec4(result.w, result.w, result.w, 1.0);
    } else if(uDebugIndex == 3) { // normal
	vec4 result = texture(voxelTex1, texCoord);
	FragColor = vec4(result.xyz , 1.0);	
    } else if(uDebugIndex == 4) { // smoothness
	vec4 result = texture(voxelTex1, texCoord);
	FragColor = vec4(result.w, result.w, result.w, 1.0);	
    } else if(uDebugIndex == 5) { // albedo
	vec4 result = texture(voxelTex2, texCoord);
	FragColor = vec4(result.xyz , 1.0);	
    } else if(uDebugIndex == 6) { // emissive factor
	vec4 result = texture(voxelTex2, texCoord);
	FragColor = vec4(result.w, result.w, result.w, 1.0);	
    } 
    if(uDebugIndex !=0) {
	return;
    }


    if(texture(voxelTex2, texCoord).xyz != vec3(0.0)) {   
    	FragColor = vec4(1.0);
    }
}
