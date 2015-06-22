#include "quaternion.h"
#define dispVec(v) "(" << v.x << "," << v.y << "," << v.z << ")"

namespace MecaCell {
Quaternion Quaternion::normalized() const {
	double magnitude = sqrt(w * w + v.x * v.x + v.y * v.y + v.z * v.z);
	return Quaternion(v.x / magnitude, v.y / magnitude, v.z / magnitude, w / magnitude);
}

void Quaternion::normalize() {
	double magnitude = sqrt(w * w + v.x * v.x + v.y * v.y + v.z * v.z);
	w = min(w / magnitude, 1.0);
	v = v / magnitude;
}

Quaternion::Quaternion(const double &angle, const Vector3D &n) {
	double halfangle = angle * 0.5;
	w = cos(halfangle);
	v = n * sin(halfangle);
}

Quaternion::Quaternion(const Vector3D &v0, const Vector3D &v1) {
	Vector3D v2 = v0.normalized();
	Vector3D v3 = v1.normalized();
	double sc = min(1.0, max(-1.0, v2.dot(v3)));
	if (sc < -0.9999) {
		*this = Quaternion(M_PI, v2.ortho());
	} else {
		v = v2.cross(v3);
		w = 1.0 + sc;
		normalize();
	}
}

Rotation<Vector3D> Quaternion::toAxisAngle() {
	normalize();
	double s = sqrt(1.0 - w * w);
	if (s == 0) return Rotation<Vector3D>(Vector3D(1, 0, 0), acos(w) * 2.0);
	return Rotation<Vector3D>(v / s, acos(w) * 2.0);
}

double Quaternion::getAngle() const {
	assert(w <= 1.0);
	return 2.0 * acos(w);
}

Vector3D Quaternion::getAxis() const {
	assert(w <= 1.0);
	double s = sqrt(1.0 - w * w);
	if (s == 0) return Vector3D(1, 0, 0);
	return v / s;
}

Vector3D Quaternion::operator*(const Vector3D &V) const {
	Vector3D vcV = 2.0 * v.cross(V);
	return V + w * vcV + v.cross(vcV);
}

Quaternion Quaternion::operator*(const Quaternion &q2) const {
	return Quaternion(v.x * q2.w + v.y * q2.v.z - v.z * q2.v.y + w * q2.v.x,
	                  -v.x * q2.v.z + v.y * q2.w + v.z * q2.v.x + w * q2.v.y,
	                  v.x * q2.v.y - v.y * q2.v.x + v.z * q2.w + w * q2.v.z,
	                  -v.x * q2.v.x - v.y * q2.v.y - v.z * q2.v.z + w * q2.w);
}
}
