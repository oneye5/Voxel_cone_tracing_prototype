#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <stdexcept>
#include <functional>

class gBufferPrepass {
public:
    gBufferPrepass(int targetWidth, int targetHeight)
        : width(targetWidth), height(targetHeight) {
        setupGBuffer();
    }

    ~gBufferPrepass() {
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(gAttachments.size(), gAttachments.data());
        glDeleteRenderbuffers(1, &depthRBO);
    }

    void executePrepass(GLuint shader,
        std::function<void()> drawScene) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, width, height);

        // Clear g buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader);
        glUniform1i(glGetUniformLocation(shader, "uRenderMode"), 1); // set to draw to gbuffer
        drawScene(); // user-supplied function that draws geometry

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    GLuint getAttachment(unsigned int index) const {
        if (index >= gAttachments.size()) throw std::out_of_range("G-buffer attachment index");
        return gAttachments[index];
    }

    GLuint getFBO() const { return fbo; }

private:
    GLuint fbo = 0;
    GLuint depthRBO = 0;
    std::vector<GLuint> gAttachments;
    int width, height;

    void setupGBuffer() {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        gAttachments.resize(4);

        glGenTextures(gAttachments.size(), gAttachments.data());

        // G-Buffer 0: world position.xyz + metallic
        glBindTexture(GL_TEXTURE_2D, gAttachments[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0,
                     GL_RGBA, GL_FLOAT, nullptr);
        setTextureParams();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, gAttachments[0], 0);

        // G-Buffer 1: world normal.xyz + smoothness
        glBindTexture(GL_TEXTURE_2D, gAttachments[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0,
                     GL_RGBA, GL_FLOAT, nullptr);
        setTextureParams();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
                               GL_TEXTURE_2D, gAttachments[1], 0);

        // G-Buffer 2: albedo.rgb + emissiveFactor
        glBindTexture(GL_TEXTURE_2D, gAttachments[2]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        setTextureParams();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2,
                               GL_TEXTURE_2D, gAttachments[2], 0);

        // G-Buffer 3: emissive.rgb + spare channel
        glBindTexture(GL_TEXTURE_2D, gAttachments[3]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        setTextureParams();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3,
                               GL_TEXTURE_2D, gAttachments[3], 0);

        // Specify multiple draw buffers for MRT
        GLenum drawBuffers[4] = {
            GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
        glDrawBuffers(4, drawBuffers);

        // Depth buffer
        glGenRenderbuffers(1, &depthRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                  GL_RENDERBUFFER, depthRBO);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("G-Buffer framebuffer is not complete!");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void setTextureParams() {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
};
