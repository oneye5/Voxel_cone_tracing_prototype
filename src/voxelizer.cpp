#include "voxelizer.hpp"
#include "cgra/cgra_shader.hpp"
#include <iostream>
#include <array>

using namespace glm;
using namespace cgra;

Voxelizer::Voxelizer(int resolution)
    : m_params{ resolution, 30.0f, vec3(0.0f) }
    , m_voxelTexture(0)
    , m_voxelShader(0)
    , m_debugShader(0)
    , m_quadVAO(0)
    , m_quadVBO(0)
    , m_initialized(false)
    , m_currentViewportWidth(0)
    , m_currentViewportHeight(0)
{
    initializeTexture();
    initializeShaders();
    initializeQuad();
    m_initialized = true;
}

Voxelizer::~Voxelizer() {
    if (m_voxelTexture) glDeleteTextures(1, &m_voxelTexture);
    if (m_voxelShader) glDeleteProgram(m_voxelShader);
    if (m_debugShader) glDeleteProgram(m_debugShader);
    if (m_quadVAO) glDeleteVertexArrays(1, &m_quadVAO);
    if (m_quadVBO) glDeleteBuffers(1, &m_quadVBO);
}

void Voxelizer::initializeTexture() {
    glGenTextures(1, &m_voxelTexture);
    glBindTexture(GL_TEXTURE_3D, m_voxelTexture);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Allocate texture storage
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8,
        m_params.resolution, m_params.resolution, m_params.resolution,
        0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Bind as image texture for compute operations
    glBindImageTexture(0, m_voxelTexture, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);

    glBindTexture(GL_TEXTURE_3D, 0);
}

void Voxelizer::initializeShaders() {
    // Voxelization shader
    shader_builder voxelBuilder;
    voxelBuilder.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//voxelize_vert.glsl"));
    voxelBuilder.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//voxelize_frag.glsl"));
    m_voxelShader = voxelBuilder.build();

    // Debug visualization shader
    shader_builder debugBuilder;
    debugBuilder.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//fullscreen_quad_vert.glsl"));
    debugBuilder.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//voxel_debug_frag.glsl"));
    m_debugShader = debugBuilder.build();
}

void Voxelizer::initializeQuad() {
    float quadVertices[] = {
        // positions   // uv
        -1.f, -1.f,    0.f, 0.f,
         1.f, -1.f,    1.f, 0.f,
        -1.f,  1.f,    0.f, 1.f,
         1.f,  1.f,    1.f, 1.f
    };

    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);

    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}

void Voxelizer::voxelize(std::function<void()> drawMainGeometry, const mat4& modelTransform) {
    if (!m_initialized) {
        std::cerr << "Voxelizer not properly initialized!" << std::endl;
        return;
    }

    // Store current viewport
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    m_currentViewportWidth = viewport[2];
    m_currentViewportHeight = viewport[3];

    setupVoxelizationState();
    performVoxelization(drawMainGeometry, modelTransform);
    restoreRenderingState(m_currentViewportWidth, m_currentViewportHeight);

    // Memory barrier to ensure writes are complete
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // Check for errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error after voxelization: " << error << std::endl;
    }
}

void Voxelizer::clearVoxelTexture() {
    GLfloat clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    glClearTexImage(m_voxelTexture, 0, GL_RGBA, GL_FLOAT, clearColor);
}

void Voxelizer::setupVoxelizationState() {
    glUseProgram(m_voxelShader);
    glViewport(0, 0, m_params.resolution, m_params.resolution);

    // Bind voxel texture for writing
    glBindImageTexture(0, m_voxelTexture, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);

    // Disable framebuffer rendering since we're writing directly to 3D texture
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
}

void Voxelizer::restoreRenderingState(int width, int height) {
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glViewport(0, 0, width, height);
}

void Voxelizer::performVoxelization(std::function<void()> drawMainGeometry, const glm::mat4& modelTransform) {
    // Set common uniforms
    mat4 orthoProj = createOrthographicProjection();
    auto views = createOrthographicViews();

    glUniformMatrix4fv(glGetUniformLocation(m_voxelShader, "uModelMatrix"), 1, GL_FALSE, value_ptr(modelTransform));
    glUniformMatrix4fv(glGetUniformLocation(m_voxelShader, "uProjectionMatrix"), 1, GL_FALSE, value_ptr(orthoProj));
    glUniform1i(glGetUniformLocation(m_voxelShader, "uVoxelRes"), m_params.resolution);
    glUniform1f(glGetUniformLocation(m_voxelShader, "uVoxelWorldSize"), m_params.worldSize);

    // Render from three orthogonal directions
    const char* axisNames[] = { "X", "Y", "Z" };
    for (int i = 0; i < 3; ++i) {
        glUniformMatrix4fv(glGetUniformLocation(m_voxelShader, "uViewMatrix"), 1, GL_FALSE, value_ptr(views[i]));
        drawMainGeometry();
    }
}

mat4 Voxelizer::createOrthographicProjection() const {
    float halfSize = m_params.worldSize / 2.0f;
    return ortho(-halfSize, halfSize, -halfSize, halfSize, -halfSize, halfSize);
}

std::array<mat4, 3> Voxelizer::createOrthographicViews() const {
    float halfSize = m_params.worldSize / 2.0f;
    vec3 center = m_params.center;

    return {
        lookAt(center + vec3(halfSize, 0, 0), center, vec3(0, 1, 0)),  // Looking along -X
        lookAt(center + vec3(0, halfSize, 0), center, vec3(0, 0, 1)),  // Looking along -Y  
        lookAt(center + vec3(0, 0, halfSize), center, vec3(0, 1, 0))   // Looking along -Z
    };
}

void Voxelizer::renderDebugSlice(float sliceValue) {
    if (!m_initialized) {
        std::cerr << "Voxelizer not properly initialized!" << std::endl;
        return;
    }

    glUseProgram(m_debugShader);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, m_voxelTexture);
    glUniform1i(glGetUniformLocation(m_debugShader, "uVoxelTex"), 0);
    glUniform1f(glGetUniformLocation(m_debugShader, "uSlice"), sliceValue);

    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Voxelizer::setResolution(int resolution) {
    if (resolution != m_params.resolution) {
        m_params.resolution = resolution;
        // Recreate texture with new resolution
        glDeleteTextures(1, &m_voxelTexture);
        initializeTexture();
    }
}

void Voxelizer::setWorldSize(float worldSize) {
    m_params.worldSize = worldSize;
}

void Voxelizer::setCenter(const vec3& center) {
    m_params.center = center;
}