#ifndef MECACELL_3DOBJ_HPP
#define MECACELL_3DOBJ_HPP
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include "../geometry/geometry.hpp"
#include "../geometry/matrix4x4.h"
#include "utils.hpp"

namespace MecaCell {

/**
 * @brief 3D object representation: contains the vertices, UV coordinates, normals and
 * faces
 */
struct Obj3D {
	enum TriangleData { V, VT, VN };
	using UV = std::array<double, 2>;
	using triangle_t = std::array<size_t, 3>;

	std::vector<Vector3D> vertices;
	std::vector<UV> uv;
	std::vector<Vector3D> normals;
	std::vector<std::array<triangle_t, 3>> faces;
};

/**
 * @brief Class allowing to load a 3D scene from an obj file. It contains one or several
 * Obj3D as well as various helper method
 */
struct Scene3D {
	using triangle_t = Obj3D::triangle_t;
	using UV = Obj3D::UV;

	Matrix4x4 transformation;
	std::unordered_map<std::string, Obj3D> originalObjects;
	std::unordered_map<std::string, Obj3D> transformedObjects;

	/**
	 * @brief usefull to test if a point v is inside an object of the scene
	 *
	 * @param v the considered point
	 *
	 * @return a vector of container objects names, ordered by distance (closest englobing
	 * first)
	 */
	std::vector<std::string> isInside(const Vector3D &v) const {
		std::vector<std::pair<std::string, double>> containers;
		for (auto &o : transformedObjects) {
			std::pair<std::pair<size_t, double>, std::pair<size_t, double>> hits{
			    {0, std::numeric_limits<double>::max()},
			    {0, std::numeric_limits<double>::max()}};
			auto &obj = o.second;

			// nb of hits with the closest squared distance in one direction and its opposite.
			// If both are uneven, the point v is inside the object. The object with the closest
			// pair of hits is the closest containing one.

			for (auto &f : obj.faces) {
				auto raycast = rayInTriangle(obj.vertices[f[Obj3D::TriangleData::V][0]],
				                             obj.vertices[f[Obj3D::TriangleData::V][1]],
				                             obj.vertices[f[Obj3D::TriangleData::V][2]], v,
				                             Vector3D(0, 0, 1));
				if (raycast.first) {
					++hits.first.first;
					auto sqdist = (v - raycast.second).sqlength();
					if (sqdist < hits.first.second) {
						hits.first.second = sqdist;
					}
				} else {
					// opposite direction
					raycast = rayInTriangle(obj.vertices[f[Obj3D::TriangleData::V][0]],
					                        obj.vertices[f[Obj3D::TriangleData::V][1]],
					                        obj.vertices[f[Obj3D::TriangleData::V][2]], v,
					                        Vector3D(0, 0, -1));
					if (raycast.first) {
						++hits.second.first;
						auto sqdist = (v - raycast.second).sqlength();
						if (sqdist < hits.second.second) hits.second.second = sqdist;
					}
				}
			}
			if (hits.first.first % 2 == 1 && hits.second.first % 2 == 1) {  // we're inside
				containers.push_back(std::pair<std::string, double>(
				    o.first, hits.first.second + hits.second.second));
			}
		}
		std::sort(containers.begin(), containers.end(),
		          [](const auto &a, const auto &b) { return a.second < b.second; });
		std::vector<std::string> result;
		for (auto &c : containers) result.push_back(c.first);
		return result;
	}

	void updateFromTransformation() {
		for (auto &oo : originalObjects) {
			if (!transformedObjects.count(oo.first)) transformedObjects[oo.first] = oo.second;
			Obj3D &to = transformedObjects[oo.first];
			for (size_t i = 0; i < to.vertices.size(); ++i)
				to.vertices[i] = transformation * oo.second.vertices[i];
			for (size_t i = 0; i < to.normals.size(); ++i)
				to.normals[i] = (transformation * oo.second.normals[i]).normalized();
		}
	}

	void scale(const Vec &s) {
		transformation.scale(s);
		updateFromTransformation();
	}

	void translate(const Vec &t) {
		transformation.translate(t);
		updateFromTransformation();
	}

	void rotate(const Rotation<Vec> &r) {
		transformation.rotate(r);
		updateFromTransformation();
	}

	Scene3D(){};
	Scene3D(const string &filename) { load(filename); }
	/**
	 * @brief Loads an obj (wavefront) file
	 *
	 * @param filepath the path to the obj file
	 */
	void load(const string &filepath) {
		std::ifstream file(filepath);
		if (!file.is_open()) {
			throw std::runtime_error("Unable to open scene3D file");
		}

		string line;
		Obj3D *currentObjPtr = nullptr;
		std::string currentObj = "";
		size_t currentObjVertexId = 0, currentObjUVId = 0, currentObjNormalId = 0;
		while (std::getline(file, line)) {
			vector<string> vs = splitStr(line, ' ');
			if (vs.size() > 1) {
				if (vs[0] == "o") {
					// new object
					if (currentObjPtr) {
						currentObjVertexId += currentObjPtr->vertices.size();
						currentObjUVId += currentObjPtr->uv.size();
						currentObjNormalId += currentObjPtr->normals.size();
					}
					currentObj = vs[1];
					originalObjects.insert({{currentObj, Obj3D()}});
					currentObjPtr = &originalObjects[currentObj];

				} else {
					if (currentObjPtr) {
						if (vs[0] == "v" && vs.size() > 3) {  // vertex coordinates
							currentObjPtr->vertices.push_back(
							    Vec(stod(vs[1]), stod(vs[2]), stod(vs[3])));
						} else if (vs[0] == "vt" && vs.size() > 2) {  // UV coordinates
							currentObjPtr->uv.push_back({{stod(vs[1]), stod(vs[2])}});
						} else if (vs[0] == "vn" && vs.size() > 3) {  // normal coordinates
							currentObjPtr->normals.push_back(
							    Vec(stod(vs[1]), stod(vs[2]), stod(vs[3])));
						} else if (vs[0] == "f" &&
						           vs.size() == 4) {  // face (only triangles allowed for now)
							std::array<triangle_t, 3> face;
							for (size_t i = 1; i < vs.size(); ++i) {
								std::vector<std::string> index = splitStr(vs[i], '/');
								assert(index.size() == 3);
								auto vid = stoul(index[0]) - 1 - currentObjVertexId;
								assert(vid < currentObjPtr->vertices.size() && vid >= 0);
								face[Obj3D::TriangleData::V][i - 1] = vid;
								if (!index[1].empty()) {
									auto uvid = stoul(index[1]) - 1 - currentObjUVId;
									assert(uvid < currentObjPtr->uv.size() && uvid >= 0);
									face[Obj3D::TriangleData::VT][i - 1] = uvid;
								}
								auto nid = stoul(index[2]) - 1 - currentObjNormalId;
								assert(nid < currentObjPtr->normals.size() && nid >= 0);
								face[Obj3D::TriangleData::VN][i - 1] = stoul(index[2]) - 1;
							}
							currentObjPtr->faces.push_back(face);
						}
					}
				}
			}
		}
		updateFromTransformation();
	}
};
}

#endif
