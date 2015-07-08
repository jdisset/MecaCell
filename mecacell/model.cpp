#include "model.h"

using std::string;
using std::vector;
using std::unordered_map;
using std::unordered_set;

namespace MecaCell {
Model::Model(const string &filepath) : obj(filepath) {
	//updateFacesFromObj();
	//computeAdjacency();
	updateFromTransformation();
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
void Model::updateFromTransformation() {
	vertices.clear();
	normals.clear();
	for (auto &v : obj.vertices) {
		vertices.push_back(transformation * v);
	}
	for (auto &n : obj.normals) {
		normals.push_back((transformation * n).normalized());
	}
}
void Model::updateFacesFromObj() {
	for (auto &f : obj.faces) {
		faces.push_back(ModelFace(f.at("vt"), this));
	}
}
void Model::computeAdjacency() {
	for (size_t i = 0; i < faces.size(); ++i) {
		Triangle &ti = faces[i].t;
		for (size_t j = i + 1; j < faces.size(); ++j) {
			Triangle &tj = faces[j].t;
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
