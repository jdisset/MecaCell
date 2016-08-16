#ifndef SURFACECONTROLPOINT_HPP
#define SURFACECONTROLPOINT_HPP
#include "tools.h"

namespace MecaCell {
template <typename RefFrame> struct SurfaceControlPoint {
	RefFrame *rf = nullptr;  // (can be a cell... just needs a basis)
	Rotation<Vec> restRotation, currentRotation;
	Vec restDirection, currentDirection;
	double restDist, currentDist;

	SurfaceControlPoint(){};
	SurfaceControlPoint(RefFrame *RF, const SurfaceControlPoint &scp)
	    : rf(RF),
	      restDist(scp.restDist),
	      currentDist(restDist),
	      restDirection(scp.restDirection),
	      currentDirection(restDirection),
	      restRotation(scp.restRotation),
	      currentRotation(restRotation) {}

	SurfaceControlPoint(RefFrame *RF, Vec v) : rf(RF) {
		restDist = v.length();
		currentDist = restDist;
		restDirection = v / restDist;
		currentDirection = restDirection;
		restRotation =
		    rf->getOrientationRotation().inverted() +
		    Vec::getRotation(Basis<Vec>(), Basis<Vec>(restDirection, restDirection.ortho()));
		currentRotation = restRotation;
	};

	void updateDirection() {
		restDirection = rf->getOrientation().X.rotated(
		    restRotation.rotated(rf->getOrientationRotation()));
		currentDirection = rf->getOrientation().X.rotated(
		    currentRotation.rotated(rf->getOrientationRotation()));
	}
};
}
#endif
