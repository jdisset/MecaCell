#ifndef VECTOR3D_H
#define VECTOR3D_H
#include <cmath>
#include <functional>
#include <iostream>
#include "rotation.h"
#include <array>
#include "basis.h"

namespace MecaCell {
class Vector3D {
 public:
	std::array<double, 3> coords;

	static const int dimension = 3;
	Vector3D(double a, double b, double c) : coords{{a, b, c}} {}
	Vector3D() : coords{{0, 0, 0}} {}
	explicit Vector3D(double a) : coords{{a, a, a}} {}
	explicit Vector3D(std::array<double, 3> c) : coords{c} {}

	double dot(const Vector3D &v) const;
	Vector3D cross(const Vector3D &v) const;

	inline double &xRef() { return coords[0]; }
	inline double &yRef() { return coords[1]; }
	inline double &zRef() { return coords[2]; }
	inline double x() const { return coords[0]; }
	inline double y() const { return coords[1]; }
	inline double z() const { return coords[2]; }

	void random();
	Vector3D deltaDirection(double amount);
	static Vector3D randomUnit();
	static Vector3D zero();
	bool isZero() const;

	void operator*=(const double &d);
	void operator/=(const double &d);
	void operator+=(const Vector3D &v);
	Vector3D operator+(const Vector3D &v) const;
	Vector3D operator-(const Vector3D &v) const;
	Vector3D operator-(const double &v) const;
	Vector3D operator+(const double &v) const;
	Vector3D operator/(const double &s) const;
	Vector3D operator/(const Vector3D &v) const;
	Vector3D operator-() const;

	bool operator>=(const double &v) const;
	bool operator<=(const double &v) const;
	bool operator>(const double &v) const;
	bool operator<(const double &v) const;

	double length() const;
	double sqlength() const;

	Vector3D rotated(const double &, const Vector3D &) const;
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

	double getX() const;
	double getY() const;
	double getZ() const;

	void normalize();
	Vector3D normalized() const;

	std::string toString();
	static int getHash(int a, int b);
	std::size_t getHash() const;

	void iterateTo(Vector3D const &v, const std::function<void(const Vector3D &)> &fun,
	               int inc = 1);

	Vector3D ortho() const;
	Vector3D ortho(Vector3D v) const;
	friend ostream &operator<<(ostream &out, const Vector3D &v);
};
Vector3D operator*(const Vector3D &v, const double &s);
Vector3D operator*(const double &s, const Vector3D &v);
bool operator==(const Vector3D &a, const Vector3D &b);
bool operator!=(const Vector3D &a, const Vector3D &b);
}
namespace std {
template <> struct hash<MecaCell::Vector3D> {
	std::size_t operator()(const MecaCell::Vector3D &v) const { return v.getHash(); }
};
}
#endif  // VECTOR3D_H
