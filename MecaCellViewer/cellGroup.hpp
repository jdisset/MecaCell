#ifndef CELLGROUP_HPP
#define CELLGROUP_HPP
#include "viewtools.h"
#include "primitives/sphere.hpp"

template <typename C> class CellGroup {
	QOpenGLShaderProgram shader;
	unique_ptr<QOpenGLTexture> normalMap = nullptr;
	IcoSphere sphere;

 public:
	CellGroup() {}

	void load() {
		shader.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/cell.vert");
		shader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/cell.frag");
		shader.link();
		normalMap =
		    unique_ptr<QOpenGLTexture>(new QOpenGLTexture(QImage(":/textures/cellNormalMap.jpg").mirrored()));
		normalMap->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
		normalMap->setMagnificationFilter(QOpenGLTexture::Linear);
		sphere.load(shader);
	}

	void draw(const vector<C*>& cells, const QMatrix4x4& view, const QMatrix4x4& projection,
	          const C* selected = nullptr) {
		shader.bind();
		sphere.vao.bind();
		normalMap->bind(0);
		GL->glActiveTexture(GL_TEXTURE0);
		GL->glBindTexture(GL_TEXTURE_2D, normalMap->textureId());
		shader.setUniformValue(shader.uniformLocation("nmap"), 0);
		shader.setUniformValue(shader.uniformLocation("projection"), projection);
		shader.setUniformValue(shader.uniformLocation("view"), view);
		for (auto& c : cells) {
			QMatrix4x4 model;
			double radius = c->getRadius();
			QVector3D center = toQV3D(c->getPosition());
			model.translate(center);
			model.scale(QVector3D(radius, radius, radius));
			QVector3D color(c->getColor(0), c->getColor(1), c->getColor(2));
			if (c == selected) color = QVector3D(1.0, 1.0, 1.0);
			QMatrix4x4 nmatrix = (model).inverted().transposed();
			shader.setUniformValue(shader.uniformLocation("model"), model);
			shader.setUniformValue(shader.uniformLocation("normalMatrix"), nmatrix);
			shader.setUniformValue(shader.uniformLocation("color"), color);
			GL->glDrawElements(GL_TRIANGLES, sphere.indices.size(), GL_UNSIGNED_INT, 0);
		}
		sphere.vao.release();
		shader.release();
	}
};

#endif
