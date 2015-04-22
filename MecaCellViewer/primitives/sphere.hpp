#ifndef ICOSPHERE2
#define ICOSPHERE2
#include <vector>
#include <map>
#include <iostream>
#include <cmath>
#include <QDateTime>
#include <QVector3D>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>

using namespace std;
class IcoSphere {
 public:
	int iterations;
	QOpenGLVertexArrayObject vao;
	QOpenGLBuffer vbuf, nbuf, tanbuf, tbuf, bitanbuf, ibuf;
	// vertex, normal, tangent, bitangent, indices

	vector<float> vertices;
	vector<float> normals;
	vector<float> tangents;
	vector<float> bitangents;
	vector<int> indices;
	vector<float> texCoords;

	map<long long int, unsigned int> middlePointCache;
	QVector3D specularColor;

	void createTangents() {
		for (size_t i = 0; i < vertices.size(); i += 3) {
			QVector3D p(vertices[i + 0], 0, vertices[i + 2]);
			p.normalize();
			QVector3D T = QVector3D::crossProduct(p, QVector3D(0, 1.0, 0));
			QVector3D B = QVector3D::crossProduct(p, T);
			tangents.push_back(T.x());
			tangents.push_back(T.y());
			tangents.push_back(T.z());
			bitangents.push_back(B.x());
			bitangents.push_back(B.y());
			bitangents.push_back(B.z());
		}
	}

	unsigned int addVertex(QVector3D v) {
		unsigned int index = vertices.size();
		v.normalize();
		vertices.push_back(v.x());
		vertices.push_back(v.y());
		vertices.push_back(v.z());
		return index / 3;
	}

	unsigned int getMiddlePoint(unsigned int p1, unsigned int p2) {
		bool firstIsSmaller = p1 < p2;
		long long int smallerIndex = firstIsSmaller ? p1 : p2;
		long long int greaterIndex = firstIsSmaller ? p2 : p1;
		long long int key = (smallerIndex << 32) + greaterIndex;

		unsigned int ret;
		if (middlePointCache.count(key) > 0) {
			ret = middlePointCache.at(key);
			return ret;
		}

		QVector3D a = QVector3D(vertices[p1 * 3], vertices[3 * p1 + 1], vertices[3 * p1 + 2]);
		QVector3D b = QVector3D(vertices[p2 * 3], vertices[3 * p2 + 1], vertices[3 * p2 + 2]);
		QVector3D m = (a + b) / 2.0;

		unsigned int index = addVertex(m);
		middlePointCache[key] = index;
		return index;
	}

	int getPolyCount() { return 12 * iterations * 4; }

