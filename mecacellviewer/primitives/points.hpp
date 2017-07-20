#ifndef POINTS_HPP
#define POINTS_HPP
#include "viewtools.h"
#include "primitives/deformablesphere.hpp"

namespace MecacellViewer {
class Points {
	QOpenGLShaderProgram shader;
	DeformableSphere sphere;

 public:
	Points() : sphere(20) {}

	void load() {
		shader.addShaderFromSourceCode(QOpenGLShader::Vertex,
		                               shaderWithHeader(":/shaders/point.vert"));
		shader.addShaderFromSourceCode(QOpenGLShader::Fragment,
		                               shaderWithHeader(":/shaders/point.frag"));
		shader.link();
		sphere.load(shader);
	}

	void draw(vector<tuple<QVector3D, QVector4D, double>> &points,
	          const QMatrix4x4 &viewprojection) {
		shader.bind();
		sphere.vao.bind();
		shader.setUniformValue(shader.uniformLocation("vp"), viewprojection);
		for (auto &p : points) {
			QMatrix4x4 model;
			model.translate(get<0>(p));
			double r = get<2>(p);
			model.scale(r, r, r);
			shader.setUniformValue(shader.uniformLocation("model"), model);
			shader.setUniformValue(shader.uniformLocation("color"), get<1>(p));
			GL()->glDrawElements(GL_TRIANGLES, sphere.indices.size(), GL_UNSIGNED_INT, 0);
		}
		sphere.vao.release();
		shader.release();
	}
};
}
#endif
