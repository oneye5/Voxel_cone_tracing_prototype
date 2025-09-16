#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "cgra/cgra_mesh.hpp"
#include <functional>

class Voxelizer {
public:
    struct VoxelParams {
        int resolution = 512;
        float worldSize = 30.0f;
        glm::vec3 center = glm::vec3(0.0f);
    };

    Voxelizer(int resolution = 512);
    ~Voxelizer();
    // Main interface 
    void voxelize(std::function<void()> drawMainGeometry, const  glm::mat4& modelTransform, const GLuint usingShader);
    void renderDebugSlice(float sliceValue, int debugMode = 0);
    void clearVoxelTexture(); 

    // Configuration
    void setResolution(int resolution);
    void setWorldSize(float worldSize);
    void setCenter(const glm::vec3& center);
private:
    // Initialization
    void initializeTextures();
    void initializeShaders();
    void initializeQuad();

    // Voxelization steps
    void setupVoxelizationState();
    void restoreRenderingState(int width, int height);
    void performVoxelization(std::function<void()> drawMainGeometry, const glm::mat4& modelTransform, const GLuint usingShader);

    // Helper methods
    glm::mat4 createOrthographicProjection() const;
    std::array<glm::mat4, 3> createOrthographicViews() const;

private:
    VoxelParams m_params;

    // OpenGL resources
    GLuint m_voxelTex0; // normal + smoothness
    GLuint m_voxelTex1; // albedo + emissiveFactor
    GLuint m_voxelTex2; // emissive + metallic

    GLuint m_voxelShader;
    GLuint m_debugShader;
    GLuint m_quadVAO;
    GLuint m_quadVBO;

    // State tracking
    bool m_initialized;
    int m_currentViewportWidth;
    int m_currentViewportHeight;
};