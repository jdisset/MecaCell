#ifndef SKYBOX_HPP
#define SKYBOX_HPP
#include "../camera.hpp"
#include "../primitives/sphere.hpp"
#include "../utilities/viewtools.h"

namespace MecacellViewer {
template <typename R> class Skybox : public PaintStep<R> {
	QOpenGLShaderProgram shader;
	unique_ptr<QOpenGLTexture> texture = nullptr;
	IcoSphere sky;

 public:
	Skybox() : PaintStep<R>("Skybox") {
		shader.addShaderFromSourceCode(QOpenGLShader::Vertex,
		                               shaderWithHeader(":/shaders/skybox.vert"));
		shader.addShaderFromSourceCode(QOpenGLShader::Fragment,
		                               shaderWithHeader(":/shaders/skybox.frag"));
		shader.link();
		texture = unique_ptr<QOpenGLTexture>(
		    new QOpenGLTexture(QImage(":/textures/background.jpg").mirrored()));
		texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
		texture->setMagnificationFilter(QOpenGLTexture::Linear);
		sky.load(shader);
	}

	void call(R *r) {
		const QMatrix4x4 &view = r->getViewMatrix();
		const QMatrix4x4 &projection = r->getProjectionMatrix();
		const Camera &camera = r->getCamera();
		QMatrix4x4 model;
		model.translate(camera.getPosition());
		float scale = camera.getFarPlane() - camera.getNearPlane() - 1;
		model.scale(QVector3D(scale, scale, scale));
		shader.bind();
		sky.vao.bind();
		texture->bind(0);
		GL()->glActiveTexture(GL_TEXTURE0);
		GL()->glBindTexture(GL_TEXTURE_2D, texture->textureId());
		shader.setUniformValue(shader.uniformLocation("tex"), 0);
		shader.setUniformValue(shader.uniformLocation("projection"), projection);
		shader.setUniformValue(shader.uniformLocation("view"), view);
		shader.setUniformValue(shader.uniformLocation("model"), model);
		GL()->glDrawElements(GL_TRIANGLES, sky.indices.size(), GL_UNSIGNED_INT, 0);
		sky.vao.release();
		shader.release();
	}
};
}
#endif
