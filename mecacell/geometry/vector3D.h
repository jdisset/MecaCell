#ifndef MECACELL_VECTOR3D_H
#define MECACELL_VECTOR3D_H

#include <array>
#include <cmath>
#include <functional>
#include <iostream>
#include "../utilities/utils.h"
#include "basis.h"
#include "rotation.h"

namespace MecaCell {
class Vector3D {
 public:
	std::array<double, 3> coords;

	static const int dimension = 3;
	inline Vector3D(double a, double b, double c) : coords{{a, b, c}} {}
	template <typename T>
	Vector3D(const T &otherV) : coords{{otherV.x(), otherV.y(), otherV.z()}} {}
	inline Vector3D() : coords{{0, 0, 0}} {}
	inline explicit Vector3D(double a) : coords{{a, a, a}} {}
	inline explicit Vector3D(std::array<double, 3> c) : coords(c) {}

	inline Vector3D(const Vector3D &v) : coords(v.coords) {}
	inline Vector3D(Vector3D &&v) : coords(std::move(v.coords)) {}
	Vector3D &operator=(const Vector3D &other) {
		if (&other == this) return *this;
		coords = other.coords;
		return *this;
	}

	inline double dot(const Vector3D &v) const {
		return coords[0] * v.coords[0] + coords[1] * v.coords[1] + coords[2] * v.coords[2];
	}

	inline const Vector3D cross(const Vector3D &v) const {
		return Vector3D(coords[1] * v.coords[2] - coords[2] * v.coords[1],
		                coords[2] * v.coords[0] - coords[0] * v.coords[2],
		                coords[0] * v.coords[1] - coords[1] * v.coords[0]);
	}

	inline double &xRef() { return coords[0]; }
	inline double &yRef() { return coords[1]; }
	inline double &zRef() { return coords[2]; }
	inline double x() const { return coords[0]; }
	inline double y() const { return coords[1]; }
	inline double z() const { return coords[2]; }
	inline void setX(const double f) { coords[0] = f; }
	inline void setY(const double f) { coords[1] = f; }
	inline void setZ(const double f) { coords[2] = f; }

	void random();
	Vector3D deltaDirection(double amount);
	static Vector3D randomUnit();
	static Vector3D zero();
	bool isZero() const;

	inline Vector3D &operator*=(double d) {
		coords[0] *= d;
		coords[1] *= d;
		coords[2] *= d;
		return *this;
	};

	inline Vector3D &operator/=(double d) {
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
	friend inline Vector3D operator+(const Vector3D &v1, const Vector3D &v2);
	friend inline Vector3D operator+(const Vector3D &v1, double f);
	friend inline Vector3D operator+(double f, const Vector3D &v1);
	friend inline Vector3D operator-(const Vector3D &v1, const Vector3D &v2);
	friend inline Vector3D operator-(const Vector3D &v1, double f);
	friend inline Vector3D operator-(double f, const Vector3D &v1);
	friend inline Vector3D operator*(double factor, const Vector3D &vector);
	friend inline Vector3D operator*(const Vector3D &vector, double factor);
	friend inline Vector3D operator*(const Vector3D &v1, const Vector3D &v2);
	friend inline Vector3D operator-(const Vector3D &vector);
	friend inline Vector3D operator/(const Vector3D &vector, double divisor);
	friend inline ostream &operator<<(ostream &out, const Vector3D &v);

	double length() const;
	double sqlength() const;

	Vector3D rotated(double, const Vector3D &) const;
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
	static double rayCast(const Vector3D &o, const Vector3D &n, const Vector3D &p,
	                      const Vector3D &r);

	void normalize();
	inline Vector3D normalized() const {
		double l = length();
		return l > 0 ? *this / l : zero();
	}

	std::string toString() const;
	static int getHash(const int a, const int b);
	int getHash() const;
	inline void iterateTo(const Vector3D &v,
	                      const std::function<void(const Vector3D &)> &fun,
	                      const double inc = 1) const {
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
	Vector3D ortho(const Vector3D &v) const;
};

inline bool operator==(const Vector3D &v1, const Vector3D &v2) {
	return v1.coords[0] == v2.coords[0] && v1.coords[1] == v2.coords[1] &&
	       v1.coords[2] == v2.coords[2];
}
inline bool operator!=(const Vector3D &v1, const Vector3D &v2) {
	return v1.coords[0] != v2.coords[0] && v1.coords[1] != v2.coords[1] &&
	       v1.coords[2] != v2.coords[2];
}

inline Vector3D operator+(const Vector3D &v1, const Vector3D &v2) {
	return Vector3D(v1.coords[0] + v2.coords[0], v1.coords[1] + v2.coords[1],
	                v1.coords[2] + v2.coords[2]);
}
inline Vector3D operator+(const double f, const Vector3D &v) {
	return Vector3D(v.coords[0] + f, v.coords[1] + f, v.coords[2] + f);
}
inline Vector3D operator+(const Vector3D &v, const double f) {
	return Vector3D(v.coords[0] + f, v.coords[1] + f, v.coords[2] + f);
}

inline Vector3D operator-(const Vector3D &v1, const Vector3D &v2) {
	return Vector3D(v1.coords[0] - v2.coords[0], v1.coords[1] - v2.coords[1],
	                v1.coords[2] - v2.coords[2]);
}
inline Vector3D operator*(const Vector3D &v1, const Vector3D &v2) {
	return Vector3D(v1.coords[0] * v2.coords[0], v1.coords[1] * v2.coords[1],
	                v1.coords[2] * v2.coords[2]);
}

inline Vector3D operator*(const double f, const Vector3D &v) {
	return Vector3D(v.coords[0] * f, v.coords[1] * f, v.coords[2] * f);
}
inline Vector3D operator*(const Vector3D &v, const double f) {
	return Vector3D(v.coords[0] * f, v.coords[1] * f, v.coords[2] * f);
}

inline Vector3D operator-(const double f, const Vector3D &v) {
	return Vector3D(v.coords[0] - f, v.coords[1] - f, v.coords[2] - f);
}
inline Vector3D operator-(const Vector3D &v, const double f) {
	return Vector3D(v.coords[0] - f, v.coords[1] - f, v.coords[2] - f);
}

inline Vector3D operator-(const Vector3D &v) {
	return Vector3D(-v.coords[0], -v.coords[1], -v.coords[2]);
}

inline Vector3D operator/(const Vector3D &v, const double f) {
	return Vector3D(v.coords[0] / f, v.coords[1] / f, v.coords[2] / f);
}
inline ostream &operator<<(ostream &out, const Vector3D &v) {
	out << "(" << v.coords[0] << ", " << v.coords[1] << ", " << v.coords[2] << ")";
	return out;
}
}
namespace std {
template <> struct hash<MecaCell::Vector3D> {
	int operator()(const MecaCell::Vector3D &v) const { return v.getHash(); }
};
}
#endif  // VECTOR3D_H
