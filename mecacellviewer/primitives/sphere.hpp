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
#include <unordered_set>

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

	IcoSphere(int i = 3) : iterations(i), ibuf(QOpenGLBuffer(QOpenGLBuffer::IndexBuffer)) {
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

		// UV mapping
		for (unsigned int j = 0; j < vertices.size(); j += 3) {
			QVector3D d(vertices[j], vertices[j + 1], vertices[j + 2]);
			float u = 0.5 - atan2(d.z(), d.x()) / (2 * M_PI);
			float v = 0.5 - asin(d.y()) / M_PI;
			texCoords.push_back(u);
			texCoords.push_back(v);
		}

		repairTextureWrapSeam();

		// normals
		for (unsigned int j = 0; j < vertices.size(); j++) {
			normals.push_back(vertices[j]);
		}
		// tangents & bitangents
		createTangents();
	}

	QVector2D getTexCoord(int id) { return QVector2D(texCoords[id * 2 + 0], texCoords[id * 2 + 1]); }
	QVector3D getVertCoord(int id) {
		return QVector3D(vertices[id * 3 + 0], vertices[id * 3 + 1], vertices[id * 3 + 2]);
	}

	void repairWrap() {
		unordered_set<int> corrected;
		const float threshold = 0.5f;
		for (size_t i = 0; i < indices.size() - 3; i += 3) {
			// V0 = 0 -> 1
			// V1 = 0 -> 2
			// V2 = 1 -> 2
			size_t id0 = indices[i + 0];
			size_t id1 = indices[i + 1];
			size_t id2 = indices[i + 2];
			QVector2D v0 = getTexCoord(id0) - getTexCoord(id1);
			QVector2D v1 = getTexCoord(id0) - getTexCoord(id2);
			QVector2D v2 = getTexCoord(id1) - getTexCoord(id2);
			if (v0.x() > threshold || v1.x() > threshold) { // il faut changer 0
				if (!corrected.count(id0)) {
					corrected.insert(id0);
					// on ajoute un nouveau sommet
					// on linke 1 et 2 correctement à ce nouveau sommet
				}
			}
			if (v0.x() < -threshold || v2.x() > threshold) { // il faut changer 1
			}
			if (v1.x() < -threshold || v2.x() < -threshold) { // il faut changer 2
			}
		}
	}

	void repairTextureWrapSeam() {
		vector<int> newIndices;
		int corrections = 0;

		/// whenever a vertex is split, add its original and new indices to the dictionary to avoid
		/// creating duplicates.
		map<int, int> correctionList;

		for (int i = (int)indices.size() - 3; i >= 0; i -= 3) {
			/// see if the texture coordinates appear in counter-clockwise order.
			/// If so, the triangle needs to be rectified.
			QVector3D v0(texCoords[indices[i + 0] * 2 + 0], texCoords[indices[i + 0] * 2 + 1], 0);
			QVector3D v1(texCoords[indices[i + 1] * 2 + 0], texCoords[indices[i + 1] * 2 + 1], 0);
			QVector3D v2(texCoords[indices[i + 2] * 2 + 0], texCoords[indices[i + 2] * 2 + 1], 0);

			QVector3D cross = QVector3D::crossProduct(v0 - v1, v2 - v1);

			if (cross.z() <= 0) {
				/// this should only happen if the face crosses a texture boundary

				bool corrected = false;

				for (int j = i; j < i + 3; j++) {
					int index = indices[j];

					QVector2D vertexTexCoord(texCoords[index * 2], texCoords[index * 2 + 1]);
					if (vertexTexCoord.x() >= 0.4f) {
						/// need to correct this vertex.
						if (correctionList.count(index))
							newIndices.push_back(correctionList[index]);
						else {
							QVector2D texCoordCopy = vertexTexCoord;
							texCoordCopy.setX(texCoordCopy.x() - 1.0);
							vertexTexCoord = texCoordCopy;
							corrected = true;
							texCoords.push_back(vertexTexCoord.x());
							texCoords.push_back(vertexTexCoord.y());
							vertices.push_back(vertices[index * 3 + 0]);
							vertices.push_back(vertices[index * 3 + 1]);
							vertices.push_back(vertices[index * 3 + 2]);
							int correctedVertexIndex = (vertices.size() / 3) - 1;
							correctionList[index] = correctedVertexIndex;
							newIndices.push_back(correctedVertexIndex);
						}
					} else
						newIndices.push_back(index);
				}
				if (corrected) corrections++;
			} else {
				newIndices.push_back(indices[i + 0]);
				newIndices.push_back(indices[i + 1]);
				newIndices.push_back(indices[i + 2]);
			}
		}

		indices = vector<int>();
		indices = newIndices;
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
