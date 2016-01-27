#ifndef CONTACTSURFACE_HPP
#define CONTACTSURFACE_HPP
#include "tools.h"
#include <utility>
#include <cmath>

#define TRACE 0

namespace MecaCell {
template <typename Cell> struct ContactSurface {
	ordered_pair<Cell *> cells;

	/***********************************************************
	 *                SETTABLE PARAMETERS
	 **********************************************************/
	float_t staticFrictionCoef = 7.0, dynamicFrictionCoef = 5.0;
	bool pressureEnabled = true;
	bool adhesionEnabled = false;
	bool frictionEnabled = false;
	float_t pressureDamping = 0.1;
	float_t adhCoef = 0;       // adhesion Coef
	float_t bondMaxL = 1.5;    // max length a surface bond can reach before breaking
	float_t distToMaxL = 1.0;  // at wich distance from contact do the bonds reach their
	                           // maxL (related to the average curvature of
	                           // the membrane around contact surface

	/*********************************************************
	 * 				     	INTERNALS
	 ********************************************************/
	Vec normal;  // normal of the actual contact surface (from cell 0 to cell 1)
	std::pair<float_t, float_t> midpoint;      // distance to center (viewed from each cell)
	float_t sqradius = 0;                      // squared radius of the contact disk
	float_t area = 0;                          // area of the contact disk
	float_t centersDist, prevCentersDist = 0;  // distances of the two cells centers
	bool atRest = false;  // are the cells at rest, relatively to each other ?
	static constexpr float_t restEpsilon =
	    1e-10;  // minimal speed to consider the connected Cells not to be at rest
	static constexpr float_t DIST_EPSILON = 1e-20;

	/*********************************************************
	 * 				        CONSTRUCTORS
	 ********************************************************/
	ContactSurface(){};
	ContactSurface(ordered_pair<Cell *> c) : cells(c) { updateInternals(); }

	/*********************************************************
	 * 				        MAIN UPDATE
	 ********************************************************/
	void update(float_t dt) {
		// first we update all the internals
		updateInternals();
		// then we apply all the forces
		if (pressureEnabled) applyPressureForces(dt);
	};

	/*********************************************************
	 * 				        INTERNALS UPDATES
	 ********************************************************/
	void updateInternals() {
		normal = cells.second->getPosition() - cells.first->getPosition();
		prevCentersDist = centersDist;
		centersDist = normal.length();
		if (centersDist > DIST_EPSILON) normal /= centersDist;
		midpoint = computeMidpoints();
		sqradius = std::pow(cells.first->getMembrane().getDynamicRadius(), 2) -
		           midpoint.first * midpoint.first;
		area = M_PI * sqradius;
	}

	std::pair<float_t, float_t> computeMidpoints() {
		// return the current contact disk's center distance to each cells centers
		if (centersDist <= DIST_EPSILON) return {0, 0};

		auto biggestCell = cells.first->getMembrane().getDynamicRadius() >=
		                           cells.second->getMembrane().getDynamicRadius() ?
		                       cells.first :
		                       cells.second;
		auto smallestCell = biggestCell == cells.first ? cells.second : cells.first;
		float_t biggestCellMidpoint =
		    0.5 * (centersDist +
		           (std::pow(biggestCell->getMembrane().getDynamicRadius(), 2) -
		            std::pow(smallestCell->getMembrane().getDynamicRadius(), 2)) /
		               centersDist);
		float_t smallestCellMidpoint = centersDist - biggestCellMidpoint;
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
		float_t f_speed = 0;  // pressureDamping * fabs(centersDist - prevCentersDist) / dt;
		auto F = (0.5 * area * (cells.first->getPressure() + cells.second->getPressure()) -
		          f_speed) *
		         normal;
		cells.first->receiveForce(-F);
		cells.second->receiveForce(F);
	}
};
}
#endif
