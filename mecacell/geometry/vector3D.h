#ifndef MECACELL_VECTOR3D_H
#define MECACELL_VECTOR3D_H

#include <array>
#include <cmath>
#include <functional>
#include <iostream>
#include "../utilities/config.hpp"
#include "../utilities/exportable.hpp"
#include "../utilities/utils.hpp"
#include "basis.hpp"
#include "quaternion.h"
#include "rotation.h"

namespace MecaCell {

/**
 * @brief general purpose 3D vector/point class.
 */
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

	/**
	 * @brief assignment operator for copy constructing a vector
	 *
	 * @param other
	 *
	 * @return
	 */
	Vector3D &operator=(const Vector3D &other) {
		if (&other == this) return *this;
		coords = other.coords;
		return *this;
	}

	/**
	 * @brief dot product calculation
	 *
	 * @param v a Vector3D
	 *
	 * @return the dot product of this vector against v
	 */
	inline double dot(const Vector3D &v) const {
		return coords[0] * v.coords[0] + coords[1] * v.coords[1] + coords[2] * v.coords[2];
	}

	/**
	 * @brief cross product calculation
	 *
	 * @param v a Vector3D
	 *
	 * @return the cross product of this vector against v
	 */
	inline const Vector3D cross(const Vector3D &v) const {
		return Vector3D(coords[1] * v.coords[2] - coords[2] * v.coords[1],
		                coords[2] * v.coords[0] - coords[0] * v.coords[2],
		                coords[0] * v.coords[1] - coords[1] * v.coords[0]);
	}

	/**
	 * @brief
	 *
	 * @return a reference to the x coordinate
	 */
	inline double &xRef() { return coords[0]; }
	/**
	 * @brief
	 *
	 * @return a reference to the y coordinate
	 */
	inline double &yRef() { return coords[1]; }
	/**
	 * @brief
	 *
	 * @return a reference to the z coordinate
	 */
	inline double &zRef() { return coords[2]; }
	/**
	 * @brief
	 *
	 * @return the const value of the x coordinate
	 */
	inline double x() const { return coords[0]; }
	/**
	 * @brief
	 *
	 * @return the const value of the y coordinate
	 */
	inline double y() const { return coords[1]; }
	/**
	 * @brief
	 *
	 * @return the const value of the z coordinate
	 */
	inline double z() const { return coords[2]; }

	/**
	 * @brief setter for x coordinate
	 *
	 * @param f the new value
	 */
	inline void setX(const double f) { coords[0] = f; }
	/**
	 * @brief setter for y coordinate
	 *
	 * @param f the new value
	 */
	inline void setY(const double f) { coords[1] = f; }
	/**
	 * @brief setter for z coordinate
	 *
	 * @param f the new value
	 */
	inline void setZ(const double f) { coords[2] = f; }

	/**
	 * @brief sets the current vector as a random normalized one. Uniform direction
	 * distribution on all directions of a sphere.
	 */
	void random() {
		std::normal_distribution<double> nDist(0.0, 1.0);
		coords = {{nDist(Config::globalRand()), nDist(Config::globalRand()),
		           nDist(Config::globalRand())}};
		normalize();
	}

	/**
	 * @brief creates a random normalized vector. Uniform direction
	 * distribution on all directions of a sphere.
	 *
	 * @return a 3D vector
	 */
	static inline Vector3D randomUnit() {
		Vector3D v;
		v.random();
		return v;
	}

	/**
	 * @brief returns a vector randomly tilted relatively to the original one
	 *
	 * @param amount the width of the normal distribution
	 *
	 * @return a 3D vector
	 */
	Vector3D deltaDirection(double amount) {
		std::normal_distribution<double> nDist(0.0, amount);
		return Vector3D(coords[0] + nDist(Config::globalRand()),
		                coords[1] + nDist(Config::globalRand()),
		                coords[2] + nDist(Config::globalRand()))
		    .normalized();
	}

	/**
	 * @brief constructs a zero vector
	 *
	 * @return
	 */
	static inline Vector3D zero() { return Vector3D(0.0, 0.0, 0.0); }

	/**
	 * @brief returns true if all vector coordinates are equal to zero
	 *
	 * @return
	 */
	inline bool isZero() const {
		return (coords[0] == 0.0 && coords[1] == 0.0 && coords[2] == 0.0);
	}

	/**
	 * @brief scalar multiplication operator
	 *
	 * @param d
	 *
	 * @return
	 */
	inline Vector3D &operator*=(double d) {
		coords[0] *= d;
		coords[1] *= d;
		coords[2] *= d;
		return *this;
	};

