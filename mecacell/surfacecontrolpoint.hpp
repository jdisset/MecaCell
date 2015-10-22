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

	void updateDirection(const Vec &v, const Rotation<Vec> &rot) {
		direction = v.rotated(r.rotated(rot));
	}
};
}
#endif
