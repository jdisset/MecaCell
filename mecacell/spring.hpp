#ifndef SPRING_HPP
#define SPRING_HPP
#include "tools.h"
namespace MecaCell {
////////////////////////////////////////////////////////////////////
//                SPRING STRUCTURE
////////////////////////////////////////////////////////////////////
// This is just a classic "linear" spring

struct Spring {
	float_t k = 1.0;           // stiffness
	float_t currentK = 1.0;    // stiffness
	float_t c = 1.0;           // damp coef
	float_t restLength = 1.0;  // rest length
	float_t length = 1.0;      // current length
	float_t prevLength = 1.0;
	float_t minLengthRatio = 0.5;  // max compression
	Vec direction;                 // current direction from node 0 to node 1
	bool updatedSinceLastComputeForce = false;

	Spring(){};
	Spring(const float_t &K, const float_t &C, const float_t &L)
	    : k(K), c(C), restLength(L), length(L){};

	void updateLengthDirection(const Vec &p0, const Vec &p1) {
		direction = p1 - p0;
		length = direction.length();
		if (length > 0) direction /= length;
		updatedSinceLastComputeForce = true;
	}
	void setRestLength(float_t L) { restLength = L; }
	inline float_t getRestLength() const { return restLength; }
};

////////////////////////////////////////////////////////////////////
//                       JOINT STRUCTURE
////////////////////////////////////////////////////////////////////
// flexible joint. Can be used for flexure (torque + force) or torsion (torque only)
struct Joint {
	float_t k = 1.0;  // angular stiffness
	float_t currentK = 1.0;
	float_t c = 1.0;                // damp
	float_t maxTeta = M_PI / 30.0;  // maximum angle
	Rotation<Vec> r;                // rotation from node to joint
	Rotation<Vec> delta;            // current rotation
	Rotation<Vec> prevDelta;
	Vec direction;                   // current direction
	Vec target;                      // targeted direction
	bool maxTetaAutoCorrect = true;  // do we need to handle maxTeta?
	bool targetUpdateEnabled = true;
	Joint(){};

	Joint(const float_t &K, const float_t &C, const float_t &MTETA, bool handleMteta = true)
	    : k(K), c(C), maxTeta(MTETA), maxTetaAutoCorrect(handleMteta) {}

	// current direction is computed using a reference Vector v rotated with rotation rot
	void updateDirection(const Vec &v, const Rotation<Vec> &rot) {
		direction = v.rotated(r.rotated(rot));
	}
	void updateDelta() { delta = Vec::getRotation(direction, target); }
	void setCurrentKCoef(float_t kc) { currentK = k * kc; }
};
}

#endif
