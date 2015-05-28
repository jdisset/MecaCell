#ifndef SKYBOX_HPP
#define SKYBOX_HPP
#include "viewtools.h"
#include "primitives/sphere.hpp"

#define SKY_SIZE 20000000.0

class Skybox {
	QOpenGLShaderProgram shader;
	unique_ptr<QOpenGLTexture> texture = nullptr;
	IcoSphere sky;

 public:
	void load() {
		shader.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/skybox.vert");
		shader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/skybox.frag");
		shader.link();
		texture = unique_ptr<QOpenGLTexture>(new QOpenGLTexture(QImage(":/textures/background.jpg").mirrored()));
		texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
		texture->setMagnificationFilter(QOpenGLTexture::Linear);
		sky.load(shader);
	}

	void draw(const QMatrix4x4& view, const QMatrix4x4& projection) {
		QMatrix4x4 model;
		model.scale(QVector3D(SKY_SIZE, SKY_SIZE, SKY_SIZE));
		shader.bind();
		sky.vao.bind();
		texture->bind(0);
		GL->glActiveTexture(GL_TEXTURE0);
		GL->glBindTexture(GL_TEXTURE_2D, texture->textureId());
		shader.setUniformValue(shader.uniformLocation("tex"), 0);
		shader.setUniformValue(shader.uniformLocation("projection"), projection);
		shader.setUniformValue(shader.uniformLocation("view"), view);
		shader.setUniformValue(shader.uniformLocation("model"), model);
		GL->glDrawElements(GL_TRIANGLES, sky.indices.size(), GL_UNSIGNED_INT, 0);
		sky.vao.release();
		shader.release();
	}
};
#endif
