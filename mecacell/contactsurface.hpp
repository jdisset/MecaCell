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
	bool dampingEnabled = true;
	bool frictionEnabled = false;

	/////////////// adhesion ///////////////
	float_t adhCoef = 0.5;    // adhesion Coef [0;1]
	float_t dampCoef = 0.5;   // damping [0;1]
	float_t bondMaxL = 2.0;   // max length a surface bond can reach before breaking
	float_t bondReach = 0.3;  // when are new connection created [0;bondMaxL[

	/*********************************************************
	 * 				     	INTERNALS
	 ********************************************************/
	Vec normal;  // normal of the actual contact surface (from cell 0 to cell 1)
	std::pair<float_t, float_t> midpoint;      // distance to center (viewed from each cell)
	float_t sqradius = 0;                      // squared radius of the contact disk
	float_t area = 0, adhArea = 0;             // area of the contact disk
	float_t centersDist, prevCentersDist = 0;  // distances of the two cells centers
	float_t speed = 0;
	bool atRest = false;  // are the cells at rest, relatively to each other ?
	static constexpr float_t restEpsilon =
	    1e-10;  // minimal speed to consider the connected Cells not to be at rest
	static constexpr float_t DIST_EPSILON = 1e-20;

	////////////// adhesion //////////////////
	static constexpr float_t baseBondStrength = 0.01;
	struct SimpleSpring {
		float_t k, c, restLength, prevRestLength;
	};

	SimpleSpring normalAdhSpring;

	/*********************************************************
	 * 				        CONSTRUCTORS
	 ********************************************************/
	ContactSurface(){};
	ContactSurface(ordered_pair<Cell *> c) : cells(c) { updateInternals(0.1); }

	/*********************************************************
	 * 				        MAIN UPDATE
	 ********************************************************/
	void update(float_t dt) {
		// first we update all the internals
		updateInternals(dt);
		// then we apply all the forces
		if (pressureEnabled) applyPressureForces();
		if (adhesionEnabled) applyAdhesiveForces();
		if (dampingEnabled) applyDamping();
	};

	/*********************************************************
	 * 				        INTERNALS UPDATES
	 ********************************************************/
	void updateInternals(double dt) {
		normal = cells.second->getPosition() - cells.first->getPosition();
		prevCentersDist = centersDist;
		centersDist = normal.length();
		if (centersDist > DIST_EPSILON) normal /= centersDist;
		midpoint = computeMidpoints(centersDist);
		sqradius = max(0.0, std::pow(cells.first->getMembrane().getDynamicRadius(), 2) -
		                        midpoint.first * midpoint.first);
		area = M_PI * sqradius;
		speed = (centersDist - prevCentersDist) / dt;
		adhCoef =
		    min(cells.first->getAdhesionWith(
		            cells.second,
		            normal.rotated(cells.first->getOrientationRotation().inverted())),
		        cells.second->getAdhesionWith(
		            cells.first,
		            (-normal).rotated(cells.second->getOrientationRotation().inverted())));
		updateAdhSpringParams(normalAdhSpring);
	}

	void updateAdhSpringParams(SimpleSpring &sp) {
		sp.prevRestLength = sp.restLength;
		if (sp.restLength < centersDist - bondMaxL) {
			// cells are too far apart
			sp.restLength = centersDist - bondMaxL;
		} else if (sp.restLength > centersDist - bondReach) {
			sp.restLength = centersDist - bondReach;
		}
		if (sp.prevRestLength != sp.restLength) {
			// we need to recompute the connected area
			adhArea = max(0.0, std::pow(cells.first->getMembrane().getDynamicRadius(), 2) -
			                       std::pow(std::get<0>(computeMidpoints(sp.restLength)), 2)) *
			          M_PI;
			sp.k = adhArea * adhCoef * baseBondStrength;
			sp.c = dampingFromRatio(dampCoef, cells.first->getMass() + cells.second->getMass(),
			                        sp.k);
		}
	}

	std::pair<float_t, float_t> computeMidpoints(float_t distanceBtwnCenters) {
		// return the current contact disk's center distance to each cells centers
		if (centersDist <= DIST_EPSILON) return {0, 0};

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
	void applyPressureForces() {
		// force from pressure is normal to the actual contact surface
		// and proportional to its surface
		auto F = (0.5 * area * (cells.first->getPressure() + cells.second->getPressure())) *
		         normal;
		cells.first->receiveForce(-F);
		cells.second->receiveForce(F);
	}

	void applyAdhesiveForces() {
		// simple adhesive forces act like a spring whos rest length changes
		// (but is always inferior or equal to the centerDist)
		auto F =
		    0.5 * normalAdhSpring.k * (centersDist - normalAdhSpring.restLength) * normal;
		cells.first->receiveForce(F);
		cells.second->receiveForce(-F);
	}

	void applyDamping() {
		auto dCoef = normalAdhSpring.c;
		auto F = speed * dCoef * 0.5 * normal;
		cells.first->receiveForce(F);
		cells.second->receiveForce(-F);
	}
};
}
#endif
