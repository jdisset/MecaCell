#ifndef MECACELL_ROTATION_H
#define MECACELL_ROTATION_H
#include <cmath>
using namespace std;
namespace MecaCell {

template <typename V> struct Rotation {
	V n = V(0, 1, 0);
	double teta = 0;

	Rotation() {}

	Rotation(const V& v, const double& f) : n(v), teta(f) {}

	void randomize() {
		n.random();
		n.normalize();
		teta = 1.0;
	}

	Rotation operator+(const V& v) const {
		Rotation R(n, teta);
		V::addAsAngularVelocity(v, R);
		R.compress();
		return R;
	}

	Rotation rotated(const Rotation& r) const { return V::rotateRotation(*this, r); }

	void compress() {  // teta will be < pi
		if (teta > M_PI) {
			if (teta > M_PI * 2.0) teta = fmod(teta, M_PI * 2.0);
			n = -n;
			teta = (2.0 * M_PI) - teta;
		}
	}

	Rotation compressed() const {
		Rotation res(n, teta);
		res.compress();
		return res;
	}

	Rotation inverted() const { return Rotation(-n, teta); }

	Rotation operator+(const Rotation& R2) const {
		Rotation R = V::addRotations(R2, *this);
		R.compress();
		return R;
	}
	Rotation operator-(const Rotation& R2) const {
		Rotation R = V::addRotations(-R2, *this);
		R.compress();
		return R;
	}
};
}

#endif
