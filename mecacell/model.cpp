#include "model.h"

using std::string;
using std::vector;
using std::unordered_map;
using std::unordered_set;

namespace MecaCell {
Model::Model(const string &filepath) : obj(filepath) {
	updateFacesFromObj();
	// computeAdjacency();
	updateFromTransformation();
	logger<INF>("added model");
}

void Model::scale(const Vec &s) {
	transformation.scale(s);
	updateFromTransformation();
}
void Model::translate(const Vec &t) {
	transformation.translate(t);
	updateFromTransformation();
}
void Model::rotate(const Rotation<Vec> &r) {
	transformation.rotate(r);
	updateFromTransformation();
}

bool Model::changedSinceLastCheck() {
	bool c = changed;
	changed = false;
	return c;
}

void Model::updateFromTransformation() {
	vertices.clear();
	normals.clear();
	for (auto &v : obj.vertices) {
		vertices.push_back(transformation * v);
	}
	for (auto &n : obj.normals) {
		normals.push_back((transformation * n).normalized());
	}
	changed = true;
}

void Model::updateFacesFromObj() {
	for (auto &f : obj.faces) {
		faces.push_back(f.at("v"));
	}
	changed = true;
}

void Model::computeAdjacency() {
	for (size_t i = 0; i < faces.size(); ++i) {
		Triangle &ti = faces[i];
		for (size_t j = i + 1; j < faces.size(); ++j) {
			Triangle &tj = faces[j];
			if (ti.indices[0] == tj.indices[0] || ti.indices[0] == tj.indices[1] ||
			    ti.indices[0] == tj.indices[2] || ti.indices[1] == tj.indices[0] ||
			    ti.indices[1] == tj.indices[1] || ti.indices[1] == tj.indices[2] ||
			    ti.indices[2] == tj.indices[0] || ti.indices[2] == tj.indices[1] ||
			    ti.indices[2] == tj.indices[2]) {
				adjacency[i].insert(j);
				adjacency[j].insert(i);
			}
		}
	}
}
}
