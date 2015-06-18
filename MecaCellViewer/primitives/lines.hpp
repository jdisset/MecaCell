#ifndef LINES
#define LINES
#include <vector>
#include <iostream>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>

struct Lines {
	Lines() {}
	QOpenGLBuffer vbuf;
	QOpenGLVertexArrayObject vao;
	std::vector<float> vertices;
	void load(QOpenGLShaderProgram &shader) {
		shader.bind();
		vao.create();
		vao.bind();
		vbuf.create();
		vbuf.bind();
		vbuf.setUsagePattern(QOpenGLBuffer::StreamDraw);
		vbuf.allocate(&vertices[0], vertices.size() * sizeof(float));
		shader.enableAttributeArray(0);
		shader.setAttributeBuffer(0, GL_FLOAT, 0, 3);
		vao.release();
		shader.release();
	}
};

#endif
