#ifndef CONNECTIONSGROUP_HPP
#define CONNECTIONSGROUP_HPP
#include "viewtools.h"
#include "primitives/lines.hpp"
#include <vector>
template <typename C> class ConnectionsGroup {
	QOpenGLShaderProgram shader;
	Lines lines;

public:
	ConnectionsGroup() {}
	void load() {
		shader.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/viewprojection.vert");
		shader.addShaderFromSourceFile(QOpenGLShader::Geometry, ":/shaders/line.geom");
		shader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/line.frag");
		shader.link();
		lines.load(shader);
	}
	void draw(const vector<C *> &co, const QMatrix4x4 &view, const QMatrix4x4 &projection) {
		lines.vertices = std::vector<float>();
		for (auto &c : co) {
			auto cell0 = c->getNode0();
			auto cell1 = c->getNode1();
			QVector3D center0 = toQV3D(cell0->getPosition());
			QVector3D center1 = toQV3D(cell1->getPosition());
			lines.vertices.push_back(center0.x());
			lines.vertices.push_back(center0.y());
			lines.vertices.push_back(center0.z());
			lines.vertices.push_back(center1.x());
			lines.vertices.push_back(center1.y());
			lines.vertices.push_back(center1.z());
		}
		shader.bind();
		lines.vao.bind();
		lines.vbuf.bind();
		lines.vbuf.allocate(&lines.vertices[0], lines.vertices.size() * sizeof(float));
		QVector4D color(0.95, 0.8, 0.1, 1.0);
		shader.setUniformValue(shader.uniformLocation("viewProjection"), projection * view);
		shader.setUniformValue(shader.uniformLocation("color"), color);
		GL->glDrawArrays(GL_LINES, 0, lines.vertices.size() / 3.0);
		lines.vao.release();
		shader.release();
	}
};
#endif
