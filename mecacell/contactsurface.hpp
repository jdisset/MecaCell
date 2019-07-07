#ifndef CONTACTSURFACE_HPP
#define CONTACTSURFACE_HPP
#include <cmath>
#include <utility>
#include "utilities/ordered_pair.hpp"
#include "utilities/utils.hpp"

namespace MecaCell {
template <typename Cell> struct ContactSurface {
	ordered_pair<Cell *> cells;

	/***********************************************************
	 *                SETTABLE PARAMETERS
	 **********************************************************/
	bool pressureEnabled = true;
	bool adhesionEnabled = true;
	bool frictionEnabled = false;
	bool unbreakable = false;  // is this adhesion unbreakable ?

	/////////////// adhesion ///////////////
	num_t adhCoef = 0.5;    // adhesion Coef [0;1]
	num_t ADH_DAMPING_RATIO= 0.1;   // damping [0;1]
	num_t bondMaxL = 5.0;   // max length a surface bond can reach before breaking
	num_t bondReach = 1.0;  // when are new connection created [0;bondMaxL[
	num_t fixedDamping = 0.0;
	num_t baseBondStrength = 0.05;
	num_t MIN_ADH_DIST = 8.0;

	/*********************************************************
	 * 				     	INTERNALS
	 ********************************************************/
	Vec direction;  // direction of the actual contact surface (from cell 0 to cell 1)
	std::pair<num_t, num_t> midpoint;  // distance to center (viewed from each cell)
	num_t sqradius = 0;                 // squared radius of the contact disk
	num_t area = 0, adhArea = 0;        // area of the contact disk
	num_t centersDist = 0, prevCentersDist = 0;  // distance of the two cells centers
	num_t prevLinAdhElongation = 0, linAdhElongation = 0;    // the elongation of the
	                                                          // connections caused by the
	                                                          // cells separation
	num_t prevFlexAdhElongation = 0, flexAdhElongation = 0;  // the elongation of the
	                                                          // connections caused by the
	                                                          // cells relative flexure
	//////////////// friction ////////////////
	// TODO: implement friction...
	bool atRest = false;  // are the cells at rest, relatively to each other ?
	static constexpr num_t REST_EPSILON =
	    1e-10;  // minimal speed to consider the connected Cells not to be at rest

	////////////// adhesion //////////////////
	struct TargetSurface {
		Basis<Vec> b;  // X is the direction of the surface, Y is the up vector
		// b is expressed in the cell's basis, thus if we want to compute b in world basis :
		// Bw = b.rotated(cell->getOrientationRotation());
		num_t d;  // distance to center of cell
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
			Vec ortho = direction.ortho();
			targets.first.b = Basis<Vec>(
			    direction.rotated(cells.first->getBody().getOrientationRotation().inverted()),
			    ortho.rotated(cells.first->getBody().getOrientationRotation().inverted()));
			targets.first.d = midpoint.first / cells.first->getBody().getDynamicRadius();
			targets.second.b = Basis<Vec>(
			    (-direction)
			        .rotated(cells.second->getBody().getOrientationRotation().inverted()),
			    ortho.rotated(cells.second->getBody().getOrientationRotation().inverted()));
			targets.second.d = midpoint.second / cells.second->getBody().getDynamicRadius();
		}
	}

	/*********************************************************
	 * 				        MAIN UPDATE
	 ********************************************************/
	void update(num_t dt) {
		// first we update all the internals
		updateInternals();
		// then we apply all the forces
		if (pressureEnabled) applyPressureForces(dt);
		if (adhesionEnabled) applyAdhesiveForces(dt);
	}

	/*********************************************************
	 * 				        INTERNALS UPDATES
	 ********************************************************/
	void updateInternals() {
		direction = cells.second->getPosition() - cells.first->getPosition();
		prevCentersDist = centersDist;
		centersDist = direction.length();
		if (centersDist > Config::DOUBLE_EPSILON) direction /= centersDist;
		midpoint = computeMidpoints(centersDist);
		sqradius = max(0.0, std::pow(cells.first->getBody().getDynamicRadius(), 2) -
		                        midpoint.first * midpoint.first);
		area = M_PI * sqradius;
		adhCoef = min(
		    cells.first->getAdhesionWith(
		        cells.second,
		        direction.rotated(
		            cells.first->getBody().getOrientationRotation().inverted())),
		    cells.second->getAdhesionWith(
		        cells.first,
		        (-direction)
		            .rotated(cells.second->getBody().getOrientationRotation().inverted())));
	}

