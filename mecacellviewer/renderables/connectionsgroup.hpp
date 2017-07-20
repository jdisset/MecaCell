#ifndef CONNECTIONSGROUP_HPP
#define CONNECTIONSGROUP_HPP
#include <vector>
#include "../paintstep.hpp"
#include "../primitives/cube.hpp"
#include "../primitives/lines.hpp"
#include "../utilities/viewtools.h"
namespace MecacellViewer {
template <typename R> class ConnectionsGroup : public PaintStep<R> {
	QOpenGLShaderProgram shaderCube;
	Lines lines;
	Cube cube;

 public:
	ConnectionsGroup() : PaintStep<R>("connections") {
		shaderCube.addShaderFromSourceCode(QOpenGLShader::Vertex,
		                                   shaderWithHeader(":/shaders/mvp.vert"));
		shaderCube.addShaderFromSourceCode(QOpenGLShader::Fragment,
		                                   shaderWithHeader(":/shaders/flat.frag"));
		shaderCube.link();
		cube.load(shaderCube);
	}

	template <typename Cell>
	void draw(const vector<pair<Cell *, Cell *>> &cells, const QMatrix4x4 &view,
	          const QMatrix4x4 &projection) {
		shaderCube.bind();
		cube.vao.bind();
		shaderCube.setUniformValue(shaderCube.uniformLocation("projection"), projection);
		shaderCube.setUniformValue(shaderCube.uniformLocation("view"), view);
		for (auto &c : cells) {
			QMatrix4x4 model;
			QVector3D center0 = toQV3D(c.first->getPosition());
			QVector3D center1 = toQV3D(c.second->getPosition());
			QVector3D AB = center1 - center0;
			model.translate(center0 + (AB)*0.5);
			auto dp = AB.normalized().x();
			if (dp != 1 && dp != -1) {
				model.rotate(acos(dp) * 180.0 / M_PI,
				             QVector3D::crossProduct(QVector3D(1, 0, 0), AB));
			}
			model.scale(AB.length() * 0.5, 1.0, 1.0);
			QVector4D color(1.0, 0.7, 0.1, 1.0);
			QMatrix4x4 nmatrix = (model).inverted().transposed();
			shaderCube.setUniformValue(shaderCube.uniformLocation("model"), model);
			shaderCube.setUniformValue(shaderCube.uniformLocation("normalMatrix"), nmatrix);
			shaderCube.setUniformValue(shaderCube.uniformLocation("color"), color);
			GL()->glDrawElements(GL_TRIANGLES, cube.indices.size(), GL_UNSIGNED_INT, 0);
		}
		cube.vao.release();
		shaderCube.release();
	}
};
}
#endif
