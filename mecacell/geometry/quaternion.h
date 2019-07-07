#ifndef MECACELL_QUATERNION_H
#define MECACELL_QUATERNION_H
#include <cmath>
#include "../utilities/utils.hpp"

#define dispVec(v) "(" << v.x() << "," << v.y() << "," << v.z() << ")"

namespace MecaCell {
template <typename V> struct Quaternion {
	V v;
	num_t w;
	Quaternion(const num_t& angle, const V& n) {
		num_t halfangle = angle * 0.5;
		w = cos(halfangle);
		v = n * sin(halfangle);
	}
	Quaternion(const V& v0, const V& v1) {
		V v2 = v0.normalized();
		V v3 = v1.normalized();
		num_t sc = min<num_t>(1.0, max<num_t>(-1.0, v2.dot(v3)));
		if (sc < -0.9999) {
			*this = Quaternion(M_PI, v2.ortho());
		} else {
			v = v2.cross(v3);
			w = 1.0 + sc;
			normalize();
		}
	}
	Quaternion(const Quaternion& q) : v(q.v), w(q.w) {}
	Quaternion(const num_t& x, const num_t& y, const num_t& z, const num_t& ww)
	    : v(x, y, z), w(ww) {}
	Quaternion() : v(0, 1, 0), w(0) {}
	Quaternion operator*(const Quaternion& q2) const {
		return Quaternion(v.x() * q2.w + v.y() * q2.v.z() - v.z() * q2.v.y() + w * q2.v.x(),
		                  -v.x() * q2.v.z() + v.y() * q2.w + v.z() * q2.v.x() + w * q2.v.y(),
		                  v.x() * q2.v.y() - v.y() * q2.v.x() + v.z() * q2.w + w * q2.v.z(),
		                  -v.x() * q2.v.x() - v.y() * q2.v.y() - v.z() * q2.v.z() + w * q2.w);
	}
	V operator*(const V& vec) const {
		V vcV = 2.0 * v.cross(vec);
		return vec + w * vcV + v.cross(vcV);
	}

	V getAxis() const {
		assert(w <= 1.0);
		num_t s = sqrt(1.0 - w * w);
		if (s == 0) return V(1, 0, 0);
		return v / s;
	}
	num_t getAngle() const {
		assert(w <= 1.0);
		return 2.0 * acos(w);
	}

	Quaternion normalized() const {
		num_t magnitude = sqrt(w * w + v.x() * v.x() + v.y() * v.y() + v.z() * v.z());
		return Quaternion(v.x() / magnitude, v.y() / magnitude, v.z() / magnitude,
		                  w / magnitude);
	}
	void normalize() {
		num_t magnitude = sqrt(w * w + v.x() * v.x() + v.y() * v.y() + v.z() * v.z());
		w = min<num_t>(w / magnitude, 1.0);
		v = v / magnitude;
	}

	Rotation<V> toAxisAngle() {
		normalize();
		num_t s = sqrt(1.0 - w * w);
		if (s == 0) return Rotation<V>(V(1, 0, 0), acos(w) * 2.0);
		return Rotation<V>(v / s, acos(w) * 2.0);
	}
};
}  // namespace MecaCell
#endif
