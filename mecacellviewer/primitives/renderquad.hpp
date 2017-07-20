#ifndef RENDERQUAD_HPP
#define RENDERQUAD_HPP
#include "../utilities/viewtools.h"
#include "quad.hpp"

namespace MecacellViewer {
class RenderQuad {
	QOpenGLShaderProgram shader;
	Quad quad;

 public:
	RenderQuad(){};
	void load(const QString &vs, const QString &fs) {
		shader.addShaderFromSourceCode(QOpenGLShader::Vertex, shaderWithHeader(vs));
		shader.addShaderFromSourceCode(QOpenGLShader::Fragment, shaderWithHeader(fs));
		shader.link();
		quad.load(shader);
	}

	void draw(GLuint tex) {
		shader.bind();
		quad.vao.bind();
		GL()->glActiveTexture(GL_TEXTURE0);
		GL()->glBindTexture(GL_TEXTURE_2D, tex);
		shader.setUniformValue(shader.uniformLocation("tex"), 0);
		GL()->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		quad.vao.release();
		shader.release();
	}

	void draw(GLuint tex, GLuint depth, float near, float far) {
		shader.bind();
		quad.vao.bind();
		GL()->glActiveTexture(GL_TEXTURE0);
		GL()->glBindTexture(GL_TEXTURE_2D, tex);
		GL()->glActiveTexture(GL_TEXTURE1);
		GL()->glBindTexture(GL_TEXTURE_2D, depth);
		shader.setUniformValue(shader.uniformLocation("tex"), 0);
		shader.setUniformValue(shader.uniformLocation("depthBuf"), 1);
		shader.setUniformValue(shader.uniformLocation("nearClip"), near);
		shader.setUniformValue(shader.uniformLocation("farClip"), far);
		GL()->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		GL()->glActiveTexture(GL_TEXTURE0);
		quad.vao.release();
		shader.release();
	}
};
}
#endif
