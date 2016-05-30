#ifndef CELLMODELCONTACTSURFACE_HPP
#define CELLMODELCONTACTSURFACE_HPP
#include "modelconnection.hpp"

namespace MecaCell {

template <typename Cell> struct CellModelContactSurface {
	Cell* c;
	ModelConnectionPoint bounce;  // always below the cell
	SpaceConnectionPoint anchor;  // connection position (where the springs are)
	// the anchor point is orthogonal to the bounce spring and always at its height

	float_t maxAnchorLength = 2.0;
	float_t bounceLength = 10.0;
	float_t area = 0.0;
	float_t sqradius = 0.0;
	Vec prevnormal;
	Vec normal;
	bool dirty = false;  // does this connection need to be deleted?

	float_t minDist = 1.0;

	const float_t BOUNCE_ABSORB = 0.5;

	CellModelContactSurface(Cell* C, const ModelConnectionPoint& mcp)
	    : c(C), bounce(mcp), anchor(mcp.position) {
		updateInternals();
		prevnormal = normal;
	}

	void update(Vec bounceProjection, size_t bounceFace) {
		std::cerr << "updating. bounce position = " << bounce.position << std::endl;
		bounce.position = bounceProjection;
		bounce.face = bounceFace;
		std::cerr << " now, position = " << bounce.position << std::endl;
		// anchor update
		Vec BA = anchor.position - bounce.position;
		auto anchorLength = BA.length();
		if (anchorLength > maxAnchorLength) {
			// if the anchor is too far away we reproject it closer
			anchor.position = bounce.position + (BA / anchorLength) * maxAnchorLength;
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

	void computeForces(float_t) {
		std::cerr << "computing forces" << std::endl;
		std::cerr << " Bouncelength = " << bounceLength << ", area = " << area
		          << ", pressure = " << c->getPressure() << ", normal = " << normal
		          << ", prvnormal = " << prevnormal << ", faceId = " << bounce.face
		          << std::endl;
		// two main forces : repulsion due to pressure
		// and attraction due to adhesion (anchor)
		// REPULSIVE FORCE :
		if (prevnormal.dot(normal) <= 0) {
			// la cellule essaie de traverser...
			std::cerr << "tentative de traversure. cell pos = " << c->getPosition()
			          << std::endl;
			normal = prevnormal;
			c->setPosition(c->getPosition() + normal * bounceLength);
			c->setVelocity(BOUNCE_ABSORB *
			               (c->getVelocity() - 2.0 * c->getVelocity().dot(normal) * normal));
			c->setForce(Vec(0, 0, 0));
			std::cerr << "apres correction. cell pos = " << c->getPosition() << std::endl;
		}
		auto F = area * max(0.0, c->getPressure()) * normal;
		c->receiveForce(F);
		std::cerr << " F = " << F << std::endl;
		// ADHESIVE FORCE:
		 float_t adhIntensity = c->getAdhesionWithModel(bounce.model->name);
	}
};
}
#endif
