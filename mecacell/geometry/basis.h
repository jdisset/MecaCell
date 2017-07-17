#ifndef MECACELL_BASIS_H
#define MECACELL_BASIS_H
#include <iostream>
#include "rotation.h"

namespace MecaCell {
template <typename V> struct Basis {
	V X = V(1, 0, 0), Y = V(0, 1, 0);
	Basis() {}
	Basis(const V &x, const V &y) : X(x), Y(y) {}

	V getZ() { return X.cross(Y).normalized(); }

	void updateWithRotation(const Rotation<V> &r) {
		X = V(1, 0, 0).rotated(r).normalized();
		Y = V(0, 1, 0).rotated(r).normalized();
	}

	void rotate(const Rotation<V> &r) {
		X = X.rotated(r);
		Y = Y.rotated(r);
		normalize();
	}

	void normalize() {
		X.normalize();
		Y.normalize();
	}

	Basis rotated(const Rotation<V> &r) {
		return Basis(X.rotated(r).normalized(), Y.rotated(r).normalized());
	}

	template <typename T>
	friend std::ostream &operator<<(std::ostream &out, const Basis<T> &b);
};

template <typename T> inline std::ostream &operator<<(std::ostream &out, const Basis<T> &b) {
	out << "[ " << b.X << ", " << b.Y << ", "<<b.Z<<" ]";
	return out;
}
}
#endif
