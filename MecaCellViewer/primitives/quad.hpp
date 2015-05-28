#ifndef QUADP
#define QUADP
#include <vector>
#include <iostream>
#include <cmath>
#include <QVector3D>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>

struct Quad {
	QOpenGLBuffer vbuf, nbuf;
	QOpenGLVertexArrayObject vao;
	const std::vector<float> vertices = {-1, -1, 0, 1, -1, 0, -1, 1, 0, 1, 1, 0};
	const std::vector<float> normals = {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1};

	Quad() {}
	void load(QOpenGLShaderProgram& shader) {
		shader.bind();
		vao.create();
		vao.bind();

		vbuf.create();
		vbuf.bind();
		vbuf.allocate(&vertices[0], vertices.size() * sizeof(float));
		shader.enableAttributeArray(0);
		shader.setAttributeBuffer(0, GL_FLOAT, 0, 3);

		nbuf.create();
		nbuf.bind();
		nbuf.allocate(&normals[0], normals.size() * sizeof(float));
		shader.enableAttributeArray(1);
		shader.setAttributeBuffer(1, GL_FLOAT, 0, 3);

		vao.release();
		shader.release();
	}
};

#endif
