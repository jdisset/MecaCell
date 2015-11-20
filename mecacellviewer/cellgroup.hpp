#ifndef CELLGROUP_HPP
#define CELLGROUP_HPP
#include "viewtools.h"
#include "primitives/sphere.hpp"

namespace MecacellViewer {
template <typename R> class CellGroup : public PaintStep<R> {
	QOpenGLShaderProgram shader;
	unique_ptr<QOpenGLTexture> normalMap = nullptr;
	IcoSphere sphere;

 public:
	cellMode drawMode = plain;
	CellGroup() : name("cells") {
		shader.addShaderFromSourceCode(QOpenGLShader::Vertex,
		                               shaderWithHeader(":/shaders/cell.vert"));
		shader.addShaderFromSourceCode(QOpenGLShader::Fragment,
		                               shaderWithHeader(":/shaders/cell.frag"));
		shader.link();
		shader.link();
		normalMap = unique_ptr<QOpenGLTexture>(
		    new QOpenGLTexture(QImage(":/textures/cellNormalMap.jpg").mirrored()));
		normalMap->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
		normalMap->setMagnificationFilter(QOpenGLTexture::Linear);
		sphere.load(shader);
	}

	void call(R *r) {
		const auto &cells = r->getScenario()->getWorld().cells;
		if (cells.size() > 0) {
			const QMatrix4x4 view = r->getViewMatrix();
			const QMatrix4x4 projection = r->getViewMatrix();
			const auto &viewV = r->getCamera().getViewVector();
			const auto &capPos = r->getCamera().getPosition();
			const auto *selected = r->getSelectedCell();
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
			decltype((*cells.begin())->getPosition()) camVec(camPos.x(), camPos.y(),
			                                                 camPos.z());
			std::sort(sortedCells.begin(), sortedCells.end(), [&](C *a, C *b) {
				return (a->getPosition() - camVec).sqlength() >
				       (b->getPosition() - camVec).sqlength();
			});
			for (auto &c : sortedCells) {
				if (c->getVisible()) {
					QMatrix4x4 model;
					double radius = c->getBoundingBoxRadius();
					QVector3D center = toQV3D(c->getPosition());
					model.translate(center);
					model.rotate(radToDeg(c->getOrientationRotation().teta),
					             toQV3D(c->getOrientationRotation().n));
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
			sphere.vao.release();
			shader.release();
		}
	}
};
}
#endif
