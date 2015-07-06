#ifndef CELLGROUP_HPP
#define CELLGROUP_HPP
#include "viewtools.h"
#include "primitives/sphere.hpp"

enum cellMode { plain, centers };
template <typename C> class CellGroup {
	QOpenGLShaderProgram shader;
	unique_ptr<QOpenGLTexture> normalMap = nullptr;
	IcoSphere sphere;

public:
	bool cut = false;
	cellMode drawMode;
	CellGroup() {}

	void load() {
		shader.addShaderFromSourceCode(QOpenGLShader::Vertex, shaderWithHeader(":/shaders/cell.vert"));
		shader.addShaderFromSourceCode(QOpenGLShader::Fragment, shaderWithHeader(":/shaders/cell.frag"));
		shader.link();
		shader.link();
		normalMap =
		    unique_ptr<QOpenGLTexture>(new QOpenGLTexture(QImage(":/textures/cellNormalMap.jpg").mirrored()));
		normalMap->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
		normalMap->setMagnificationFilter(QOpenGLTexture::Linear);
		sphere.load(shader);
	}

	void draw(const vector<C *> &cells, const QMatrix4x4 &view, const QMatrix4x4 &projection,
	          const QVector3D &viewV, const QVector3D &camPos, const C *selected = nullptr) {
		if (cells.size() > 0) {
			shader.bind();
			sphere.vao.bind();
			normalMap->bind(0);
			GL->glActiveTexture(GL_TEXTURE0);
			GL->glBindTexture(GL_TEXTURE_2D, normalMap->textureId());
			shader.setUniformValue(shader.uniformLocation("nmap"), 0);
			shader.setUniformValue(shader.uniformLocation("projection"), projection);
			shader.setUniformValue(shader.uniformLocation("view"), view);
			vector<C *> sortedCells = cells;
			decltype((*cells.begin())->getPosition()) viewVec(viewV.x(), viewV.y(), viewV.z());
			decltype((*cells.begin())->getPosition()) camVec(camPos.x(), camPos.y(), camPos.z());

			std::sort(sortedCells.begin(), sortedCells.end(), [&](C *a, C *b) {
				return (a->getPosition() - camVec).sqlength() > (b->getPosition() - camVec).sqlength();
			});
			for (auto &c : sortedCells) {
				if (!culling || c->getConnectedCells().size() < 8) {
					QMatrix4x4 model;
					double radius = c->getRadius();
					QVector3D center = toQV3D(c->getPosition());
					if ((cut && QVector3D::dotProduct(center, QVector3D(1, 0, 0)) > 0) || !cut) {
						model.translate(center);
						if (drawMode == plain) {
							model.scale(QVector3D(radius, radius, radius));
						} else {
							model.scale(QVector3D(2.0, 2.0, 2.0));
						}
						QVector3D color(c->getColor(0), c->getColor(1), c->getColor(2));
						if (c == selected) color = QVector3D(1.0, 1.0, 1.0);
						QMatrix4x4 nmatrix = (model).inverted().transposed();
						shader.setUniformValue(shader.uniformLocation("model"), model);
						shader.setUniformValue(shader.uniformLocation("normalMatrix"), nmatrix);
						shader.setUniformValue(shader.uniformLocation("color"), color);
						GL->glDrawElements(GL_TRIANGLES, sphere.indices.size(), GL_UNSIGNED_INT, 0);
					}
				}
			}
			sphere.vao.release();
			shader.release();
		}
	}
};

#endif
