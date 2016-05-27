#ifndef MECACELLVIEWER_MODEL_HPP
#define MECACELLVIEWER_MODEL_HPP
#include <vector>
#include "viewtools.h"

using std::vector;

namespace MecacellViewer {
template <typename M> struct Model {
	QOpenGLShaderProgram shader;
	QOpenGLVertexArrayObject vao;
	vector<float> vertices;
	vector<float> normals;
	vector<float> uv;
	vector<unsigned int> indices;
	QOpenGLBuffer vbuf, nbuf, tbuf, bitanbuf, ibuf;
	Model() {
		shader.addShaderFromSourceCode(QOpenGLShader::Vertex,
		                               shaderWithHeader(":/shaders/mvp.vert"));
		shader.addShaderFromSourceCode(QOpenGLShader::Fragment,
		                               shaderWithHeader(":/shaders/flat.frag"));
		shader.link();
	}

	void load(const M &m) {
		// extracting vertices, normals and uv (if available)
		for (auto &v : m.vertices) {
			vertices.push_back(v.x());
			vertices.push_back(v.y());
			vertices.push_back(v.z());
		}
		normals.resize(vertices.size());
		for (auto &f : m.obj.faces) {
			assert(f.count("v") && f.count("n"));
			for (auto &vid : f.at("v").indices) {
				assert(vid < m.obj.vertices.size());
				indices.push_back(vid);
			}

			for (int id = 0; id < 3; ++id) {
				size_t vid = f.at("v").indices[id];
				size_t nid = f.at("n").indices[id];
				normals[vid * 3 + 0] = m.normals[nid].x();
				normals[vid * 3 + 1] = m.normals[nid].y();
				normals[vid * 3 + 2] = m.normals[nid].z();
			}
		}

		// TODO : directly use model's transformed vertices and normals

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
		shader.release();
	}

	void draw(const QMatrix4x4 &view, const QMatrix4x4 &projection, const M &m) {
		shader.bind();
		vao.bind();
		QMatrix4x4 model;
		shader.setUniformValue(shader.uniformLocation("projection"), projection);
		shader.setUniformValue(shader.uniformLocation("view"), view);
		shader.setUniformValue(shader.uniformLocation("model"), model);
		QVector4D color(1.0, 1.0, 1.0, 1.0);
		shader.setUniformValue(shader.uniformLocation("color"), color);
		QMatrix4x4 nmatrix = model.inverted().transposed();
		shader.setUniformValue(shader.uniformLocation("normalMatrix"), nmatrix);
		// GL->glDisable(GL_CULL_FACE);
		GL->glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		vao.release();
		shader.release();
	}
};
}
#endif
