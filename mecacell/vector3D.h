#ifndef VECTOR3D_H
#define VECTOR3D_H

#include "types.h"
#include <cmath>
#include <functional>
#include <iostream>
#include "rotation.h"
#include <array>
#include "basis.h"

namespace MecaCell {
class Vector3D {
 public:
	std::array<float_t, 3> coords;

	static const int dimension = 3;
	inline Vector3D(float_t a, float_t b, float_t c) : coords{{a, b, c}} {}
	Vector3D() : coords{{0, 0, 0}} {}
	inline explicit Vector3D(float_t a) : coords{{a, a, a}} {}
	inline explicit Vector3D(std::array<float_t, 3> c) : coords(c) {}

	inline float_t dot(const Vector3D &v) const {
		return coords[0] * v.coords[0] + coords[1] * v.coords[1] + coords[2] * v.coords[2];
	}

	inline const Vector3D cross(const Vector3D &v) const {
		return Vector3D(coords[1] * v.coords[2] - coords[2] * v.coords[1],
		                coords[2] * v.coords[0] - coords[0] * v.coords[2],
		                coords[0] * v.coords[1] - coords[1] * v.coords[0]);
	}

	inline float_t &xRef() { return coords[0]; }
	inline float_t &yRef() { return coords[1]; }
	inline float_t &zRef() { return coords[2]; }
	inline float_t x() const { return coords[0]; }
	inline float_t y() const { return coords[1]; }
	inline float_t z() const { return coords[2]; }
	inline void setX(float_t f) { coords[0] = f; }
	inline void setY(float_t f) { coords[1] = f; }
	inline void setZ(float_t f) { coords[2] = f; }

	void random();
	Vector3D deltaDirection(float_t amount);
	static Vector3D randomUnit();
	static Vector3D zero();
	bool isZero() const;

	inline Vector3D &operator*=(const float_t d) {
		coords[0] *= d;
		coords[1] *= d;
		coords[2] *= d;
		return *this;
	};

	inline Vector3D &operator/=(const float_t d) {
		coords[0] /= d;
		coords[1] /= d;
		coords[2] /= d;
		return *this;
	};

	inline Vector3D &operator+=(const Vector3D &v) {
		coords[0] += v.coords[0];
		coords[1] += v.coords[1];
		coords[2] += v.coords[2];
		return *this;
	}

	inline Vector3D &operator-=(const Vector3D &v) {
		coords[0] -= v.coords[0];
		coords[1] -= v.coords[1];
		coords[2] -= v.coords[2];
		return *this;
	}

	friend inline bool operator==(const Vector3D &v1, const Vector3D &v2);
	friend inline bool operator!=(const Vector3D &v1, const Vector3D &v2);
	friend inline const Vector3D operator+(const Vector3D &v1, const Vector3D &v2);
	friend inline const Vector3D operator+(const Vector3D &v1, float_t f);
	friend inline const Vector3D operator+(float_t f, const Vector3D &v1);
	friend inline const Vector3D operator-(const Vector3D &v1, const Vector3D &v2);
	friend inline const Vector3D operator-(const Vector3D &v1, float_t f);
	friend inline const Vector3D operator-(float_t f, const Vector3D &v1);
	friend inline const Vector3D operator*(float_t factor, const Vector3D &vector);
	friend inline const Vector3D operator*(const Vector3D &vector, float_t factor);
	friend inline const Vector3D operator*(const Vector3D &v1, const Vector3D &v2);
	friend inline const Vector3D operator-(const Vector3D &vector);
	friend inline const Vector3D operator/(const Vector3D &vector, float_t divisor);
	friend inline ostream &operator<<(ostream &out, const Vector3D &v);

	float_t length() const;
	float_t sqlength() const;

	Vector3D rotated(const float_t &, const Vector3D &) const;
	Vector3D rotated(const Rotation<Vector3D> &) const;
	static void addAsAngularVelocity(const Vector3D &, Rotation<Vector3D> &);
	static Rotation<Vector3D> getRotation(const Vector3D &, const Vector3D &);
	static Rotation<Vector3D> rotateRotation(const Rotation<Vector3D> &,
	                                         const Rotation<Vector3D> &);
	static Rotation<Vector3D> addRotations(const Rotation<Vector3D> &,
	                                       const Rotation<Vector3D> &);
	static Rotation<Vector3D> getRotation(const Vector3D &, const Vector3D &,
	                                      const Vector3D &, const Vector3D &);
	static Rotation<Vector3D> getRotation(const Basis<Vector3D> &, const Basis<Vector3D> &);
	static Vector3D getProjection(const Vector3D &origin, const Vector3D &A,
	                              const Vector3D &B);
	static Vector3D getProjectionOnPlane(const Vector3D &o, const Vector3D &n,
	                                     const Vector3D &p);
	static float_t rayCast(const Vector3D &o, const Vector3D &n, const Vector3D &p,
	                       const Vector3D &r);

