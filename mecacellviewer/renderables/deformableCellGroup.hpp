#ifndef DEFORMABLECELLGROUP_HPP
#define DEFORMABLECELLGROUP_HPP
#include "../primitives/deformablesphere.hpp"
#include "../primitives/sphere.hpp"
#include "../utilities/viewtools.h"

namespace MecacellViewer {

template <typename R> class DeformableCellGroup : public PaintStep<R> {
	using C = typename R::Cell;
	QOpenGLShaderProgram shader;
	unique_ptr<QOpenGLTexture> normalMap = nullptr;
	DeformableSphere sphere;

 public:
	cellMode drawMode = plain;
	DeformableCellGroup() : PaintStep<R>("Cells"), sphere(350) {
		shader.addShaderFromSourceCode(QOpenGLShader::Vertex,
		                               shaderWithHeader(":/shaders/cell.vert"));
		shader.addShaderFromSourceCode(QOpenGLShader::Fragment,
		                               shaderWithHeader(":/shaders/cell.frag"));
		shader.link();
		normalMap = unique_ptr<QOpenGLTexture>(
		    new QOpenGLTexture(QImage(":/textures/cellNormalMap.jpg").mirrored()));
		normalMap->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
		normalMap->setMagnificationFilter(QOpenGLTexture::Linear);
		sphere.load(shader);
	}

	vector<QVector4D> getColorVector(const C *c, bool selected,
	                                 const ColorMode &colormode = color_normal) {
		return vector<QVector4D>(sphere.vert.size(),
		                         cellColorToQVector(c, selected, colormode));
	}

	void call(R *r, const ColorMode &colormode = color_normal) {
		const auto &cells = r->getScenario().getWorld().cells;
		if (cells.size() > 0) {
			const QMatrix4x4 view = r->getViewMatrix();
			const QMatrix4x4 projection = r->getProjectionMatrix();
			const auto viewV = r->getCamera().getViewVector();
			const auto camPos = r->getCamera().getPosition();
			const auto *selected = r->getSelectedCell();
			shader.bind();
			sphere.vao.bind();
			normalMap->bind(0);
			GL()->glActiveTexture(GL_TEXTURE0);
			GL()->glBindTexture(GL_TEXTURE_2D, normalMap->textureId());
			shader.setUniformValue(shader.uniformLocation("nmap"), 0);
			shader.setUniformValue(shader.uniformLocation("projection"), projection);
			shader.setUniformValue(shader.uniformLocation("view"), view);
			decltype((*cells.begin())->getPosition()) viewVec(viewV.x(), viewV.y(), viewV.z());
			decltype((*cells.begin())->getPosition()) camVec(camPos.x(), camPos.y(),
			                                                 camPos.z());
			for (auto &c : cells) {
				if (c->getVisible()) {
					QMatrix4x4 model;
					QVector3D center = toQV3D(c->getPosition());
					model.translate(center);
					QQuaternion rotation =
					    QQuaternion::fromAxisAndAngle(toQV3D(c->getOrientationRotation().n),
					                                  radToDeg(c->getOrientationRotation().teta));
					model.rotate(rotation);
					if (drawMode == plain) {
						auto newvert = sphere.vert;
						for (auto &v : newvert) {
							auto vr = rotation.rotatedVector(v);
							decltype((*cells.begin())->getPosition()) d(vr.x(), vr.y(), vr.z());
							v *= std::max(c->getMembraneDistance(d), c->getBoundingBoxRadius());
						}
						sphere.update(newvert, getColorVector(c, selected == c, colormode), shader);
					} else {
						model.scale(QVector3D(0.1, 0.1, 0.1));
					}
					QMatrix4x4 nmatrix = (model).inverted().transposed();
					shader.setUniformValue(shader.uniformLocation("model"), model);
					shader.setUniformValue(shader.uniformLocation("normalMatrix"), nmatrix);
					GL()->glDrawElements(GL_TRIANGLES, sphere.indices.size(), GL_UNSIGNED_INT, 0);
				}
			}
			sphere.vao.release();
			shader.release();
		}
	}
};
}  // namespace MecacellViewer
#endif
