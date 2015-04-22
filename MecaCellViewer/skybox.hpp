#ifndef SKYBOX_HPP
#define SKYBOX_HPP
#include "viewtools.h"
#include "primitives/quad.hpp"

class Skybox {
	QOpenGLShaderProgram shader;
	unique_ptr<QOpenGLTexture> texture = nullptr;
	Quad quad;

 public:
	void load() {
		shader.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/dumb.vert");
		shader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/skybox.frag");
		shader.link();
		texture = unique_ptr<QOpenGLTexture>(new QOpenGLTexture(QImage(":/textures/background.jpg").mirrored()));
		texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
		texture->setMagnificationFilter(QOpenGLTexture::Linear);
		quad.load(shader);
	}
	void draw(const QVector3D& viewV) {
		shader.bind();
		quad.bindVAO();
		GL->glActiveTexture(GL_TEXTURE0);
		GL->glBindTexture(GL_TEXTURE_2D, texture->textureId());
		shader.setUniformValue(shader.uniformLocation("tex"), 0);
		shader.setUniformValue(shader.uniformLocation("viewvector"), viewV);
		GL->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		quad.releaseVAO();
		shader.release();
	}
};
#endif
