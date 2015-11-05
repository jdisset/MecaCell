#ifndef CONNECTIONSGROUP_HPP
#define CONNECTIONSGROUP_HPP
#include "viewtools.h"
#include "primitives/lines.hpp"
#include <vector>
namespace MecacellViewer {
class ConnectionsGroup {
	QOpenGLShaderProgram shader;
	Lines lines;

 public:
	ConnectionsGroup() {}

	void load() {
		shader.addShaderFromSourceCode(QOpenGLShader::Vertex,
		                               shaderWithHeader(":/shaders/viewprojection.vert"));
		shader.addShaderFromSourceCode(QOpenGLShader::Geometry,
		                               shaderWithHeader(":/shaders/line.geom"));
		shader.addShaderFromSourceCode(QOpenGLShader::Fragment,
		                               shaderWithHeader(":/shaders/line.frag"));
		shader.link();
		lines.load(shader);
	}

	template <typename Cell>
	void drawModelConnections(const vector<Cell *> &cells, const QMatrix4x4 &view,
	                          const QMatrix4x4 &projection) {
		shader.bind();
		lines.vao.bind();
		lines.vbuf.bind();
		QVector4D color(0.9, 0.2, 0.1, 1.0);
		shader.setUniformValue(shader.uniformLocation("viewProjection"), projection * view);
		lines.vertices = std::vector<float>();
		for (auto &c : cells) {
			for (auto &conne : c->getRWModelConnections()) {
				QVector3D center0 = toQV3D(conne->bounce.getNode0().getPosition());
				QVector3D center1 = toQV3D(conne->bounce.getNode1()->getPosition());
				lines.vertices.push_back(center0.x());
				lines.vertices.push_back(center0.y());
				lines.vertices.push_back(center0.z());
				lines.vertices.push_back(center1.x());
				lines.vertices.push_back(center1.y());
				lines.vertices.push_back(center1.z());
			}
		}
		lines.vbuf.allocate(&lines.vertices[0], lines.vertices.size() * sizeof(float));
		shader.setUniformValue(shader.uniformLocation("color"), color);
		GL->glDrawArrays(GL_LINES, 0, lines.vertices.size() / 3.0);
		lines.vertices = std::vector<float>();
		for (auto &c : cells) {
			for (auto &conne : c->getRWModelConnections()) {
				QVector3D center0 = toQV3D(conne->anchor.getNode0().getPosition());
				QVector3D center1 = toQV3D(conne->anchor.getNode1()->getPosition());
				lines.vertices.push_back(center0.x());
				lines.vertices.push_back(center0.y());
				lines.vertices.push_back(center0.z());
				lines.vertices.push_back(center1.x());
				lines.vertices.push_back(center1.y());
				lines.vertices.push_back(center1.z());
			}
		}
		lines.vbuf.allocate(&lines.vertices[0], lines.vertices.size() * sizeof(float));
		color = QVector4D(0.0, 0.4, 0.8, 1.0);
		shader.setUniformValue(shader.uniformLocation("color"), color);
		GL->glDrawArrays(GL_LINES, 0, lines.vertices.size() / 3.0);
		lines.vao.release();
		shader.release();
	}

	template <typename Cell>
	void draw(const vector<pair<Cell *, Cell *>> &cells, const QMatrix4x4 &view,
	          const QMatrix4x4 &projection) {
		lines.vertices = std::vector<float>();
		for (auto &c : cells) {
			QVector3D center0 = toQV3D(c.first->getPosition());
			QVector3D center1 = toQV3D(c.second->getPosition());
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
}
#endif

