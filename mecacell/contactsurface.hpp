#ifndef CONTACTSURFACE_HPP
#define CONTACTSURFACE_HPP
#include "tools.h"
#include "spring.hpp"
#include <utility>
#include <cmath>

#undef DBG
#define DBG DEBUG(contact)
namespace MecaCell {
//  pour les connections avec obj, on crée une cellule virtuelle gigantesque
//  ratachée à un point sur le modele


template <typename Cell, typename Other = Cell> struct ContactSurface {
	// Other can either be a cell or a 3D obj
	Cell* c0;
	Other* c1;

	/***********************************************************
	 *                SETTABLE PARAMETERS
	 **********************************************************/
	float_t staticFrictionCoef = 7.0, dynamicFrictionCoef = 5.0;
	bool pressureEnabled = true;
	bool adhesionEnabled = true;
	bool frictionEnabled = false;

	/////////////// adhesion ///////////////
	float_t adhCoef = 0.5;    // adhesion Coef [0;1]
	float_t dampCoef = 0.01;  // damping [0;1]
	float_t bondMaxL = 5.0;   // max length a surface bond can reach before breaking
	float_t bondReach = 1.0;  // when are new connection created [0;bondMaxL[
	float_t currentDamping = 0.0;

	/*********************************************************
	 * 				     	INTERNALS
	 ********************************************************/
	static constexpr float_t DIST_EPSILON = 1e-20;
	Vec normal;  // normal of the actual contact surface (from cell 0 to cell 1)
	std::pair<float_t, float_t> midpoint;  // distance to center (viewed from each cell)
	float_t sqradius = 0;                  // squared radius of the contact disk
	float_t area = 0, adhArea = 0;         // area of the contact disk
	float_t centersDist = 0, prevCentersDist = 0;  // distance of the two cells centers
	float_t prevLinAdhElongation = 0, linAdhElongation = 0;  // the elongation of the
	                                                         // connections caused by the
	                                                         // cells separation or flexion
	//////////////// friction ////////////////
	// TODO: implement friction...
	bool atRest = false;  // are the cells at rest, relatively to each other ?
	static constexpr float_t REST_EPSILON =
	    1e-10;  // minimal speed to consider the connected Cells not to be at rest

	////////////// adhesion //////////////////
	static constexpr float_t baseBondStrength = 0.005;
	static constexpr float_t MIN_ADH_DIST = 8.0;
	struct TargetSurface {
		Basis<Vec> b;  // X is the normal of the surface, Y is the up vector
		// b is expressed in the cell's basis, thus if we want to compute b in world basis :
		// Bw = b.rotated(cell->getOrientationRotation());
		float_t d;  // distance to center of cell
	};

	std::pair<TargetSurface, TargetSurface> targets;

	/*********************************************************
	 * 				        CONSTRUCTORS
	 ********************************************************/
	ContactSurface(){};
	ContactSurface(Cell* c, Other* o) : c0(c), c1(o) {
		updateInternals();
		if (adhesionEnabled) {
			// we need to init the targets
			Vec ortho = normal.ortho();
			targets.first.b =
			    Basis<Vec>(normal.rotated(c0->getOrientationRotation().inverted()),
			               ortho.rotated(c0->getOrientationRotation().inverted()));
			targets.first.d = midpoint.first;
			targets.second.b =
			    Basis<Vec>((-normal).rotated(c1->getOrientationRotation().inverted()),
			               ortho.rotated(c1->getOrientationRotation().inverted()));
			targets.second.d = midpoint.second;
		}
	}

	/*********************************************************
	 * 				        MAIN UPDATE
	 ********************************************************/
	void update(float_t dt) {
		// first we update all the internals
		updateInternals();
		// then we apply all the forces
		if (pressureEnabled) applyPressureForces(dt);
		if (adhesionEnabled) applyAdhesiveForces(dt);
	};

	/*********************************************************
	 * 				        INTERNALS UPDATES
	 ********************************************************/
	void updateInternals() {
		normal = c0->getPosition() - c1->getPosition();
		prevCentersDist = centersDist;
		centersDist = normal.length();
		if (centersDist > DIST_EPSILON) normal /= centersDist;
		midpoint = computeMidpoints(centersDist);
		sqradius = max(
		    0.0, std::pow(c0->getBoundingBoxRadius(), 2) - midpoint.first * midpoint.first);
		area = M_PI * sqradius;
		adhCoef = min(
		    c0->getAdhesionWith(c1, normal.rotated(c0->getOrientationRotation().inverted())),
		    c1->getAdhesionWith(c0,
		                        (-normal).rotated(c1->getOrientationRotation().inverted())));
	}

	std::pair<float_t, float_t> computeMidpoints(float_t distanceBtwnCenters) {
		// return the current contact disk's center distance to each cells centers
		if (distanceBtwnCenters <= DIST_EPSILON) return {0, 0};

		auto biggestCell = c0->getBoundingBoxRadius() >= c1->getBoundingBoxRadius() ? c0 : c1;
		auto smallestCell = biggestCell == c0 ? c1 : c0;
		float_t biggestCellMidpoint =
		    0.5 * (distanceBtwnCenters +
		           (std::pow(biggestCell->getBoundingBoxRadius(), 2) -
		            std::pow(smallestCell->getBoundingBoxRadius(), 2)) /
		               distanceBtwnCenters);
		float_t smallestCellMidpoint = distanceBtwnCenters - biggestCellMidpoint;
		if (biggestCell == c0)
			return {biggestCellMidpoint, smallestCellMidpoint};
		else
			return {smallestCellMidpoint, biggestCellMidpoint};
	}

	/*********************************************************
	 * 				     FORCES COMPUTATIONS
	 ********************************************************/
	// All forces applying method assume the internal values have
	// correctly been updated before

	// from internal pressure
	void applyPressureForces(double dt) {
		// force from pressure is normal to the actual contact surface
		// and proportional to its surface
		float_t adhSpeed = (centersDist - prevCentersDist) / dt;
		auto F = 0.5 * (area * (max(0.0, c0->getPressure()) + max(0.0, c1->getPressure()))) *
		         normal;
		c0->receiveForce(-F);
		c1->receiveForce(F);
	}

	void applyAdhesiveForces(double dt) {
		// first we express our targets basis into world basis
		std::pair<Basis<Vec>, Basis<Vec>> targetsBw = {
		    targets.first.b.rotated(c0->getOrientationRotation()),
		    targets.second.b.rotated(c1->getOrientationRotation())};

		// projections of the target normals onto the current actual collision surface
		Vec midpointPos = c0->getPosition() +
		                  normal * midpoint.first;  // we need the actual midpoint position;
		std::pair<double, double> projDist = {
		    Vec::rayCast(midpointPos, normal, c0->getPosition(), targetsBw.first.X),
		    Vec::rayCast(midpointPos, normal, c1->getPosition(), targetsBw.second.X)};
		if (projDist.first > MIN_ADH_DIST && projDist.first - bondReach < targets.first.d) {
			// we got closer! we need to update our target midpoints
			targets.first.d = projDist.first - bondReach;
		}
		if (projDist.second > MIN_ADH_DIST &&
		    projDist.second - bondReach < targets.second.d) {
			targets.second.d = projDist.second - bondReach;
		}
		if (targets.first.d < MIN_ADH_DIST) targets.first.d = MIN_ADH_DIST;
		if (targets.second.d < MIN_ADH_DIST) targets.second.d = MIN_ADH_DIST;
		// we can now compute the target centers;
		std::pair<Vec, Vec> o = {c0->getPosition() + targets.first.d * targetsBw.first.X,
		                         c1->getPosition() + targets.second.d * targetsBw.second.X};

		// now we apply the forces exerted by all the connections.
		// we apply them at the centers of the two targets

		// compute the connected area. Only needed when the sum of the targets.d changes
		adhArea = max(0.0, std::pow(c0->getBoundingBoxRadius(), 2) -
		                       std::pow(std::get<0>(computeMidpoints(targets.first.d +
		                                                             targets.second.d)),
		                                2)) *
		          M_PI;
		// dampingFromRatio(dampCoef, c0->getMass() + c1->getMass(), k);
		// we can now compute the forces applied at o.first & o.second
		Vec o0o1 = o.second - o.first;
		prevLinAdhElongation = linAdhElongation;
		linAdhElongation = o0o1.length();
		if (linAdhElongation > DIST_EPSILON) {
			o0o1 /= linAdhElongation;
			if (linAdhElongation > bondMaxL) {
				// some bonds are going to break
				float_t halfD = 0.5 * (linAdhElongation - bondMaxL);
				o.first += halfD * o0o1;
				o.second -= halfD * o0o1;
				auto newX = (o.first - c0->getPosition());
				auto newD = newX.length();
				if (newD > 0)
					newX /= newD;
				else
					newX = Vec(1, 0, 0);
				targets.first.b.X = newX.rotated(c0->getOrientationRotation().inverted());
				targets.first.d = newD;
				newX = (o.second - c1->getPosition());
				newD = newX.length();
				if (newD > 0)
					newX /= newD;
				else
					newX = Vec(1, 0, 0);
				targets.second.b.X = newX.rotated(c1->getOrientationRotation().inverted());
				targets.second.d = newD;
				linAdhElongation = bondMaxL;
			}
			float_t springSpeed = (linAdhElongation - prevLinAdhElongation) / dt;
			float_t k = adhArea * adhCoef * baseBondStrength;
			currentDamping = dampingFromRatio(dampCoef, c0->getMass() + c1->getMass(), k);

			Vec F = 0.5 * (k * linAdhElongation + currentDamping * springSpeed) * o0o1;
			// c0 receive F at o0 and in the direction o0o1
			c0->receiveForce(F);
			c0->receiveTorque((targetsBw.first.X * targets.first.d).cross(F));
			// c1 receive F at o1 and in the direction -o0o1
			c1->receiveForce(-F);
			c1->receiveTorque((targetsBw.second.X * targets.second.d).cross(-F));
		}
	}
};
}
#endif
