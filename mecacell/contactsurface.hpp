#ifndef CONTACTSURFACE_HPP
#define CONTACTSURFACE_HPP
#include <cmath>
#include <utility>
#include "utilities/utils.h"

#undef DBG
#define DBG DEBUG(contact)
namespace MecaCell {
template <typename Cell> struct ContactSurface {
	ordered_pair<Cell *> cells;

	/***********************************************************
	 *                SETTABLE PARAMETERS
	 **********************************************************/
	double staticFrictionCoef = 7.0, dynamicFrictionCoef = 5.0;
	bool pressureEnabled = true;
	bool adhesionEnabled = true;
	bool frictionEnabled = false;

	/////////////// adhesion ///////////////
	double adhCoef = 0.5;    // adhesion Coef [0;1]
	double dampCoef = 0.01;  // damping [0;1]
	double bondMaxL = 5.0;   // max length a surface bond can reach before breaking
	double bondReach = 1.0;  // when are new connection created [0;bondMaxL[
	double currentDamping = 0.0;

	/*********************************************************
	 * 				     	INTERNALS
	 ********************************************************/
	static constexpr double DIST_EPSILON = 1e-20;
	Vec normal;  // normal of the actual contact surface (from cell 0 to cell 1)
	std::pair<double, double> midpoint;  // distance to center (viewed from each cell)
	double sqradius = 0;                  // squared radius of the contact disk
	double area = 0, adhArea = 0;         // area of the contact disk
	double centersDist = 0, prevCentersDist = 0;  // distance of the two cells centers
	double prevLinAdhElongation = 0, linAdhElongation = 0;  // the elongation of the
	                                                         // connections caused by the
	                                                         // cells separation or flexion
	//////////////// friction ////////////////
	// TODO: implement friction...
	bool atRest = false;  // are the cells at rest, relatively to each other ?
	static constexpr double REST_EPSILON =
	    1e-10;  // minimal speed to consider the connected Cells not to be at rest

	////////////// adhesion //////////////////
	static constexpr double baseBondStrength = 0.005;
	static constexpr double MIN_ADH_DIST = 8.0;
	struct TargetSurface {
		Basis<Vec> b;  // X is the normal of the surface, Y is the up vector
		// b is expressed in the cell's basis, thus if we want to compute b in world basis :
		// Bw = b.rotated(cell->getOrientationRotation());
		double d;  // distance to center of cell
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
	void update(double dt) {
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

	std::pair<double, double> computeMidpoints(double distanceBtwnCenters) {
		// return the current contact disk's center distance to each cells centers
		if (distanceBtwnCenters <= DIST_EPSILON) return {0, 0};

		auto biggestCell = cells.first->getMembrane().getDynamicRadius() >=
		                           cells.second->getMembrane().getDynamicRadius() ?
		                       cells.first :
		                       cells.second;
		auto smallestCell = biggestCell == cells.first ? cells.second : cells.first;
		double biggestCellMidpoint =
		    0.5 * (distanceBtwnCenters +
		           (std::pow(biggestCell->getMembrane().getDynamicRadius(), 2) -
		            std::pow(smallestCell->getMembrane().getDynamicRadius(), 2)) /
		               distanceBtwnCenters);
		double smallestCellMidpoint = distanceBtwnCenters - biggestCellMidpoint;
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
		double adhSpeed = (centersDist - prevCentersDist) / dt;
		auto F = 0.5 * (area * (max(0.0, cells.first->getPressure()) +
		                        max(0.0, cells.second->getPressure()))) *
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
				double halfD = 0.5 * (linAdhElongation - bondMaxL);
				o.first += halfD * o0o1;
				o.second -= halfD * o0o1;
				auto newX = (o.first - cells.first->getPosition());
				auto newD = newX.length();
				if (newD > 0)
					newX /= newD;
				else
					newX = Vec(1, 0, 0);
				targets.first.b.X =
				    newX.rotated(cells.first->getOrientationRotation().inverted());
				targets.first.d = newD;
				newX = (o.second - cells.second->getPosition());
				newD = newX.length();
				if (newD > 0)
					newX /= newD;
				else
					newX = Vec(1, 0, 0);
				targets.second.b.X =
				    newX.rotated(cells.second->getOrientationRotation().inverted());
				targets.second.d = newD;
				linAdhElongation = bondMaxL;
			}
			double springSpeed = (linAdhElongation - prevLinAdhElongation) / dt;
			double k = adhArea * adhCoef * baseBondStrength;
			currentDamping =
			    dampingFromRatio(dampCoef, cells.first->getMass() + cells.second->getMass(), k);

			Vec F = 0.5 * (k * linAdhElongation + currentDamping * springSpeed) * o0o1;
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
