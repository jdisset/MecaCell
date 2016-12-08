#ifndef DEFSPHERE
#define DEFSPHERE
#include <QVector3D>
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "../utilities/viewtools.h"

using namespace std;

namespace MecacellViewer {
struct DeformableSphere {
	QOpenGLVertexArrayObject vao;
	QOpenGLBuffer vbuf, cbuf, nbuf, tanbuf, tbuf, bitanbuf, ibuf;

	vector<QVector3D> vert;
	vector<QVector4D> col;
	vector<QVector3D> faceNormals;
	vector<float> vertices;
	vector<float> colors;
	vector<float> normals;
	vector<float> tangents;
	vector<float> bitangents;
	vector<int> indices;
	vector<float> texCoords;
	unordered_map<size_t, vector<size_t>> vertVertAdjacency;
	unordered_map<size_t, vector<size_t>> vertFaceAdjacency;

	vector<int> unitSphereTriangulation(const vector<QVector3D>& points) {
		using Face = std::array<size_t, 3>;
		vector<Face> faces;
		cerr << "points.size = " << points.size() << endl;
		if (points.size() < 6) return vector<int>();
		// first we need a tetrahedron
		// looking at the farthest points on each axis
		pair<size_t, size_t> X, Y, Z;
		X = {0, 1};
		Y = {0, 1};
		Z = {0, 1};
		for (size_t i = 0; i < points.size(); ++i) {
			if (points[i].x() < points[X.first].x()) X.first = i;
			if (points[i].x() > points[X.second].x()) X.second = i;
			if (points[i].y() < points[Y.first].y()) Y.first = i;
			if (points[i].y() > points[Y.second].y()) Y.second = i;
			if (points[i].z() < points[Z.first].z()) Z.first = i;
			if (points[i].z() > points[Z.second].z()) Z.second = i;
		}
		// qDebug() << "X = " << X.first << "," << X.second << "; Y = " << Y.first << ","
		//<< Y.second << "; Z = " << Z.first << "," << Z.second;

		faces = {{Y.second, X.first, Z.first},   {Y.second, Z.first, X.second},
		         {Y.second, X.second, Z.second}, {Y.second, Z.second, X.first},
		         {Y.first, Z.first, X.first},    {Y.first, X.second, Z.first},
		         {Y.first, Z.second, X.second},  {Y.first, X.first, Z.second}};
		for (size_t i = 0; i < points.size(); ++i) {
			if (i != X.first && i != X.second && i != Y.first && i != Y.second &&
			    i != Z.first && i != Z.second) {
				auto& p = points[i];
				vector<size_t> visible;    // list of faces p can "see"
				vector<size_t> invisible;  // list of faces p can't "see"
				for (size_t fid = 0; fid < faces.size(); ++fid) {
					auto& f = faces[fid];
					QVector3D normal = QVector3D::crossProduct(points[f[0]] - points[f[1]],
					                                           points[f[2]] - points[f[1]]);
					if (QVector3D::dotProduct(normal, points[f[1]]) < 0) {
						// wrong direction...
						normal = -normal;
					}
					double dotP = QVector3D::dotProduct(normal, p - points[f[1]]);
					if (dotP > 0) {
						visible.push_back(fid);
					} else {
						invisible.push_back(fid);
					}
				}
				// now we need to find the horizon edges (fronteer btwn visible & not visible
				// faces)
				for (auto& inv : invisible) {
					for (auto& vis : visible) {
						// do they have a comon edge ?
						vector<size_t> commonVertices;
						for (auto& p0 : faces[vis]) {
							for (auto& p1 : faces[inv]) {
								if (p1 == p0) {
									commonVertices.push_back(p0);
								}
							}
						}
						if (commonVertices.size() == 2) {
							// we have a horizon edge
							Face nf = {{commonVertices[0], commonVertices[1], i}};
							faces.push_back(nf);
							// cerr << "face created  " << nf[0] << "," << nf[1] << "," << nf[2] <<
							// endl;
						}
					}
				}
				// now we can delete the previously visible faces;
				vector<Face> tmpFaces;
				for (size_t index = 0; index < faces.size(); ++index) {
					bool ok = true;
					for (auto& v : visible) {
						if (index == v) {
							ok = false;
							break;
						}
					}
					if (ok) tmpFaces.push_back(faces[index]);
				}
				faces = tmpFaces;
			}
		}
		vector<int> result;
		for (auto& f : faces) {
			if (QVector3D::dotProduct(
			        points[f[0]], QVector3D::crossProduct(points[f[0]] - points[f[1]],
			                                              points[f[2]] - points[f[1]])) > 0) {
				result.push_back(f[0]);
				result.push_back(f[1]);
				result.push_back(f[2]);
			} else {
				result.push_back(f[2]);
				result.push_back(f[1]);
				result.push_back(f[0]);
			}
		}

		return result;
	}