	IcoSphere(int i = 3)
	    : iterations(i),
	      ibuf(QOpenGLBuffer(QOpenGLBuffer::IndexBuffer)),
	      specularColor(QVector3D(1.0, 0.7, 0.4)) {
		// std::cerr<<"Init"<<std::endl;

		middlePointCache.clear();

		// init with an icosahedron
		float t = (1.0 + sqrt(5.0) / 2.0);

		addVertex(QVector3D(-1, t, 0));
		addVertex(QVector3D(1, t, 0));
		addVertex(QVector3D(-1, -t, 0));
		addVertex(QVector3D(1, -t, 0));

		addVertex(QVector3D(0, -1, t));
		addVertex(QVector3D(0, 1, t));
		addVertex(QVector3D(0, -1, -t));
		addVertex(QVector3D(0, 1, -t));

		addVertex(QVector3D(t, 0, -1));
		addVertex(QVector3D(t, 0, 1));
		addVertex(QVector3D(-t, 0, -1));
		addVertex(QVector3D(-t, 0, 1));

		indices.push_back(0);
		indices.push_back(11);
		indices.push_back(5);
		indices.push_back(0);
		indices.push_back(5);
		indices.push_back(1);
		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(7);
		indices.push_back(0);
		indices.push_back(7);
		indices.push_back(10);
		indices.push_back(0);
		indices.push_back(10);
		indices.push_back(11);

		indices.push_back(1);
		indices.push_back(5);
		indices.push_back(9);
		indices.push_back(5);
		indices.push_back(11);
		indices.push_back(4);
		indices.push_back(11);
		indices.push_back(10);
		indices.push_back(2);
		indices.push_back(10);
		indices.push_back(7);
		indices.push_back(6);
		indices.push_back(7);
		indices.push_back(1);
		indices.push_back(8);

		indices.push_back(3);
		indices.push_back(9);
		indices.push_back(4);
		indices.push_back(3);
		indices.push_back(4);
		indices.push_back(2);
		indices.push_back(3);
		indices.push_back(2);
		indices.push_back(6);
		indices.push_back(3);
		indices.push_back(6);
		indices.push_back(8);
		indices.push_back(3);
		indices.push_back(8);
		indices.push_back(9);

		indices.push_back(4);
		indices.push_back(9);
		indices.push_back(5);
		indices.push_back(2);
		indices.push_back(4);
		indices.push_back(11);
		indices.push_back(6);
		indices.push_back(2);
		indices.push_back(10);
		indices.push_back(8);
		indices.push_back(6);
		indices.push_back(7);
		indices.push_back(9);
		indices.push_back(8);
		indices.push_back(1);

		// refine triangles
		for (int j = 0; j < iterations; j++) {
			std::vector<int> indicesTmp;
			indicesTmp.clear();
			for (unsigned int k = 0; k < indices.size(); k += 3) {
				// replace triangle by 4 triangles
				unsigned int a = getMiddlePoint(indices[k + 0], indices[k + 1]);
				unsigned int b = getMiddlePoint(indices[k + 1], indices[k + 2]);
				unsigned int c = getMiddlePoint(indices[k + 2], indices[k + 0]);
				indicesTmp.push_back(indices[k + 0]);
				indicesTmp.push_back(a);
				indicesTmp.push_back(c);
				indicesTmp.push_back(indices[k + 1]);
				indicesTmp.push_back(b);
				indicesTmp.push_back(a);
				indicesTmp.push_back(indices[k + 2]);
				indicesTmp.push_back(c);
				indicesTmp.push_back(b);
				indicesTmp.push_back(a);
				indicesTmp.push_back(b);
				indicesTmp.push_back(c);
			}
			indices.clear();
			for (unsigned int k = 0; k < indicesTmp.size(); k++) {
				indices.push_back(indicesTmp[k]);
			}
		}

		// normals
		for (unsigned int j = 0; j < vertices.size(); j++) {
			normals.push_back(vertices[j]);
		}

		// UV mapping
		for (unsigned int j = 0; j < vertices.size(); j += 3) {
			QVector3D d(vertices[j], vertices[j + 1], vertices[j + 2]);
			float u = 0.5 - atan2(d.z(), d.x()) / (2 * M_PI);
			float v = 0.5 - asin(d.y()) / M_PI;
			texCoords.push_back(u);
			texCoords.push_back(v);
		}

		// tangents & bitangents
		createTangents();
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

		tanbuf.create();
		tanbuf.bind();
		tanbuf.allocate(&tangents[0], tangents.size() * sizeof(float));
		shader.enableAttributeArray("tangent");
		shader.setAttributeBuffer("tangent", GL_FLOAT, 0, 3);

		bitanbuf.create();
		bitanbuf.bind();
		bitanbuf.allocate(&bitangents[0], bitangents.size() * sizeof(float));
		shader.enableAttributeArray("bitangent");
		shader.setAttributeBuffer("bitangent", GL_FLOAT, 0, 3);

		tbuf.create();
		tbuf.bind();
		tbuf.allocate(&texCoords[0], texCoords.size() * sizeof(float));
		shader.enableAttributeArray("texCoord");
		shader.setAttributeBuffer("texCoord", GL_FLOAT, 0, 2);

		ibuf.create();
		ibuf.bind();
		ibuf.allocate(&indices[0], indices.size() * sizeof(int));
		vao.release();
		shader.release();
	}
};

#endif
