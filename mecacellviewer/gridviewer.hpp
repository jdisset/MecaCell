#ifndef GRIDVIEWER_HPP
#define GRIDVIEWER_HPP
#include "viewtools.h"
#include "primitives/cube.hpp"
#include "paintstep.hpp"
#include <functional>
#include <QString>
#include <QMatrix4x4>
#include <QVector4D>

namespace MecacellViewer {
template <typename R, typename G> class GridViewer : public PaintStep<R> {
	QOpenGLShaderProgram shader;
	Cube cube;
	std::function<const G *(R *r)> getGrid;

 public:
	GridViewer(QString n, decltype(getGrid) gg, const QString &vs, const QString &fs)
	    : PaintStep<R>(n), getGrid(gg) {
		shader.addShaderFromSourceCode(QOpenGLShader::Vertex, shaderWithHeader(vs));
		shader.addShaderFromSourceCode(QOpenGLShader::Fragment, shaderWithHeader(fs));
		shader.link();
		cube.load(shader);
	}

	void call(R *r) {
		const G &g = *getGrid(r);
		const QMatrix4x4 &view = r->getViewMatrix();
		const QMatrix4x4 &projection = r->getProjectionMatrix();
		shader.bind();
		cube.vao.bind();
		shader.setUniformValue(shader.uniformLocation("projection"), projection);
		shader.setUniformValue(shader.uniformLocation("view"), view);
		double cellSize = g.getCellSize();
		for (const auto &c : g.getContent()) {
			QMatrix4x4 model;
			double v = g.vecToColor(c.first);
			double h = v / 8.0;
			QColor col = QColor::fromHsvF(h, 1, 0.65);
			model.translate(toQV3D(c.first) * cellSize);
			model.scale(cellSize * 0.5, cellSize * 0.5, cellSize * 0.5);
			QMatrix4x4 nmatrix = (model).inverted().transposed();
			shader.setUniformValue(shader.uniformLocation("model"), model);
			shader.setUniformValue(shader.uniformLocation("normalMatrix"), nmatrix);
			shader.setUniformValue(shader.uniformLocation("color"), col);
			GL->glDrawElements(GL_TRIANGLES, cube.indices.size(), GL_UNSIGNED_INT, 0);
		}
		cube.vao.release();
		shader.release();
	}
};
}
#endif
