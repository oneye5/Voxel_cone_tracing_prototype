#include "voxelizer.hpp"
#include "cgra/cgra_shader.hpp"
#include <iostream>
#include <array>

using namespace glm;
using namespace cgra;

Voxelizer::Voxelizer(int resolution)
    : m_params{ resolution, 30.0f, vec3(0.0f) }
    , m_voxelTex0(0)
    , m_voxelTex1(0)
    , m_voxelTex2(0)
    , m_voxelShader(0)
    , m_debugShader(0)
    , m_quadVAO(0)
    , m_quadVBO(0)
    , m_initialized(false)
    , m_currentViewportWidth(0)
    , m_currentViewportHeight(0)
{
    initializeTextures();
    initializeShaders();
    initializeQuad();
    m_initialized = true;
}

Voxelizer::~Voxelizer() {
    glUseProgram(0);

    if (m_quadVBO != 0) {
        glDeleteBuffers(1, &m_quadVBO);
        m_quadVBO = 0;
    }
    if (m_quadVAO != 0) {
        glDeleteVertexArrays(1, &m_quadVAO);
        m_quadVAO = 0;
    }
    if (m_voxelTex0 != 0) {
        glDeleteTextures(1, &m_voxelTex0);
        m_voxelTex0 = 0;
    }
    if (m_voxelTex1 != 0) {
        glDeleteTextures(1, &m_voxelTex1);
        m_voxelTex1 = 0;
    }
    if (m_voxelTex2 != 0) {
        glDeleteTextures(1, &m_voxelTex2);
        m_voxelTex2 = 0;
    }
    if (m_voxelShader != 0 && glIsProgram(m_voxelShader)) {
        glDeleteProgram(m_voxelShader);
        m_voxelShader = 0;
    }
    if (m_debugShader != 0 && glIsProgram(m_debugShader)) {
        glDeleteProgram(m_debugShader);
        m_debugShader = 0;
    }
    m_initialized = false;
}

void Voxelizer::initializeTextures() {
    auto make3DTex = [&](GLuint& tex) {
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_3D, tex);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8,
            m_params.resolution, m_params.resolution, m_params.resolution,
            0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_3D, 0);
        };

    make3DTex(m_voxelTex0);
    make3DTex(m_voxelTex1);
    make3DTex(m_voxelTex2);
}


void Voxelizer::initializeShaders() {
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

void Voxelizer::voxelize(std::function<void()> drawMainGeometry, const mat4& modelTransform, const GLuint usingShader) {
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
    performVoxelization(drawMainGeometry, modelTransform, usingShader);
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
    glClearTexImage(m_voxelTex0, 0, GL_RGBA, GL_FLOAT, clearColor);
    glClearTexImage(m_voxelTex1, 0, GL_RGBA, GL_FLOAT, clearColor);
    glClearTexImage(m_voxelTex2, 0, GL_RGBA, GL_FLOAT, clearColor);
}

void Voxelizer::setupVoxelizationState() {
    // glUseProgram(m_voxelShader); not needed
    glViewport(0, 0, m_params.resolution, m_params.resolution);

    // Bind voxel texture for writing
    glBindImageTexture(0, m_voxelTex0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glBindImageTexture(1, m_voxelTex1, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glBindImageTexture(2, m_voxelTex2, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);

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

void Voxelizer::performVoxelization(std::function<void()> drawMainGeometry, const glm::mat4& modelTransform, const GLuint usingShader) {
    // Set common uniforms
    mat4 orthoProj = createOrthographicProjection();
    auto views = createOrthographicViews();

    glUniformMatrix4fv(glGetUniformLocation(usingShader, "uModelMatrix"), 1, GL_FALSE, value_ptr(modelTransform));
    glUniformMatrix4fv(glGetUniformLocation(usingShader, "uProjectionMatrix"), 1, GL_FALSE, value_ptr(orthoProj));
    glUniform1i(glGetUniformLocation(usingShader, "uVoxelRes"), m_params.resolution);
    glUniform1f(glGetUniformLocation(usingShader, "uVoxelWorldSize"), m_params.worldSize);
    glUniform1i(glGetUniformLocation(usingShader, "uRenderMode"), 0); // voxelize mode

    // Render from three orthogonal directions
    const char* axisNames[] = { "X", "Y", "Z" };
    for (int i = 0; i < 3; ++i) {
        glm::mat4 modelView = views[i] * modelTransform;
        glUniformMatrix4fv(glGetUniformLocation(usingShader, "uModelViewMatrix"), 1, GL_FALSE, value_ptr(views[i]));
        glUniformMatrix4fv(glGetUniformLocation(usingShader, "uViewMatrix"), 1, GL_FALSE, value_ptr(modelView));
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

void Voxelizer::renderDebugSlice(float sliceValue, int debugMode) {
    if (!m_initialized) {
        std::cerr << "Voxelizer not properly initialized!" << std::endl;
        return;
    }

    glUseProgram(m_debugShader);
    glUniform1i(glGetUniformLocation(m_debugShader, "uDebugIndex"), debugMode);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, m_voxelTex0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, m_voxelTex1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, m_voxelTex2);

    glUniform1i(glGetUniformLocation(m_debugShader, "uVoxelTex"), 0);
    glUniform1f(glGetUniformLocation(m_debugShader, "uSlice"), sliceValue);

    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    restoreRenderingState(m_currentViewportWidth, m_currentViewportHeight);
}

void Voxelizer::setResolution(int resolution) {
    if (resolution != m_params.resolution) {
        m_params.resolution = resolution;
        // Recreate texture with new resolution
        glDeleteTextures(1, &m_voxelTex0);
        glDeleteTextures(1, &m_voxelTex1);
        glDeleteTextures(1, &m_voxelTex2);
        initializeTextures();
    }
}

void Voxelizer::setWorldSize(float worldSize) {
    m_params.worldSize = worldSize;
}

void Voxelizer::setCenter(const vec3& center) {
    m_params.center = center;
}