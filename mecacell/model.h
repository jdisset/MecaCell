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
	unordered_map<size_t, unordered_set<size_t>> adjacency; // adjacent faces share at least one vertex
	bool changed = true;
};

struct ModelConnectionPoint {
	ModelConnectionPoint(Model *m, Vec p, size_t f) : model(m), position(p), face(f) {}
	Model *model;
	Vec position;
	size_t face;
	Vec getPosition() { return position; }
	Vec getVelocity() { return Vec::zero(); }
	Vec getAngularVelocity() { return Vec::zero(); }
	Basis<Vec> getOrientation() { return Basis<Vec>(); }
	Rotation<Vec> getOrientationRotation() { return Rotation<Vec>(); }
	double getInertia() { return 1; }
	void receiveForce(double, const Vec &, bool) {}
	void receiveTorque(const Vec &) {}
};
}
#endif
