#ifndef BLURQUAD_HPP
#define BLURQUAD_HPP
#include <QOpenGLFramebufferObject>
#include "../primitives/quad.hpp"
#include "../primitives/renderquad.hpp"
#include "../utilities/viewtools.h"

namespace MecacellViewer {
class BlurQuad {
	unique_ptr<QOpenGLFramebufferObject> fboA, fboB;
	QOpenGLFramebufferObjectFormat format;
	QOpenGLShaderProgram shader;
	Quad quad;
	RenderQuad render;

 public:
	BlurQuad(){};

	void load(const QString &vs, const QString &fs, const QSize &s) {
		fboA = unique_ptr<QOpenGLFramebufferObject>(new QOpenGLFramebufferObject(s, format));
		fboB = unique_ptr<QOpenGLFramebufferObject>(new QOpenGLFramebufferObject(s, format));
		render.load(":/shaders/dumb.vert", ":/shaders/dumb.frag");
		shader.addShaderFromSourceCode(QOpenGLShader::Vertex, shaderWithHeader(vs));
		shader.addShaderFromSourceCode(QOpenGLShader::Fragment, shaderWithHeader(fs));
		shader.link();
		quad.load(shader);
	}

	void draw(GLuint tex, int amount, const QSize &s, const QRect &r) {
		if (fboA->size() != s || fboB->size() != s) {
			fboA =
			    unique_ptr<QOpenGLFramebufferObject>(new QOpenGLFramebufferObject(s, format));
			fboB =
			    unique_ptr<QOpenGLFramebufferObject>(new QOpenGLFramebufferObject(s, format));
		}

		fboA->bind();
		render.draw(tex);
		fboA->release();

		shader.bind();
		quad.vao.bind();

		fboB->bind();
		GL()->glActiveTexture(GL_TEXTURE0);
		shader.setUniformValue(shader.uniformLocation("tex"), 0);
		shader.setUniformValue(shader.uniformLocation("xRatio"),
		                       (float)r.width() / (float)s.width());
		for (int i = 0; i < amount; ++i) {
			if (i == 0) GL()->glBindTexture(GL_TEXTURE_2D, tex);
			shader.setUniformValue(shader.uniformLocation("dir"), QVector2D(1.0, 0));
			GL()->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			shader.setUniformValue(shader.uniformLocation("dir"),
			                       QVector2D(0.70710678118, 0.70710678118));
			GL()->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			shader.setUniformValue(shader.uniformLocation("dir"),
			                       QVector2D(-0.70710678118, 0.70710678118));
			GL()->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			shader.setUniformValue(shader.uniformLocation("dir"), QVector2D(0, 1.0));
			GL()->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			if (i == 0) GL()->glBindTexture(GL_TEXTURE_2D, fboB->texture());
		}
		fboB->release();
		quad.vao.release();
		shader.release();
		QOpenGLFramebufferObject::blitFramebuffer(fboA.get(), r, fboB.get(), r,
		                                          GL_COLOR_BUFFER_BIT, GL_LINEAR);
		render.draw(fboA->texture());
		render.draw(tex);
	}
};
}
#endif