	/**
	 * @brief scalar division operator
	 *
	 * @param d
	 *
	 * @return
	 */
	inline Vector3D &operator/=(double d) {
		coords[0] /= d;
		coords[1] /= d;
		coords[2] /= d;
		return *this;
	};

	/**
	 * @brief addition operator
	 *
	 * @param v
	 *
	 * @return
	 */
	inline Vector3D &operator+=(const Vector3D &v) {
		coords[0] += v.coords[0];
		coords[1] += v.coords[1];
		coords[2] += v.coords[2];
		return *this;
	}

	/**
	 * @brief substract operator
	 *
	 * @param v
	 *
	 * @return
	 */
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

	/**
	 * @brief compute the length of the vector
	 *
	 * @return
	 */
	double length() const {
		return sqrt(coords[0] * coords[0] + coords[1] * coords[1] + coords[2] * coords[2]);
	}
	/**
	 * @brief compute the square length of the current vector (faster than length)
	 *
	 * @return
	 */
	double sqlength() const {
		return coords[0] * coords[0] + coords[1] * coords[1] + coords[2] * coords[2];
	}

	/**
	 * @brief gives a rotated copy of the current vector
	 *
	 * @param angle the rotation angle in radians
	 * @param vec the axis of rotation
	 *
	 * @return  a rotated copy of the current vector
	 */
	Vector3D rotated(double angle, const Vector3D &vec) const {
		double halfangle = angle * 0.5;
		Vector3D v = vec * sin(halfangle);
		Vector3D vcV = 2.0 * v.cross(*this);
		return *this + cos(halfangle) * vcV + v.cross(vcV);
	}

	/**
	 * @brief gives a rotated copy of the current vector
	 *
	 * @param r the rotation
	 *
	 * @return a rotated copy of the current vector
	 */
	Vector3D rotated(const Rotation<Vector3D> &r) const {
		double halfangle = r.teta * 0.5;
		Vector3D v = r.n * sin(halfangle);
		Vector3D vcV = 2.0 * v.cross(*this);
		return *this + cos(halfangle) * vcV + v.cross(vcV);
	}

	/**
	 * @brief adds a vector representing an angular Velocity (or any rotation coded on only
	 * one vector) to an existing rotation
	 *
	 * @param v the single vector coded rotation (its length codes for the angle)
	 * @param r the rotation which is going to be modified
	 */
	static void addAsAngularVelocity(const Vector3D &v, Rotation<Vector3D> &r) {
		double dTeta = v.length();
		Vector3D n0(0, 1, 0);
		if (dTeta > 0) {
			n0 = v / dTeta;
		}
		r = addRotations(r, Rotation<Vector3D>(n0, dTeta));
	}

	/**
	 * @brief computes the rotation from one vector to another
	 *
	 * @param v0 first vector
	 * @param v1 second vector
	 *
	 * @return a rotation that can transform v0 into v1
	 */
	static Rotation<Vector3D> getRotation(const Vector3D &v0, const Vector3D &v1) {
		Rotation<Vector3D> res;
		res.teta = acos(min<double>(1.0, max<double>(-1.0, v0.dot(v1))));
		Vector3D cross = v0.cross(v1);
		if (cross.sqlength() == 0) {
			cross = Vector3D(0, 1, 0);
		}
		res.n = cross.normalized();
		return res;
	}

	/**
	 * @brief simple raycasting on a plane
	 *
	 * @param o a point in the plane
	 * @param n normal of the plane
	 * @param p origin of the ray
	 * @param r direction of the ray
	 *
	 * @return l such that p + l.r lies on the plane defined by its normal n and an offset
	 * o l > 0 means that the ray hits the plane, l < 0 means that the racoords[1] does
	 * not face the plane l = 0 means that the ray is parallel to the plane or that p is
	 * on the plane
	 */
	static double rayCast(const Vector3D &o, const Vector3D &n, const Vector3D &p,
	                      const Vector3D &r) {
		double nr = n.dot(r);
		return (nr == 0) ? 0 : n.dot(o - p) / nr;
	}

	/**
	 * @brief project a point on a plane
	 *
	 * @param o a point in the plane
	 * @param n the plane's normal
	 * @param p the point to be projected onto the surface of the plane
	 *
	 * @return the projection of p onto a plane defined by its normal n and an
	 * offset o
	 */
	static Vector3D getProjectionOnPlane(const Vector3D &o, const Vector3D &n,
	                                     const Vector3D &p) {
		return p - (n.dot(p - o) * n);
	}

