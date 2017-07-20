#ifndef MECACELLVIEWER_MODEL_HPP
#define MECACELLVIEWER_MODEL_HPP
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QThread>
#include <QVector3D>
#include <cmath>
#include <iostream>
#include <vector>
#include "../utilities/viewtools.h"

using std::vector;

namespace MecacellViewer {
struct Mesh {
	QOpenGLShaderProgram shader;
	QOpenGLVertexArrayObject vao;
	vector<float> vertices;
	vector<float> normals;
	vector<float> uv;
	vector<unsigned int> indices;
	QOpenGLBuffer vbuf, nbuf, tbuf, bitanbuf, ibuf;

	Mesh() {
		shader.addShaderFromSourceCode(QOpenGLShader::Vertex,
		                               shaderWithHeader(":/shaders/mvp.vert"));
		shader.addShaderFromSourceCode(QOpenGLShader::Fragment,
		                               shaderWithHeader(":/shaders/flat.frag"));
		shader.link();
	}

	template <typename V>
	void load(const std::vector<V> &vert,
	          const std::vector<std::array<double, 2>> &uvCoords, const std::vector<V> &nor,
	          const std::vector<std::array<std::array<size_t, 3>, 3>> &faces) {
		// extracting vertices, normals and uv (if available)
		vertices.clear();
		uv.clear();
		normals.clear();
		indices.clear();
		vertices.reserve(vert.size() * 3);
		for (const auto &v : vert) {
			vertices.push_back(v.x());
			vertices.push_back(v.y());
			vertices.push_back(v.z());
		}
		uv.resize(vert.size() * 2);
		normals.resize(vertices.size());
		int fid = 0;
		for (const auto &f : faces) {
			for (size_t i = 0; i < 3; ++i) {
				size_t vid = f[0][i];
				indices.push_back(vid);
				if (uvCoords.size() > 0) {
					uv[vid * 2] = uvCoords[f[1][i]][0];
					uv[vid * 2 + 1] = uvCoords[f[1][i]][1];
				}
				normals.push_back(i);
				normals.push_back(i);
				normals.push_back(i);
				normals[vid * 3] = i;
				normals[vid * 3 + 1] = i * 2;
				normals[vid * 3 + 2] = i;
			}
		}

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

		// tbuf.create();
		// tbuf.bind();
		// tbuf.allocate(&uv[0], uv.size() * sizeof(float));
		// shader.enableAttributeArray("texCoord");
		// shader.setAttributeBuffer("texCoord", GL_FLOAT, 0, 2);

		ibuf = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
		ibuf.create();
		ibuf.bind();
		ibuf.allocate(&indices[0], indices.size() * sizeof(unsigned int));

		vao.release();
		/*shader.release();*/
	}

	void draw(const QMatrix4x4 &viewMatrix, const QMatrix4x4 &projectionMatrix,
	          const QMatrix4x4 &modelMatrix) {
		shader.bind();
		vao.bind();
		shader.setUniformValue(shader.uniformLocation("projection"), projectionMatrix);
		shader.setUniformValue(shader.uniformLocation("view"), viewMatrix);
		shader.setUniformValue(shader.uniformLocation("model"), modelMatrix);
		QVector4D color(1.0, 1.0, 1.0, 1.0);
		shader.setUniformValue(shader.uniformLocation("color"), color);
		QMatrix4x4 nmatrix = modelMatrix.inverted().transposed();
		shader.setUniformValue(shader.uniformLocation("normalMatrix"), nmatrix);
		GL()->glDisable(GL_CULL_FACE);
		GL()->glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		vao.release();
		shader.release();
	}
};
}
#endif
