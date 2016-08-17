#ifndef CELLMODELCONTACTSURFACE_HPP
#define CELLMODELCONTACTSURFACE_HPP
#include "modelconnection.hpp"

namespace MecaCell {

template <typename Cell> struct CellModelContactSurface {
	Cell* c;
	ModelConnectionPoint bounce;  // always below the cell
	SpaceConnectionPoint anchor;  // connection position (where the springs are)
	// the anchor point is orthogonal to the bounce spring and always at its height

	double maxAnchorLength = 5.0;
	double bounceLength = 10.0;
	double area = 0.0;
	double sqradius = 0.0;
	Vec prevnormal;
	Vec normal;
	Vec anchorDir;
	double anchorLength = 0;
	bool dirty = false;  // does this connection need to be deleted?
	static constexpr double baseBondStrength = 0.005;

	double minDist = 1.0;

	const double BOUNCE_ABSORB = 1.0;

	CellModelContactSurface(Cell* C, const ModelConnectionPoint& mcp)
	    : c(C), bounce(mcp), anchor(mcp.position) {
		updateInternals();
		prevnormal = normal;
	}

	void update(Vec bounceProjection, size_t bounceFace) {
		bounce.position = bounceProjection;
		bounce.face = bounceFace;
		// anchor update
		anchorDir = anchor.position - bounce.position;
		anchorLength = anchorDir.length();
		anchorDir = anchorDir / anchorLength;
		if (anchorLength > maxAnchorLength) {
			// if the anchor is too far away we reproject it closer
			anchor.position = bounce.position + anchorDir * maxAnchorLength;
		}
		updateInternals();
	}

	void updateInternals() {
		Vec bounceDir = c->getPosition() - bounce.position;
		bounceLength = bounceDir.length();
		sqradius = max(0.0, std::pow(c->getMembrane().getDynamicRadius(), 2) -
		                        std::pow(bounceLength, 2));
		area = M_PI * sqradius;
		prevnormal = normal;
		if (bounceLength > 0) {
			normal = bounceDir / bounceLength;
		}
	}

	void computeForces(double) {
		// REPULSIVE FORCE :
		if (prevnormal.dot(normal) <= 0) {
			// la cellule essaie de traverser...
			normal = prevnormal;
			c->setPosition(c->getPosition() + normal * (1.1 + bounceLength));
			c->setVelocity(BOUNCE_ABSORB *
			               (c->getVelocity() - 2.0 * c->getVelocity().dot(normal) * normal));
			c->setForce(Vec(0, 0, 0));
		}
		auto F = area * max(0.0, c->getPressure()) * normal;
		c->receiveForce(F);

		// FRICTION FORCE:
		// ADHESION FORCE:
	}
};
}
#endif
