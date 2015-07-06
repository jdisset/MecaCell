#ifndef MECACELL_MODEL_H
#define MECACELL_MODEL_H
#include "matrix4x4.hpp"
#include "objmodel.h"
#include "tools.h"
#include <vector>
#include <string>
#include <vector>

using std::string;
using std::vector;

namespace MecaCell {
class Model {
public:
	Model(const string &filepath) : obj(filepath) { updateFromTransformation(); }
	//void scale(const Vec &s) {
		//transformation.scale(s);
		//updateFromTransformation();
	//}
	//void translate(const Vec &t) {
		//transformation.translate(s);
		//updateFromTransformation();
	//}
	//void rotate(const Rotation<Vec> &r) {
		//transformation.rotate(r);
		//updateFromTransformation();
	//}
	void updateFromTransformation() {
		vertices.clear();
		normals.clear();
		for (auto &v : obj.vertices) {
			// vertices.push_back(tranformation * v);
		}
		for (auto &n : obj.normals) {
			// vertices.push_back(tranformation * n);
		}
	}
	ObjModel obj;
	Matrix4x4 transformation;
	vector<Vec> vertices;
	vector<Vec> normals;
};
}
#endif