	void normalize();
	inline const Vector3D normalized() const { return *this / length(); }

	std::string toString();
	static int getHash(int a, int b);
	std::size_t getHash() const;
	inline void iterateTo(Vector3D const &v,
	                      const std::function<void(const Vector3D &)> &fun,
	                      float_t inc = 1) {
		Vector3D base(floor(min(coords[0], v.coords[0])), floor(min(coords[1], v.coords[1])),
		              floor(min(coords[2], v.coords[2])));
		Vector3D nxt(ceil(max(coords[0], v.coords[0])), ceil(max(coords[1], v.coords[1])),
		             ceil(max(coords[2], v.coords[2])));
		for (auto i = base.x(); i <= nxt.x(); i += inc) {
			for (auto j = base.y(); j <= nxt.y(); j += inc) {
				for (auto k = base.z(); k <= nxt.z(); k += inc) {
					fun(Vector3D(i, j, k));
				}
			}
		}
	}

	Vector3D ortho() const;
	Vector3D ortho(Vector3D v) const;
};

inline bool operator==(const Vector3D &v1, const Vector3D &v2) {
	return v1.coords[0] == v2.coords[0] && v1.coords[1] == v2.coords[1] &&
	       v1.coords[2] == v2.coords[2];
}
inline bool operator!=(const Vector3D &v1, const Vector3D &v2) {
	return v1.coords[0] != v2.coords[0] && v1.coords[1] != v2.coords[1] &&
	       v1.coords[2] != v2.coords[2];
}

inline const Vector3D operator+(const Vector3D &v1, const Vector3D &v2) {
	return Vector3D(v1.coords[0] + v2.coords[0], v1.coords[1] + v2.coords[1],
	                v1.coords[2] + v2.coords[2]);
}
inline const Vector3D operator+(float_t f, const Vector3D &v) {
	return Vector3D(v.coords[0] + f, v.coords[1] + f, v.coords[2] + f);
}
inline const Vector3D operator+(const Vector3D &v, float_t f) {
	return Vector3D(v.coords[0] + f, v.coords[1] + f, v.coords[2] + f);
}

inline const Vector3D operator-(const Vector3D &v1, const Vector3D &v2) {
	return Vector3D(v1.coords[0] - v2.coords[0], v1.coords[1] - v2.coords[1],
	                v1.coords[2] - v2.coords[2]);
}
inline const Vector3D operator*(const Vector3D &v1, const Vector3D &v2) {
	return Vector3D(v1.coords[0] * v2.coords[0], v1.coords[1] * v2.coords[1],
	                v1.coords[2] * v2.coords[2]);
}

inline const Vector3D operator*(float_t f, const Vector3D &v) {
	return Vector3D(v.coords[0] * f, v.coords[1] * f, v.coords[2] * f);
}
inline const Vector3D operator*(const Vector3D &v, float_t f) {
	return Vector3D(v.coords[0] * f, v.coords[1] * f, v.coords[2] * f);
}

inline const Vector3D operator-(float_t f, const Vector3D &v) {
	return Vector3D(v.coords[0] - f, v.coords[1] - f, v.coords[2] - f);
}
inline const Vector3D operator-(const Vector3D &v, float_t f) {
	return Vector3D(v.coords[0] - f, v.coords[1] - f, v.coords[2] - f);
}

inline const Vector3D operator-(const Vector3D &v) {
	return Vector3D(-v.coords[0], -v.coords[1], -v.coords[2]);
}

inline const Vector3D operator/(const Vector3D &v, float_t f) {
	return Vector3D(v.coords[0] / f, v.coords[1] / f, v.coords[2] / f);
}
inline ostream &operator<<(ostream &out, const Vector3D &v) {
	out << "(" << v.coords[0] << ", " << v.coords[1] << ", " << v.coords[2] << ")";
	return out;
}
}
namespace std {
template <> struct hash<MecaCell::Vector3D> {
	std::size_t operator()(const MecaCell::Vector3D &v) const { return v.getHash(); }
};
}
#endif  // VECTOR3D_H