	std::pair<num_t, num_t> computeMidpoints(num_t distanceBtwnCenters) {
		// return the current contact disk's center distance to each cells centers
		if (distanceBtwnCenters <= Config::DOUBLE_EPSILON) return {0, 0};

		auto biggestCell = cells.first->getBody().getDynamicRadius() >=
		                           cells.second->getBody().getDynamicRadius() ?
		                       cells.first :
		                       cells.second;
		auto smallestCell = biggestCell == cells.first ? cells.second : cells.first;

		num_t biggestCellMidpoint =
		    0.5 *
		    (distanceBtwnCenters + (std::pow(biggestCell->getBody().getDynamicRadius(), 2) -
		                            std::pow(smallestCell->getBody().getDynamicRadius(), 2)) /
		                               distanceBtwnCenters);
		num_t smallestCellMidpoint = distanceBtwnCenters - biggestCellMidpoint;
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
	void applyPressureForces(num_t) {
		// force from pressure is direction to the actual contact surface
		// and proportional to its surface
		// num_t adhSpeed = (centersDist - prevCentersDist) / dt;
		auto F = 0.5 *
		         (area * (max(0.0, cells.first->getBody().getPressure()) +
		                  max(0.0, cells.second->getBody().getPressure()))) *
		         direction;
		cells.first->getBody().receiveForce(-F);
		cells.second->getBody().receiveForce(F);
	}

	void applyAdhesiveForces(num_t dt) {
		// two main spring like forces :
		// torsion + linear = targets points trying to meet.
		// flexure = midpoint trying to align with the targets points.
		std::pair<num_t, num_t> computedTargetsDistances = {
		    targets.first.d * cells.first->getBody().getDynamicRadius(),
		    targets.second.d * cells.second->getBody().getDynamicRadius()};
		// first we express our targets basis into world basis
		std::pair<Basis<Vec>, Basis<Vec>> targetsBw = {
		    targets.first.b.rotated(cells.first->getBody().getOrientationRotation()),
		    targets.second.b.rotated(cells.second->getBody().getOrientationRotation())};

		// projections of the target directions onto the current actual collision surface
		Vec midpointPos =
		    cells.first->getPosition() +
		    direction * midpoint.first;  // we need the actual midpoint position;

		// std::pair<num_t, num_t> projDist = {
		// Vec::rayCast(midpointPos, direction, cells.first->getPosition(),
		// targetsBw.first.X),
		// Vec::rayCast(midpointPos, direction, cells.second->getPosition(),
		// targetsBw.second.X)};
		// if (!fixedAdhesion && projDist.first > MIN_ADH_DIST &&
		// projDist.first - bondReach < computedTargetsDistances.first) {
		//// we got closer! we need to update our target midpoints
		// computedTargetsDistances.first = projDist.first - bondReach;
		// targets.first.d =
		// computedTargetsDistances.first / cells.first->getBody().getDynamicRadius();
		//}
		// if (!fixedAdhesion && projDist.second > MIN_ADH_DIST &&
		// projDist.second - bondReach < computedTargetsDistances.second) {
		// computedTargetsDistances.second = projDist.second - bondReach;
		// targets.second.d =
		// computedTargetsDistances.second / cells.second->getBody().getDynamicRadius();
		//}
		if (computedTargetsDistances.first < MIN_ADH_DIST) {
			computedTargetsDistances.first = MIN_ADH_DIST;
			targets.first.d =
			    computedTargetsDistances.first / cells.first->getBody().getDynamicRadius();
		}
		if (computedTargetsDistances.second < MIN_ADH_DIST) {
			computedTargetsDistances.second = MIN_ADH_DIST;
			targets.second.d =
			    computedTargetsDistances.second / cells.second->getBody().getDynamicRadius();
		}
		// we can now compute the target centers;
		std::pair<Vec, Vec> o = {
		    cells.first->getPosition() + computedTargetsDistances.first * targetsBw.first.X,
		    cells.second->getPosition() +
		        computedTargetsDistances.second * targetsBw.second.X};

		// now we apply the forces exerted by all the connections.
		// we apply them at the centers of the two targets

		// compute the connected area. Only needed when the sum of the targets.d changes
		adhArea =
		    max(0.0,
		        std::pow(cells.first->getBody().getDynamicRadius(), 2) -
		            std::pow(std::get<0>(computeMidpoints(computedTargetsDistances.first +
		                                                  computedTargetsDistances.second)),
		                     2)) *
		    M_PI;
		// we can now compute the forces applied at o.first & o.second
		Vec o0o1 = o.second - o.first;
		prevLinAdhElongation = linAdhElongation;
		linAdhElongation = o0o1.length();
		// targets midpoint, where the actual midpoint is going to try to align
		Vec targetMidpoint = o.first + o0o1 / 2;
		Vec m0m1 = targetMidpoint - midpointPos;
		prevFlexAdhElongation = flexAdhElongation;
		flexAdhElongation = m0m1.length();

		// on axis forces
		if (linAdhElongation > Config::DOUBLE_EPSILON) {
			o0o1 /= linAdhElongation;
			if (linAdhElongation > bondMaxL && !unbreakable) {
				// some bonds are going to break
				num_t halfD = 0.5 * (linAdhElongation - bondMaxL);
				o.first += halfD * o0o1;
				o.second -= halfD * o0o1;
				auto newX = (o.first - cells.first->getPosition());
				auto newD = newX.length();
				if (newD > 0)
					newX /= newD;
				else
					newX = Vec(1, 0, 0);
				targets.first.b.X =
				    newX.rotated(cells.first->getBody().getOrientationRotation().inverted());
				targets.first.d = newD / cells.first->getBody().getDynamicRadius();
				newX = (o.second - cells.second->getPosition());
				newD = newX.length();
				if (newD > 0)
					newX /= newD;
				else
					newX = Vec(1, 0, 0);
				targets.second.b.X =
				    newX.rotated(cells.second->getBody().getOrientationRotation().inverted());
				targets.second.d = newD / cells.second->getBody().getDynamicRadius();
				linAdhElongation = bondMaxL;
			}
			num_t springSpeed = (linAdhElongation - prevLinAdhElongation) / dt;
			num_t k = adhArea * adhCoef * baseBondStrength;
			num_t ratioDamping = dampingFromRatio(
			    ADH_DAMPING_RATIO, cells.first->getBody().getMass() + cells.second->getBody().getMass(),
			    k);

			Vec F = 0.5 * (k * linAdhElongation + (fixedDamping + ratioDamping) * springSpeed) *
			        o0o1;
			// cells.first receive F at o0 and in the direction o0o1
			cells.first->getBody().receiveForce(F);
			cells.first->getBody().receiveTorque(
			    (targetsBw.first.X * computedTargetsDistances.first).cross(F));
			// cells.second receive F at o1 and in the direction -o0o1
			cells.second->getBody().receiveForce(-F);
			cells.second->getBody().receiveTorque(
			    (targetsBw.second.X * computedTargetsDistances.second).cross(-F));
		}
		// midpoints alignment forces
		// if (flexAdhElongation > DIST_EPSILON) {
		// num_t springSpeed = (flexAdhElongation - prevFlexAdhElongation) / dt;
		// num_t k = adhArea * adhCoef * baseBondStrength;
		// num_t ratioDamping =
		// dampingFromRatio(ADH_DAMPING_RATIO, cells.first->getMass() + cells.second->getMass(), k);
		// Vec F = 0.5 *
		//(k * flexAdhElongation + (fixedDamping + ratioDamping) * springSpeed) *
		// m0m1;
		//// cells.first->receiveForce(F);
		// cells.first->getBody().receiveTorque(
		//(targetsBw.first.X * computedTargetsDistances.first).cross(F));
		//// cells.second receive F at o1 and in the direction -o0o1
		//// cells.second->receiveForce(F);
		// cells.second->getBody().receiveTorque(
		//(targetsBw.second.X * computedTargetsDistances.second).cross(F));
		//}
	}
};
}  // namespace MecaCell
#endif
