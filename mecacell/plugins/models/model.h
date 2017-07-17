#ifndef MECACELL_MODEL_H
#define MECACELL_MODEL_H
#include "matrix4x4.h"
#include "objmodel.h"
#include "utils.hpp"
#include <vector>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

using std::string;
using std::vector;
using std::unordered_map;
using std::unordered_set;
using std::pair;

namespace MecaCell {

struct Model;

struct Model {
	Model(const string &filepath);

	void scale(const Vec &s);
	void translate(const Vec &t);
	void rotate(const Rotation<Vec> &r);
	void updateFromTransformation();
	void computeAdjacency();
	void updateFacesFromObj();
	bool changedSinceLastCheck();

	string name;
	ObjModel obj;
	Matrix4x4 transformation;
	vector<Vec> vertices;
	vector<Vec> normals;
	vector<Triangle> faces;
	unordered_map<size_t, unordered_set<size_t>>
	    adjacency;  // adjacent faces share at least one vertex
	bool changed = true;
};
}
namespace std {
template <> struct hash<pair<MecaCell::Model *, size_t>> {
	typedef pair<MecaCell::Model *, size_t> argument_type;
	typedef uintptr_t result_type;

	result_type operator()(const pair<MecaCell::Model *, size_t> &t) const {
		return ((reinterpret_cast<result_type>(t.first) + t.second) *
		        (reinterpret_cast<result_type>(t.first) + t.second + 1) / 2) +
		       t.second;
	}
};
}
#endif
