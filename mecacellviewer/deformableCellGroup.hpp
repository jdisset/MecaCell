#ifndef DEFORMABLECELLGROUP_HPP
#define DEFORMABLECELLGROUP_HPP
#include "viewtools.h"
#include "primitives/deformablesphere.hpp"
#include "primitives/sphere.hpp"

namespace MecacellViewer {

template <typename C> class DeformableCellGroup {
	QOpenGLShaderProgram shader;
	unique_ptr<QOpenGLTexture> normalMap = nullptr;
	DeformableSphere sphere;

 public:
	cellMode drawMode = plain;
	DeformableCellGroup() : sphere(150) {}

	void load() {
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

	void draw(const vector<C *> &cells, const QMatrix4x4 &view,
	          const QMatrix4x4 &projection, const QVector3D &viewV, const QVector3D &camPos,
	          const colorMode &cm, const C *selected = nullptr) {
		if (cells.size() > 0) {
			shader.bind();
			sphere.vao.bind();
			normalMap->bind(0);
			GL->glActiveTexture(GL_TEXTURE0);
			GL->glBindTexture(GL_TEXTURE_2D, normalMap->textureId());
			shader.setUniformValue(shader.uniformLocation("nmap"), 0);
			shader.setUniformValue(shader.uniformLocation("projection"), projection);
			shader.setUniformValue(shader.uniformLocation("view"), view);
			decltype((*cells.begin())->getPosition()) viewVec(viewV.x(), viewV.y(), viewV.z());
			decltype((*cells.begin())->getPosition()) camVec(camPos.x(), camPos.y(),
			                                                 camPos.z());
			for (auto &c : cells) {
				if (c->getVisible()) {
					QMatrix4x4 model;
					double radius = c->getBoundingBoxRadius();
					QVector3D center = toQV3D(c->getPosition());
					model.translate(center);
					// model.rotate(radToDeg(c->getOrientationRotation().teta),
					// toQV3D(c->getOrientationRotation().n));
					if (drawMode == plain) {
						auto newvert = sphere.vert;
						for (auto &v : newvert) {
							decltype((*cells.begin())->getPosition()) d(v.x(), v.y(), v.z());
							v *= c->getMembraneDistance(-d);
						}
						sphere.update(newvert, shader);
					} else {
						model.scale(QVector3D(0.1, 0.1, 0.1));
					}
					QVector3D color(c->getColor(0), c->getColor(1), c->getColor(2));
					if (cm == pressure) {
						QColor co;
						co.setHsvF(mix(0.0, 0.7, 1.0 - c->getNormalizedPressure()), 0.8, 0.8);
						color = QVector3D(co.redF(), co.greenF(), co.blueF());
					}
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
