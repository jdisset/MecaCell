#ifndef SURFACECONTROLPOINT_HPP
#define SURFACECONTROLPOINT_HPP
#include "tools.h"

namespace MecaCell {
template <typename RefFrame> struct SurfaceControlPoint {
	RefFrame *rf = nullptr;  // (can be a cell... just needs a basis)
	Rotation<Vec> r;
	Vec direction;
	float_t restDist, currentDist;

	SurfaceControlPoint(RefFrame *RF, Rotation<Vec> &&R, float_t rd)
	    : rf(RF), r(std::move(R)), restDist(rd), currentDist(rd){};
	SurfaceControlPoint(RefFrame *RF, Vec v) : rf(RF) {
		restDist = v.length();
		currentDist = restDist;
		direction = v / restDist;
		r = rf->getOrientationRotation().inverted() +
		    Vec::getRotation(Basis<Vec>(), Basis<Vec>(direction, direction.ortho()));
	};
	void updateDirection() {
		direction = rf->getOrientation().X.rotated(r.rotated(rf->getOrientationRotation()));
	}
};
}
#endif
