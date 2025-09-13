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
    void voxelize(std::function<void()>, const glm::mat4& modelTransform);
    void renderDebugSlice(float sliceValue);
    void clearVoxelTexture();

    // Configuration
    void setResolution(int resolution);
    void setWorldSize(float worldSize);
    void setCenter(const glm::vec3& center);

    // Getters
    GLuint getVoxelTexture() const { return m_voxelTexture; }
    const VoxelParams& getParams() const { return m_params; }

private:
    // Initialization
    void initializeTexture();
    void initializeShaders();
    void initializeQuad();

    // Voxelization steps
    void setupVoxelizationState();
    void restoreRenderingState(int width, int height);
    void performVoxelization(std::function<void()> drawMainGeometry, const glm::mat4& modelTransform);

    // Helper methods
    glm::mat4 createOrthographicProjection() const;
    std::array<glm::mat4, 3> createOrthographicViews() const;

private:
    VoxelParams m_params;

    // OpenGL resources
    GLuint m_voxelTexture;
    GLuint m_voxelShader;
    GLuint m_debugShader;
    GLuint m_quadVAO;
    GLuint m_quadVBO;

    // State tracking
    bool m_initialized;
    int m_currentViewportWidth;
    int m_currentViewportHeight;
};