	DeformableSphere(){};
	DeformableSphere(size_t n)
	    : ibuf(QOpenGLBuffer(QOpenGLBuffer::IndexBuffer)),
	      col(n, QVector4D(1.0, 1.0, 1.0, 1.0)) {
		vert = getSpherePointsPacking(n);
		for (auto& v : vert) {
			v.normalize();
		}
		indices = unitSphereTriangulation(vert);

		for (size_t i = 0; i < indices.size(); i += 3) {
			vertVertAdjacency[indices[i]].push_back(indices[i + 1]);
			vertVertAdjacency[indices[i]].push_back(indices[i + 2]);
			vertVertAdjacency[indices[i + 1]].push_back(indices[i]);
			vertVertAdjacency[indices[i + 1]].push_back(indices[i + 2]);
			vertVertAdjacency[indices[i + 2]].push_back(indices[i]);
			vertVertAdjacency[indices[i + 2]].push_back(indices[i + 1]);
			vertFaceAdjacency[indices[i + 0]].push_back(i / 3);
			vertFaceAdjacency[indices[i + 1]].push_back(i / 3);
			vertFaceAdjacency[indices[i + 2]].push_back(i / 3);
		}
		for (auto& pvec : vertVertAdjacency) {
			auto& vec = pvec.second;
			sort(vec.begin(), vec.end());
			vec.erase(unique(vec.begin(), vec.end()), vec.end());
		}
		for (auto& pvec : vertFaceAdjacency) {
			auto& vec = pvec.second;
			sort(vec.begin(), vec.end());
			vec.erase(unique(vec.begin(), vec.end()), vec.end());
		}

		for (auto& c : col) {
			colors.push_back(c.x());
			colors.push_back(c.y());
			colors.push_back(c.z());
			colors.push_back(c.w());
		}
		for (auto& v : vert) {
			vertices.push_back(v.x());
			vertices.push_back(v.y());
			vertices.push_back(v.z());

			// uv mapping
			float U = 0.5 - atan2(v.z(), v.x()) / (2.0 * M_PI);
			float V = 0.5 - asin(v.y()) / M_PI;
			texCoords.push_back(U);
			texCoords.push_back(V);

			// normals
			normals.push_back(v.x());
			normals.push_back(v.y());
			normals.push_back(v.z());

			// tangents
			QVector3D T = QVector3D::crossProduct(v, QVector3D(0, 1.0, 0));
			QVector3D B = QVector3D::crossProduct(v, T);
			tangents.push_back(T.x());
			tangents.push_back(T.y());
			tangents.push_back(T.z());
			bitangents.push_back(B.x());
			bitangents.push_back(B.y());
			bitangents.push_back(B.z());
		}
	}

	void update(const vector<QVector3D>& nv, const vector<QVector4D>& nc,
	            QOpenGLShaderProgram& shader) {
		assert(nv.size() == vert.size());
		assert(nc.size() == col.size());
		vertices.clear();
		colors.clear();
		normals.clear();
		tangents.clear();
		bitangents.clear();
		for (auto& v : nv) {
			vertices.push_back(v.x());
			vertices.push_back(v.y());
			vertices.push_back(v.z());
		}
		for (auto& c : nc) {
			// qDebug() << "c = " << c;
			colors.push_back(c.x());
			colors.push_back(c.y());
			colors.push_back(c.z());
			colors.push_back(c.w());
		}
		// normals
		// first we compute face normals
		faceNormals.clear();
		for (size_t i = 0; i < indices.size(); i += 3) {
			size_t faceId = i / 3;
			faceNormals.push_back(QVector3D::crossProduct(nv[indices[i]] - nv[indices[i + 1]],
			                                              nv[indices[i]] - nv[indices[i + 2]])
			                          .normalized());
		}
		// then we avg them for each vertex
		for (size_t i = 0; i < nv.size(); ++i) {
			QVector3D avg(0, 0, 0);
			double n = 0;
			for (auto& fid : vertFaceAdjacency.at(i)) {
				avg += faceNormals[fid];
				++n;
			}
			if (n == 0)
				avg = nv[i];
			else {
				avg /= n;
				avg.normalize();
			}
			normals.push_back(avg.x());
			normals.push_back(avg.y());
			normals.push_back(avg.z());
		}
		vao.bind();
		vbuf.bind();
		vbuf.allocate(&vertices[0], vertices.size() * sizeof(float));
		cbuf.bind();
		cbuf.allocate(&colors[0], colors.size() * sizeof(float));
		nbuf.bind();
		nbuf.allocate(&normals[0], normals.size() * sizeof(float));
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

		cbuf.create();
		cbuf.bind();
		cbuf.setUsagePattern(QOpenGLBuffer::StreamDraw);
		cbuf.allocate(&colors[0], colors.size() * sizeof(float));
		shader.enableAttributeArray("color");
		shader.setAttributeBuffer("color", GL_FLOAT, 0, 4);

		nbuf.create();
		nbuf.bind();
		nbuf.setUsagePattern(QOpenGLBuffer::StreamDraw);
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
}
#endif