	/**
	 * @brief rotates the axis of rotation of a rotation
	 *
	 * @param start
	 * @param offset
	 *
	 * @return
	 */
	static Rotation<Vector3D> rotateRotation(const Rotation<Vector3D> &start,
	                                         const Rotation<Vector3D> &offset) {
		return Rotation<Vector3D>(start.n.rotated(offset), start.teta);
	}
	/**
	 * @brief adds two rotations
	 *
	 * @param R0
	 * @param R1
	 *
	 * @return
	 */
	static Rotation<Vector3D> addRotations(const Rotation<Vector3D> &R0,
	                                       const Rotation<Vector3D> &R1) {
		auto q2 = Quaternion<Vector3D>(R1.teta, R1.n) * Quaternion<Vector3D>(R0.teta, R0.n);
		q2.normalize();
		return q2.toAxisAngle();
	}

	/**
	 * @brief computes the rotation transform from basis X0,Y0 to X1,Y1
	 *
	 * @param X0
	 * @param Y0
	 * @param X1
	 * @param Y1
	 *
	 * @return
	 */
	static Rotation<Vector3D> getRotation(const Vector3D &X0, const Vector3D &Y0,
	                                      const Vector3D &X1, const Vector3D &Y1) {
		Quaternion<Vector3D> q0(X0.normalized(), X1.normalized());
		Vector3D Ytmp = q0 * Y0;
		Ytmp.normalize();
		auto qres = Quaternion<Vector3D>(Ytmp, Y1.normalized()) * q0;
		qres.normalize();
		return qres.toAxisAngle();
	}
	/**
	 * @brief computes the rotation transform from basis b0 to b1
	 *
	 * @param X0
	 * @param Y0
	 * @param X1
	 * @param Y1
	 *
	 * @return
	 */
	static Rotation<Vector3D> getRotation(const Basis<Vector3D> &b0,
	                                      const Basis<Vector3D> &b1) {
		return getRotation(b0.X, b0.Y, b1.X, b1.Y);
	}

	/**
	 * @brief computes the orthogonal projection of a point onto a segment AB
	 *
	 * @param A the origin of the segment
	 * @param B the endpoint of the segment
	 * @param P the point to be projected
	 *
	 * @return
	 */
	static Vector3D getProjection(const Vector3D &A, const Vector3D &B, const Vector3D &P) {
		Vector3D a = B - A;
		return A + a * (a.dot(P - A) / a.sqlength());
	}

	/**
	 * @brief normalizes the vector
	 */
	void normalize() { *this /= length(); }

	/**
	 * @brief returns a normalized copy of the current vector
	 *
	 * @return
	 */
	inline Vector3D normalized() const {
		double l = length();
		return l > 0 ? *this / l : zero();
	}

	std::string toString() const {
		std::stringstream s;
		s.precision(500);
		s << "(" << coords[0] << " , " << coords[1] << ", " << coords[2] << ")";
		return s.str();
	}
	/**
	 * @brief fast hash for two ints
	 *
	 * @param a
	 * @param b
	 *
	 * @return
	 */
	inline int getHash(const int a, const int b) const {
		unsigned int A = (unsigned int)(a >= 0 ? 2 * a : -2 * a - 1);
		unsigned int B = (unsigned int)(b >= 0 ? 2 * b : -2 * b - 1);
		int C = ((A >= B ? A * A + A + B : A + B * B) / 2);
		return (a < 0 && b < 0) || (a >= 0 && b >= 0) ? C : -C - 1;
	}

	/**
	 * @brief fast hash method
	 *
	 * @return
	 */
	int getHash() const {
		return getHash(
		    static_cast<int>(floor(coords[0])),
		    getHash(static_cast<int>(floor(coords[1])), static_cast<int>(floor(coords[2]))));
	}

	/**
	 * @brief helper method to iterates over a 3D rectangular cuboid bounded by two corner
	 * vectors
	 *
	 * @param v the upper bound corner (lower bound is the current vector)
	 * @param fun a function taking the upper bound vector as well as an increent (default =
	 * 1.0)
	 * @param inc the increment step
	 */
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

	/**
	 * @brief deterministic generation of an orthogonal vector
	 *
	 * @return
	 */
	Vector3D ortho() const {
		if (coords[1] == 0 && coords[0] == 0) {
			return Vector3D(0.0, 1.0, 0.0);
		}
		return Vector3D(-coords[1], coords[0], 0.0);
	}

	Vector3D ortho(const Vector3D &v) const {
		if ((v - *this).sqlength() > 0.000000000001) {
			Vector3D res = cross(v);
			if (res.sqlength() > 0.0000000000001) return res;
		}
		return ortho();
	}

	EXPORTABLE(Vector3D, KV(coords));
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
}  // namespace MecaCell
namespace std {
template <> struct hash<MecaCell::Vector3D> {
	int operator()(const MecaCell::Vector3D &v) const { return v.getHash(); }
};
}  // namespace std
#endif  // VECTOR3D_H
