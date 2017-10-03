#ifndef CUBE_HPP
#define CUBE_HPP
#include <vector>
#include <iostream>
#include <cmath>
#include <QVector3D>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

struct Cube {
	QOpenGLBuffer vbuf, nbuf, ibuf;
	QOpenGLVertexArrayObject vao;
	const std::vector<float> vertices = {-1.0, -1.0, 1.0,  1.0, -1.0, 1.0,  1.0, 1.0, 1.0,  -1.0, 1.0, 1.0,
	                                     -1.0, -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 1.0, -1.0};
	const std::vector<float> normals = {-1.0, -1.0, 1.0,  1.0, -1.0, 1.0,  1.0, 1.0, 1.0,  -1.0, 1.0, 1.0,
	                                    -1.0, -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 1.0, -1.0};
	const std::vector<unsigned int> indices = {0, 1, 2, 2, 3, 0, 3, 2, 6, 6, 7, 3, 7, 6, 5, 5, 4, 7,
	                                          4, 0, 3, 3, 7, 4, 0, 1, 5, 5, 4, 0, 1, 5, 6, 6, 2, 1};
	Cube() {}
void load(QOpenGLShaderProgram &shader) {
		shader.bind();
		vao.create();
		vao.bind();

		vbuf.create();
		vbuf.bind();
		vbuf.allocate(&vertices[0], vertices.size() * sizeof(float));
		shader.enableAttributeArray("position");
		shader.setAttributeBuffer("position", GL_FLOAT, 0, 3);

		nbuf.create();
		nbuf.bind();
		nbuf.allocate(&normals[0], normals.size() * sizeof(float));
		shader.enableAttributeArray("normal");
		shader.setAttributeBuffer("normal", GL_FLOAT, 0, 3);

		ibuf = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
		ibuf.create();
		ibuf.bind();
		ibuf.allocate(&indices[0], indices.size() * sizeof(unsigned int));

		vao.release();
		shader.release();
	}
};

#endif
