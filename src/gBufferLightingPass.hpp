#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <stdexcept>
#include <functional>
#include "gBufferPrepass.hpp"
#include <cgra/cgra_shader.hpp>

class gBufferLightingPass {
public:
	gBufferLightingPass(gBufferPrepass* prepassObj) {
		prepass = prepassObj;

		// setup shaders
		cgra::shader_builder sb;
		sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//fullscreen_quad_vert.glsl"));
		sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//lighting_pass_frag.glsl"));
		shader = sb.build();

		setupQuad();
	}

	~gBufferLightingPass() {
		glUseProgram(0);
		if (shader != 0 && glIsProgram(shader)) {
			glDeleteProgram(shader);
			shader = 0;
		}
		if (quadVBO != 0) {
			glDeleteBuffers(1, &quadVBO);
			quadVBO = 0;
		}
		if (quadVAO != 0) {
			glDeleteVertexArrays(1, &quadVAO);
			quadVAO = 0;
		}
	}

	void runPass() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);  // Render to screen
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shader);

		// Bind all G-buffer attachments
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, prepass->getAttachment(0)); // Position
		glUniform1i(glGetUniformLocation(shader, "gBufferPosition"), 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, prepass->getAttachment(1)); // Normal
		glUniform1i(glGetUniformLocation(shader, "gBufferNormal"), 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, prepass->getAttachment(2)); // Albedo
		glUniform1i(glGetUniformLocation(shader, "gBufferAlbedo"), 2);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, prepass->getAttachment(3)); // Emissive
		glUniform1i(glGetUniformLocation(shader, "gBufferEmissive"), 3);

		// Draw fullscreen quad
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
private:
	gBufferPrepass* prepass;
	GLuint shader; 
	GLuint quadVAO;
	GLuint quadVBO;
	void setupQuad() {
		float quadVertices[] = {
			// positions   // texCoords
			-1.0f,  1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,

			-1.0f,  1.0f,  0.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
			 1.0f,  1.0f,  1.0f, 1.0f
		};

		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);

		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// Texture coordinate attribute  
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
};
