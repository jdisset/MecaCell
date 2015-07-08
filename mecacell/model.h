#ifndef MECACELL_MODEL_H
#define MECACELL_MODEL_H
#include "matrix4x4.h"
#include "objmodel.h"
#include "tools.h"
#include <vector>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

using std::string;
using std::vector;
using std::unordered_map;
using std::unordered_set;

namespace MecaCell {

struct Model;
struct ModelFace {
	ModelFace(Triangle T, Model *M) : t(T), m(M) {}
	Triangle t;
	Model *m;
};
struct Model {
	Model(const string &filepath);

	void scale(const Vec &s);
	void translate(const Vec &t);
	void rotate(const Rotation<Vec> &r);
	void updateFromTransformation();
	void computeAdjacency();
	void updateFacesFromObj();

	ObjModel obj;
	Matrix4x4 transformation;
	vector<Vec> vertices;
	vector<Vec> normals;
	vector<ModelFace> faces;
	unordered_map<size_t, unordered_set<size_t>> adjacency; // adjacent faces share at least one vertex
};

struct ModelConnectionPoint {
	Vec position;
	size_t face;
};
}
#endif
