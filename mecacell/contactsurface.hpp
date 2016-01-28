#ifndef CONTACTSURFACE_HPP
#define CONTACTSURFACE_HPP
#include "tools.h"
#include "spring.hpp"
#include <utility>
#include <cmath>

#undef DBG
#define DBG DEBUG(contact)
namespace MecaCell {
template <typename Cell> struct ContactSurface {
	ordered_pair<Cell *> cells;

	/***********************************************************
	 *                SETTABLE PARAMETERS
	 **********************************************************/
	float_t staticFrictionCoef = 7.0, dynamicFrictionCoef = 5.0;
	bool pressureEnabled = true;
	bool adhesionEnabled = true;
	bool frictionEnabled = false;

	/////////////// adhesion ///////////////
	float_t adhCoef = 0.5;    // adhesion Coef [0;1]
	float_t dampCoef = 0.9;   // damping [0;1]
	float_t bondMaxL = 2.0;   // max length a surface bond can reach before breaking
	float_t bondReach = 0.3;  // when are new connection created [0;bondMaxL[

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
	static constexpr float_t baseBondStrength = 0.001;
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
	ContactSurface(ordered_pair<Cell *> c) : cells(c) {
		updateInternals();
		if (adhesionEnabled) {
			// we need to init the targets
			Vec ortho = normal.ortho();
			targets.first.b =
			    Basis<Vec>(normal.rotated(cells.first->getOrientationRotation().inverted()),
			               ortho.rotated(cells.first->getOrientationRotation().inverted()));
			targets.first.d = midpoint.first;
			targets.second.b =
			    Basis<Vec>((-normal).rotated(cells.second->getOrientationRotation().inverted()),
			               ortho.rotated(cells.second->getOrientationRotation().inverted()));
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
		normal = cells.second->getPosition() - cells.first->getPosition();
		prevCentersDist = centersDist;
		centersDist = normal.length();
		if (centersDist > DIST_EPSILON) normal /= centersDist;
		midpoint = computeMidpoints(centersDist);
		sqradius = max(0.0, std::pow(cells.first->getMembrane().getDynamicRadius(), 2) -
		                        midpoint.first * midpoint.first);
		area = M_PI * sqradius;
		adhCoef =
		    min(cells.first->getAdhesionWith(
		            cells.second,
		            normal.rotated(cells.first->getOrientationRotation().inverted())),
		        cells.second->getAdhesionWith(
		            cells.first,
		            (-normal).rotated(cells.second->getOrientationRotation().inverted())));
	}

	std::pair<float_t, float_t> computeMidpoints(float_t distanceBtwnCenters) {
		// return the current contact disk's center distance to each cells centers
		if (distanceBtwnCenters <= DIST_EPSILON) return {0, 0};

		auto biggestCell = cells.first->getMembrane().getDynamicRadius() >=
		                           cells.second->getMembrane().getDynamicRadius() ?
		                       cells.first :
		                       cells.second;
		auto smallestCell = biggestCell == cells.first ? cells.second : cells.first;
		float_t biggestCellMidpoint =
		    0.5 * (distanceBtwnCenters +
		           (std::pow(biggestCell->getMembrane().getDynamicRadius(), 2) -
		            std::pow(smallestCell->getMembrane().getDynamicRadius(), 2)) /
		               distanceBtwnCenters);
		float_t smallestCellMidpoint = distanceBtwnCenters - biggestCellMidpoint;
		if (biggestCell == cells.first)
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
		const auto dCoef = 1.0;
		auto speed = max(0.0, (centersDist - prevCentersDist) / dt);
		auto F = (0.5 * (area * (cells.first->getPressure() + cells.second->getPressure())) -
		          speed * dCoef) *
		         normal;
		cells.first->receiveForce(-F);
		cells.second->receiveForce(F);
	}

	void applyAdhesiveForces(double dt) {
		// first we express our targets basis into world basis
		std::pair<Basis<Vec>, Basis<Vec>> targetsBw = {
		    targets.first.b.rotated(cells.first->getOrientationRotation()),
		    targets.second.b.rotated(cells.second->getOrientationRotation())};

		// projections of the target normals onto the current actual collision surface
		Vec midpointPos = cells.first->getPosition() +
		                  normal * midpoint.first;  // we need the actual midpoint position;
		std::pair<double, double> projDist = {
		    Vec::rayCast(midpointPos, normal, cells.first->getPosition(), targetsBw.first.X),
		    Vec::rayCast(midpointPos, normal, cells.second->getPosition(),
		                 targetsBw.second.X)};
		if (projDist.first > 0 && projDist.first < targets.first.d) {
			// we got closer! we need to update our target midpoints
			targets.first.d = projDist.first;
		}
		if (projDist.second > 0 && projDist.second < targets.second.d) {
			targets.second.d = projDist.second;
		}
		// we can now compute the target centers;
		std::pair<Vec, Vec> o = {
		    cells.first->getPosition() + targets.first.d * targetsBw.first.X,
		    cells.second->getPosition() + targets.second.d * targetsBw.second.X};

		// now we apply the forces exerted by all the connections.
		// we apply them at the centers of the two targets

		// compute the connected area. Only needed when the sum of the targets.d changes
		adhArea = max(0.0, std::pow(cells.first->getMembrane().getDynamicRadius(), 2) -
		                       std::pow(std::get<0>(computeMidpoints(targets.first.d +
		                                                             targets.second.d)),
		                                2)) *
		          M_PI;
		// dampingFromRatio(dampCoef, cells.first->getMass() + cells.second->getMass(), k);
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
				auto newX = (o.first - cells.first->getPosition());
				auto newD = newX.length();
				targets.first.b.X =
				    (newX / newD).rotated(cells.first->getOrientationRotation().inverted());
				targets.first.d = newD;
				newX = (o.second - cells.second->getPosition());
				newD = newX.length();
				targets.second.b.X =
				    (newX / newD).rotated(cells.second->getOrientationRotation().inverted());
				targets.second.d = newD;
				linAdhElongation = bondMaxL;
			}
			float_t speed = (linAdhElongation - prevLinAdhElongation) / dt;
			float_t c = 0.0;
			Vec F = 0.5 *
			        (adhArea * adhCoef * baseBondStrength * linAdhElongation - c * speed) *
			        o0o1;
			// cells.first receive F at o0 and in the direction o0o1
			cells.first->receiveForce(F);
			cells.first->receiveTorque((targetsBw.first.X * targets.first.d).cross(F));
			// cells.second receive F at o1 and in the direction -o0o1
			cells.second->receiveForce(-F);
			cells.second->receiveTorque((targetsBw.second.X * targets.second.d).cross(-F));
		}
	}
};
}
#endif
