#ifndef CONNECTIONSGROUP_HPP
#define CONNECTIONSGROUP_HPP
#include "viewtools.h"
#include "primitives/lines.hpp"
#include <vector>
class ConnectionsGroup {
	QOpenGLShaderProgram shader;
	Lines lines;

public:
	ConnectionsGroup() {}

	void load() {
		shader.addShaderFromSourceCode(QOpenGLShader::Vertex, shaderWithHeader(":/shaders/viewprojection.vert"));
		shader.addShaderFromSourceCode(QOpenGLShader::Geometry, shaderWithHeader(":/shaders/line.geom"));
		shader.addShaderFromSourceCode(QOpenGLShader::Fragment, shaderWithHeader(":/shaders/line.frag"));
		shader.link();
		lines.load(shader);
	}

	template <typename Cell, typename modelConn_ptr>
	void drawModelConnections(
	    const unordered_map<string, unordered_map<Cell *, unordered_map<size_t, modelConn_ptr>>> &co,
	    const QMatrix4x4 &view, const QMatrix4x4 &projection) {
		lines.vertices = std::vector<float>();
		shader.bind();
		lines.vao.bind();
		lines.vbuf.bind();
		lines.vbuf.allocate(&lines.vertices[0], lines.vertices.size() * sizeof(float));
		QVector4D color(0.9, 0.2, 0.1, 1.0);
		shader.setUniformValue(shader.uniformLocation("viewProjection"), projection * view);
		shader.setUniformValue(shader.uniformLocation("color"), color);
		GL->glDrawArrays(GL_LINES, 0, lines.vertices.size() / 3.0);
		lines.vertices = std::vector<float>();
		for (auto &p : co) {
			for (auto &c : p.second) {
				for (auto &conne : c.second) {
					// basic spring
					auto F0 = conne.second->second.getNode0(); // ModelConnectionPoint&
					auto F1 = conne.second->second.getNode1(); // Cell *

					QVector3D center0 = toQV3D(F0.getPosition());
					QVector3D center1 = toQV3D(F1->getPosition());
					lines.vertices.push_back(center0.x());
					lines.vertices.push_back(center0.y());
					lines.vertices.push_back(center0.z());
					lines.vertices.push_back(center1.x());
					lines.vertices.push_back(center1.y());
					lines.vertices.push_back(center1.z());
				}
			}
		}
		lines.vbuf.allocate(&lines.vertices[0], lines.vertices.size() * sizeof(float));
		color = QVector4D(0.0, 0.4, 0.8, 1.0);
		shader.setUniformValue(shader.uniformLocation("viewProjection"), projection * view);
		shader.setUniformValue(shader.uniformLocation("color"), color);
		GL->glDrawArrays(GL_LINES, 0, lines.vertices.size() / 3.0);
		lines.vao.release();
		shader.release();
	}

	template <typename C>
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
