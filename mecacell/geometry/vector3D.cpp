#include <cmath>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <random>
#include <sstream>
#include "quaternion.h"
#include "rotation.h"
#include "vector3D.h"

namespace MecaCell {

// shortcut for Vector3D
void Vector3D::random() {
	std::normal_distribution<double> nDist(0.0, 1.0);
	coords = {{nDist(globalRand), nDist(globalRand), nDist(globalRand)}};
	normalize();
}

Vector3D Vector3D::randomUnit() {
	Vector3D v;
	v.random();
	return v;
}

Vector3D Vector3D::deltaDirection(double amount) {
	std::normal_distribution<double> nDist(0.0, amount);
	return Vector3D(coords[0] + nDist(globalRand), coords[1] + nDist(globalRand),
	                coords[2] + nDist(globalRand))
	    .normalized();
}

Vector3D Vector3D::zero() { return Vector3D(0.0, 0.0, 0.0); }

bool Vector3D::isZero() const {
	return (coords[0] == 0.0 && coords[1] == 0.0 && coords[2] == 0.0);
}

double Vector3D::length() const {
	return sqrt(coords[0] * coords[0] + coords[1] * coords[1] + coords[2] * coords[2]);
}
double Vector3D::sqlength() const {
	return coords[0] * coords[0] + coords[1] * coords[1] + coords[2] * coords[2];
}

void Vector3D::normalize() { *this /= length(); }

std::string Vector3D::toString() const {
	std::stringstream s;
	s.precision(500);
	s << "(" << coords[0] << " , " << coords[1] << ", " << coords[2] << ")";
	return s.str();
}

int Vector3D::getHash(int a, int b) {
	unsigned int A = (unsigned int)(a >= 0 ? 2 * a : -2 * a - 1);
	unsigned int B = (unsigned int)(b >= 0 ? 2 * b : -2 * b - 1);
	int C = ((A >= B ? A * A + A + B : A + B * B) / 2);
	return (a < 0 && b < 0) || (a >= 0 && b >= 0) ? C : -C - 1;
}

int Vector3D::getHash() const {
	return getHash(
	    static_cast<int>(floor(coords[0])),
	    getHash(static_cast<int>(floor(coords[1])), static_cast<int>(floor(coords[2]))));
}

Vector3D Vector3D::ortho() const {
	if (coords[1] == 0 && coords[0] == 0) {
		return Vector3D(0.0, 1.0, 0.0);
	}
	return Vector3D(-coords[1], coords[0], 0.0);
}
Vector3D Vector3D::ortho(const Vector3D &v) const {
	if ((v - *this).sqlength() > 0.000000000001) {
		Vector3D res = cross(v);
		if (res.sqlength() > 0.0000000000001) return res;
	}
	return ortho();
}

Vector3D Vector3D::rotated(double angle, const Vector3D &vec) const {
	double halfangle = angle * 0.5;
	Vector3D v = vec * sin(halfangle);
	Vector3D vcV = 2.0 * v.cross(*this);
	return *this + cos(halfangle) * vcV + v.cross(vcV);
}

Vector3D Vector3D::rotated(const Rotation<Vector3D> &r) const {
	double halfangle = r.teta * 0.5;
	Vector3D v = r.n * sin(halfangle);
	Vector3D vcV = 2.0 * v.cross(*this);
	return *this + cos(halfangle) * vcV + v.cross(vcV);
}
// return Quaternion(r.teta, r.n) * *this; }

Rotation<Vector3D> Vector3D::rotateRotation(const Rotation<Vector3D> &start,
                                            const Rotation<Vector3D> &offset) {
	return Rotation<Vector3D>(start.n.rotated(offset), start.teta);
}

Rotation<Vector3D> Vector3D::addRotations(const Rotation<Vector3D> &R0,
                                          const Rotation<Vector3D> &R1) {
	Quaternion q2 = Quaternion(R1.teta, R1.n) * Quaternion(R0.teta, R0.n);
	q2.normalize();
	return q2.toAxisAngle();
}

void Vector3D::addAsAngularVelocity(const Vector3D &v, Rotation<Vector3D> &r) {
	double dTeta = v.length();
	Vector3D n0(0, 1, 0);
	if (dTeta > 0) {
		n0 = v / dTeta;
	}
	r = addRotations(r, Rotation<Vector3D>(n0, dTeta));
}

double Vector3D::rayCast(const Vector3D &o, const Vector3D &n, const Vector3D &p,
                          const Vector3D &r) {
	// returns l such that p + l.r lies on the plane defined by its normal n and an offset o
	// l > 0 means that the ray hits the plane,
	// l < 0 means that the racoords[1] does not face the plane
	// l = 0 means that the ray is parallel to the plane or that p is on the plane
	double nr = n.dot(r);
	return (nr == 0) ? 0 : n.dot(o - p) / nr;
}

Vector3D Vector3D::getProjectionOnPlane(const Vector3D &o, const Vector3D &n,
                                        const Vector3D &p) {
	// returns the projection of p onto a plane defined bcoords[1] its normal n and an
	// offset o
	return p - (n.dot(p - o) * n);
}

Vector3D Vector3D::getProjection(const Vector3D &origin, const Vector3D &B,
                                 const Vector3D &P) {
	// returns the projected P point onto the origin -> B vector
	Vector3D a = B - origin;
	return origin + a * (a.dot(P - origin) / a.sqlength());
}

Rotation<Vector3D> Vector3D::getRotation(const Vector3D &v0, const Vector3D &v1) {
	Rotation<Vector3D> res;
	res.teta = acos(min<double>(1.0, max<double>(-1.0, v0.dot(v1))));
	Vector3D cross = v0.cross(v1);
	if (cross.sqlength() == 0) {
		cross = Vector3D(0, 1, 0);
	}
	res.n = cross.normalized();
	return res;
}

Rotation<Vector3D> Vector3D::getRotation(const Basis<Vector3D> &b0,
                                         const Basis<Vector3D> &b1) {
	return getRotation(b0.X, b0.Y, b1.X, b1.Y);
}

Rotation<Vector3D> Vector3D::getRotation(const Vector3D &X0, const Vector3D &Y0,
                                         const Vector3D &X1, const Vector3D &Y1) {
	Quaternion q0(X0.normalized(), X1.normalized());
	Vector3D Ytmp = q0 * Y0;
	Ytmp.normalize();
	Quaternion qres = Quaternion(Ytmp, Y1.normalized()) * q0;
	qres.normalize();
	return qres.toAxisAngle();
}
}
