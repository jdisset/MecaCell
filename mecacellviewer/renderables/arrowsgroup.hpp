#ifndef ARROWSGROUP_HPP
#define ARROWSGROUP_HPP
#include <QDebug>
#include <vector>
#include "../paintstep.hpp"
#include "../primitives/cube.hpp"
#include "../utilities/viewtools.h"

namespace MecacellViewer {
template <typename R> class ArrowsGroup : public PaintStep<R> {
	QOpenGLShaderProgram shader;
	Cube cube;
	std::function<const vector<pair<QVector3D, QVector3D>>(R *)> getArrows;
	QVector4D color = QVector4D(1.0, 1.0, 1.0, 1.0);
	double scaleCoef = 1.0;

 public:
	ArrowsGroup(double sc = 1.0) : scaleCoef(sc) {
		shader.addShaderFromSourceCode(QOpenGLShader::Vertex,
		                               shaderWithHeader(":/shaders/mvp.vert"));
		shader.addShaderFromSourceCode(QOpenGLShader::Fragment,
		                               shaderWithHeader(":/shaders/flat.frag"));
		shader.link();
		cube.load(shader);
	}

	void call(R *r, vector<pair<QVector3D, QVector3D>> &arrows, QVector4D c) {
		color = c;
		const auto &view = r->getViewMatrix();
		const auto &projection = r->getProjectionMatrix();
		shader.bind();
		cube.vao.bind();
		shader.setUniformValue(shader.uniformLocation("color"), color);
		shader.setUniformValue(shader.uniformLocation("projection"), projection);
		shader.setUniformValue(shader.uniformLocation("view"), view);
		for (auto &a : arrows) {
			QMatrix4x4 model;
			model.translate(a.first + a.second * scaleCoef);
			auto dp = a.second.normalized().x();
			if (dp != 1 && dp != -1) {
				model.rotate(acos(dp) * 180.0 / M_PI,
				             QVector3D::crossProduct(QVector3D(1, 0, 0), a.second));
			}
			model.scale(a.second.length() * scaleCoef, 1.0, 1.0);
			QMatrix4x4 nmatrix = (model).inverted().transposed();
			shader.setUniformValue(shader.uniformLocation("model"), model);
			shader.setUniformValue(shader.uniformLocation("normalMatrix"), nmatrix);
			GL()->glDrawElements(GL_TRIANGLES, cube.indices.size(), GL_UNSIGNED_INT, 0);
		}
		cube.vao.release();
		shader.release();
	}
};
}
#endif
