#ifndef MECACELL_ELASTICCONNECTION_HPP
#define MECACELL_ELASTICCONNECTION_HPP
#include <cmath>
#include <utility>
#include "spring.hpp"
#include "utilities/ordered_pair.hpp"
#include "utilities/utils.h"

namespace MecaCell {
/**
 * @brief An Elastic Connection is a connection between two cells.
 * @tparam Cell a connectable cell class with an elastic body.
 */

template <typename Cell> struct ElasticConnection {
	ordered_pair<Cell *> cells;
	double area = 0;     // area of the contact disk
	double adhArea = 0;  // connected area
	double overlap = 0, adhOverlap = 0;
	double bondBreakDist = 2.0;
	std::pair<double, double>
	    midpoint;        // contact disk's distance to center (viewed from each cell)
	Vector3D direction;  // normalized direction from cell 0 to cell 1
	double prevDist;     // distance btwn the two cells
	double dist;         // distance btwn the two cells
	std::pair<Joint, Joint> flex, tors;
	bool adhesionEnabled = false;
	bool fixedAdhesion = false;  // is this connection indestructible ?

	ElasticConnection(){};
	ElasticConnection(ordered_pair<Cell *> c) : cells(c) { init(); };

	void updateDirection() {
		direction = cells.second->getPosition() - cells.first->getPosition();
		prevDist = dist;
		dist = direction.length();
		if (dist > 0) direction /= dist;
		midpoint = computeMidpoints();
	}

	std::pair<double, double> computeMidpoints() {
		// return the current contact disk's center distance to each cells centers
		if (dist <= Config::DOUBLE_EPSILON) return {0, 0};

		auto biggestCell = cells.first->getBody().getBoundingBoxRadius() >=
		                           cells.second->getBody().getBoundingBoxRadius() ?
		                       cells.first :
		                       cells.second;
		auto smallestCell = biggestCell == cells.first ? cells.second : cells.first;

		double biggestCellMidpoint =
		    0.5 * (dist +
		           (std::pow(biggestCell->getBody().getBoundingBoxRadius(), 2) -
		            std::pow(smallestCell->getBody().getBoundingBoxRadius(), 2)) /
		               dist);
		double smallestCellMidpoint = dist - biggestCellMidpoint;
		if (biggestCell == cells.first)
			return {biggestCellMidpoint, smallestCellMidpoint};
		else
			return {smallestCellMidpoint, biggestCellMidpoint};
	}

	void updateCollisionForces() {
		overlap = max(0.0, (cells.first->getBoundingBoxRadius() +
		                    cells.second->getBoundingBoxRadius()) -
		                       dist);
		const auto &c0 = cells.first->getBody();
		const auto &c1 = cells.first->getBody();
		double invertRadii = 1.0 / (1.0 / cells.first->getBoundingBoxRadius() +
		                            1.0 / cells.second->getBoundingBoxRadius());
		double invertYoung =
		    1.0 / (((1.0 - pow(c0.getPoissonCoef(), 2)) / c0.getYoungModulus()) +
		           ((1.0 - pow(c1.getPoissonCoef(), 2)) / c1.getYoungModulus()));
		double tempterm = sqrt(invertRadii) * pow(overlap, 3.0 / 2.0);
		area = pow(cbrt(invertRadii * tempterm), 2) * M_PI;
		auto F = direction * ((2.0 / 3.0) * invertYoung * tempterm);
		cells.first->getBody().receiveForce(-F);
		cells.second->getBody().receiveForce(F);
	}

	void init() {
		updateDirection();
		prevDist = dist;
	}
	void update(double dt) {
		updateDirection();
		updateCollisionForces();
		if (adhesionEnabled) {
			// updateAdhesionForces(dt);
		}
	}

	void updateAdhesionForces(double dt) {
		// just stronger static frition forces
	}
};
}
#endif
