#ifndef CELLGROUP_HPP
#define CELLGROUP_HPP
#include "viewtools.h"
#include "primitives/sphere.hpp"

template <typename Cell> class CellGroup {
	QOpenGLShaderProgram shader;
	unique_ptr<QOpenGLTexture> normalMap = nullptr;
	IcoSphere sphere;

 public:
	CellGroup() {}

	void load() {
		shader.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/cell.vert");
		shader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/cell.frag");
		shader.link();
		normalMap = unique_ptr<QOpenGLTexture>(new QOpenGLTexture(QImage(":/textures/cellNormalMap.jpg").mirrored()));
		normalMap->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
		normalMap->setMagnificationFilter(QOpenGLTexture::Linear);
		sphere.load(shader);
	}

	void draw(const vector<Cell*>& cells, const QMatrix4x4& view, const QMatrix4x4& projection) {
		shader.bind();
		sphere.vao.bind();
		normalMap->bind(0);
		GL->glActiveTexture(GL_TEXTURE0);
		GL->glBindTexture(GL_TEXTURE_2D, normalMap->textureId());
		shader.setUniformValue(shader.uniformLocation("nmap"), 0);
		shader.setUniformValue(shader.uniformLocation("projection"), projection);
		for (auto& c : cells) {
			QMatrix4x4 model;
			double radius = c->getRadius();
			QVector3D center = toQV3D(c->getPosition());
			model.translate(center);
			model.scale(QVector3D(radius, radius, radius));
			QVector3D color(1.0, 0.6, 0.2);
			QMatrix4x4 nmatrix = (model).inverted().transposed();
			shader.setUniformValue(shader.uniformLocation("model"), model);
			shader.setUniformValue(shader.uniformLocation("view"), view);
			shader.setUniformValue(shader.uniformLocation("normalMatrix"), nmatrix);
			shader.setUniformValue(shader.uniformLocation("color"), color);
			GL->glDrawElements(GL_TRIANGLES, sphere.indices.size(), GL_UNSIGNED_INT, 0);
		}
		sphere.vao.release();
		shader.release();
	}

	QVector3D getCellColor(Cell* c) { return QVector3D(c->getColor(0), c->getColor(1), c->getColor(2)); }
};

#endif
