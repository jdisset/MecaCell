#ifndef SPRING_HPP
#define SPRING_HPP
#include "utilities/utils.hpp"
namespace MecaCell {
////////////////////////////////////////////////////////////////////
//                SPRING STRUCTURE
////////////////////////////////////////////////////////////////////
// This is just a classic spring-damper

struct Spring {
	double k = 1.0;           // stiffness
	double c = 1.0;           // damp coef
	double restLength = 1.0;  // rest length
	double length = 1.0;      // current length
	double prevLength = 1.0;
	double minLengthRatio = 0.5;  // max compression

	Spring(){};
	Spring(const double &K, const double &C, const double &L)
	    : k(K), c(C), restLength(L), length(L){};

	void updateLength(double l) {
		prevLength = length;
		length = l;
	}

	double computeForce(double dt) {
		double v = (length - prevLength) / dt;
		return 0.5 * (k * (length - restLength) + c * v);
	}

	template <typename A, typename B>
	void applyForce(A &a, B &b, const Vector3D &direction, double dt) {
		// direction = a -> b
		double f = computeForce(dt);
		a.receiveForce(direction * f);
		b.receiveForce(-direction * f);
	}
};

////////////////////////////////////////////////////////////////////
//                       JOINT STRUCTURE
////////////////////////////////////////////////////////////////////
// flexible joint. Can be used for flexure (torque + force) or torsion (torque only)
struct Joint {
	double k = 1.0;                // angular stiffness
	double c = 1.0;                // damp
	double maxTeta = M_PI / 10.0;  // maximum angle
	Rotation<Vec> r;               // rotation from node to joint
	Rotation<Vec> delta;           // current rotation
	Rotation<Vec> prevDelta;
	Vec direction;                   // current direction
	Vec target;                      // targeted direction
	bool maxTetaAutoCorrect = true;  // do we need to handle maxTeta?
	bool targetUpdateEnabled = true;
	Joint(){};

	Joint(const double &K, const double &C, const double &MTETA, bool handleMteta = true)
	    : k(K), c(C), maxTeta(MTETA), maxTetaAutoCorrect(handleMteta) {}

	// current direction is computed using a reference Vector v rotated with rotation rot
	void updateDirection(const Vec &v, const Rotation<Vec> &rot) {
		direction = v.rotated(r.rotated(rot));
	}
	void updateDelta() { delta = Vec::getRotation(direction, target); }
};
}  // namespace MecaCell

#endif
