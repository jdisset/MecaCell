#ifndef DEFDISK
#define DEFDISK
#include <QVector3D>
#include <cmath>
#include <iostream>
#include <map>
#include <vector>
#include "../utilities/viewtools.h"

using namespace std;

namespace MecacellViewer {
struct DeformableDisk {
	QOpenGLBuffer vbuf, nbuf, ibuf;
	QOpenGLVertexArrayObject vao;

	vector<QVector3D> vert;
	vector<float> vertices;
	vector<float> normals;
	vector<int> indices;

	DeformableDisk(){};
	DeformableDisk(size_t n) : ibuf(QOpenGLBuffer(QOpenGLBuffer::IndexBuffer)) {
		if (n < 3) n = 3;
		vert.push_back(QVector3D(0, 0, 0));
		for (double i = 0.0; i < 2.0 * M_PI; i += 2.0 * M_PI / static_cast<double>(n)) {
			vert.push_back(QVector3D(cos(i), sin(i), 0.0));
		}
		for (size_t i = 1; i < vert.size(); ++i) {
			indices.push_back(0);
			indices.push_back(i);
			indices.push_back(i < vert.size() - 1 ? i + 1 : 1);
		}
		for (const auto& v : vert) {
			vertices.push_back(v.x());
			vertices.push_back(v.y());
			vertices.push_back(v.z());
			normals.push_back(0);
			normals.push_back(0);
			normals.push_back(1);
		}
	}

	void update(const vector<QVector3D>& nv, QOpenGLShaderProgram& shader) {
		assert(nv.size() == vert.size());
		vertices.clear();
		for (auto& v : nv) {
			vertices.push_back(v.x());
			vertices.push_back(v.y());
			vertices.push_back(v.z());
		}
		vao.bind();
		vbuf.bind();
		vbuf.allocate(&vertices[0], vertices.size() * sizeof(float));
	}

	void load(QOpenGLShaderProgram& shader) {
		shader.bind();
		vao.create();
		vao.bind();
		vbuf.create();
		vbuf.bind();
		vbuf.setUsagePattern(QOpenGLBuffer::StreamDraw);
		vbuf.allocate(&vertices[0], vertices.size() * sizeof(float));
		shader.enableAttributeArray("position");
		shader.setAttributeBuffer("position", GL_FLOAT, 0, 3);

		nbuf.create();
		nbuf.bind();
		nbuf.setUsagePattern(QOpenGLBuffer::StreamDraw);
		nbuf.allocate(&normals[0], normals.size() * sizeof(float));
		shader.enableAttributeArray("normal");
		shader.setAttributeBuffer("normal", GL_FLOAT, 0, 3);

		ibuf.create();
		ibuf.bind();
		ibuf.allocate(&indices[0], indices.size() * sizeof(int));
		vao.release();
		shader.release();
	}
};
}
#endif
