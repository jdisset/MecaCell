#ifndef MECACELL_QUATERNION_H
#define MECACELL_QUATERNION_H
#include <cmath>
#include "../utilities/utils.hpp"
#include "vector3D.h"

namespace MecaCell {
struct Quaternion {
 public:
	Vector3D v;
	double w;
	Quaternion(const double&, const Vector3D&);
	Quaternion(const Vector3D&, const Vector3D&);
	Quaternion(const Quaternion& q) : v(q.v), w(q.w) {}
	Quaternion(const double& x, const double& y, const double& z, const double& ww)
	    : v(x, y, z), w(ww) {}
	Quaternion() : v(0, 1, 0), w(0) {}
	Quaternion operator*(const Quaternion&)const;
	Vector3D operator*(const Vector3D&)const;
	Vector3D getAxis() const;
	double getAngle() const;
	Quaternion normalized() const;
	void normalize();
	Rotation<Vector3D> toAxisAngle();
};
}
#endif
