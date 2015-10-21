#ifndef GRIDVIEWER_HPP
#define GRIDVIEWER_HPP
#include "viewtools.h"
#include "primitives/cube.hpp"

namespace MecacellViewer {
class GridViewer {
	QOpenGLShaderProgram shader;
	Cube cube;

 public:
	GridViewer(){};
	void load(const QString &vs, const QString &fs) {
		shader.addShaderFromSourceCode(QOpenGLShader::Vertex, shaderWithHeader(vs));
		shader.addShaderFromSourceCode(QOpenGLShader::Fragment, shaderWithHeader(fs));
		shader.link();
		cube.load(shader);
	}

	template <typename G>
	void draw(const G &g, const QMatrix4x4 &view, const QMatrix4x4 &projection,
	          const QVector4D &color) {
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
