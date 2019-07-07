#ifndef MECACELL_ELASTICCONNECTION_HPP
#define MECACELL_ELASTICCONNECTION_HPP
#include <cmath>
#include <utility>
#include "spring.hpp"
#include "utilities/ordered_pair.hpp"
#include "utilities/utils.hpp"

namespace MecaCell {
/**
 * @brief An Elastic Connection is a connection between two cells.
 * @tparam Cell a connectable cell class with an elastic body.
 */

template <typename Cell> struct ElasticConnection {
	ordered_pair<Cell *> cells;
	num_t area = 0;     // area of the contact disk
	num_t adhArea = 0;  // connected area
	num_t overlap = 0, adhOverlap = 0;
	num_t bondBreakDist = 2.0;
	std::pair<num_t, num_t>
	    midpoint;        // contact disk's distance to center (viewed from each cell)
	Vector3D direction;  // normalized direction from cell 0 to cell 1
	num_t prevDist;     // distance btwn the two cells
	num_t dist;         // distance btwn the two cells
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

	std::pair<num_t, num_t> computeMidpoints() {
		// return the current contact disk's center distance to each cells centers
		if (dist <= Config::DOUBLE_EPSILON) return {0, 0};

		auto biggestCell = cells.first->getBody().getBoundingBoxRadius() >=
		                           cells.second->getBody().getBoundingBoxRadius() ?
		                       cells.first :
		                       cells.second;
		auto smallestCell = biggestCell == cells.first ? cells.second : cells.first;

		num_t biggestCellMidpoint =
		    0.5 * (dist +
		           (std::pow(biggestCell->getBody().getBoundingBoxRadius(), 2) -
		            std::pow(smallestCell->getBody().getBoundingBoxRadius(), 2)) /
		               dist);
		num_t smallestCellMidpoint = dist - biggestCellMidpoint;
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
		num_t invertRadii = 1.0 / (1.0 / cells.first->getBoundingBoxRadius() +
		                            1.0 / cells.second->getBoundingBoxRadius());
		num_t invertYoung =
		    1.0 / (((1.0 - pow(c0.getPoissonCoef(), 2)) / c0.getYoungModulus()) +
		           ((1.0 - pow(c1.getPoissonCoef(), 2)) / c1.getYoungModulus()));
		num_t tempterm = sqrt(invertRadii) * pow(overlap, 3.0 / 2.0);
		area = pow(cbrt(invertRadii * tempterm), 2) * M_PI;
		auto F = direction * ((2.0 / 3.0) * invertYoung * tempterm);
		cells.first->getBody().receiveForce(-F);
		cells.second->getBody().receiveForce(F);
	}

	void updateAdhesionForces(num_t) {
		/*
		           dir
		          x---->

		       *  *       *  *
		    *  pf1t2 * *  pf2t1 *
		   *  <--x    *    x-->  *
		   *          *          *
		    *        * *        *
		       *  *       *  *
		       c1          c2

		 */

		//const num_t adhCoef = 0.1;
		//num_t resistiveForce = area * adhCoef;

		// first we compute the force's component parrallel to the collision axis
		// and only the forces trying to separate the cells
		num_t parallelForce1to2 =
		    std::max(-cells.first->getBody().getForce().dot(direction), 0.0);
		num_t parallelForce2to1 =
		    std::max(cells.second->getBody().getForce().dot(direction), 0.0);
		num_t sum = parallelForce2to1 + parallelForce1to2;
		logger<DBG>("pf2t1 = ", parallelForce2to1);
		logger<DBG>("pf1t2 = ", parallelForce1to2);
		cells.first->getBody().receiveForce(parallelForce2to1 * direction);
		cells.second->getBody().receiveForce(-parallelForce1to2 * direction);
		logger<DBG>("sum = ", sum);
	}

	void init() {
		updateDirection();
		prevDist = dist;
	}
	void update(num_t dt) {
		updateDirection();
		updateCollisionForces();
		if (adhesionEnabled) {
			updateAdhesionForces(dt);
		}
	}
};
}
#endif
