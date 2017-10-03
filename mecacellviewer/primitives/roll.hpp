#ifndef ROLL_HPP 
#define ROLL_HPP  // roll == empty cylinder
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <vector>

struct Roll {
	const float H = 2;
	const float diam = 2;
	int nDiv = 12;
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<unsigned int> indices;

	QOpenGLBuffer vbuf, nbuf, ibuf;
	QOpenGLVertexArrayObject vao;

	Roll(int n = 12) : nDiv(n) { generate(); }

	std::vector<QVector3D> generateDiskVertices(float d, int n) {
		std::vector<QVector3D> result;
		for (int i = 0; i < n; ++i) {
			float theta = 2.0f * M_PI * static_cast<float>(i) / static_cast<float>(n);
			result.push_back({d * 0.5f * cos(theta), 0.0f, d * 0.5f * sin(theta)});
		}
		return result;
	}

	void generate() {
		auto diskVertices = generateDiskVertices(2.0f, nDiv);
		std::vector<QVector3D> vertices_3D;
		std::vector<QVector3D> normals_3D;
		unsigned int sideUpBegin = vertices_3D.size();
		for (const auto &v : diskVertices) {
			vertices_3D.push_back({v.x(), H / 2.0f, v.z()});
			normals_3D.push_back(v.normalized());
		}
		unsigned int sideBottomBegin = vertices_3D.size();
		for (const auto &v : diskVertices) {
			vertices_3D.push_back({v.x(), -H / 2.0f, v.z()});
			normals_3D.push_back(v.normalized());
		}
		for (size_t i = 0; i < diskVertices.size(); ++i) {
			// up, down, up_left
			indices.push_back(sideUpBegin + i);
			indices.push_back(sideBottomBegin + i);
			indices.push_back(sideUpBegin + i + 1);
			// down, down_left, up_left
			indices.push_back(sideBottomBegin + i);
			indices.push_back(sideBottomBegin + i + 1);
			indices.push_back(sideUpBegin + i + 1);
		}
		indices[indices.size() - 1] = sideUpBegin;
		indices[indices.size() - 2] = sideBottomBegin;
		indices[indices.size() - 4] = sideUpBegin;

		assert(normals_3D.size() == vertices_3D.size());

		for (const auto &v : vertices_3D) {
			vertices.push_back(v.x());
			vertices.push_back(v.y());
			vertices.push_back(v.z());
		}
		for (const auto &n : normals_3D) {
			normals.push_back(n.x());
			normals.push_back(n.y());
			normals.push_back(n.z());
		}
	}

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

