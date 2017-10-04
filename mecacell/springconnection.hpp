#ifndef MECACELL_SPRINGCONNECTION_HPP
#define MECACELL_SPRINGCONNECTION_HPP
#include <cmath>
#include <utility>
#include "spring.hpp"
#include "utilities/ordered_pair.hpp"
#include "utilities/utils.hpp"

namespace MecaCell {
/**
 * @brief A Spring Connection is a connection between two cells that aims to models both
 * attractive (for adhesion) and repulsive (for collision) forces. It can be seen as
 * composed of 4 dynamically updated mass-spring-damper systems (which can also be seen as
 * a simplified Euler-Bernouilli beam system):
 *  - 2 straight spring (collision + adhesion), called sc, directly attached to the cells
 * centers and whose
 * stiffness is dynamically updated and depends on the level of interpenetration (the
 * deformation of the cells) as well as their mutual attractions (their adhesions).
 *  - 2 rotational springs (AKA joints), flex & tors, ensuring orientation lock when the
 * cells
 * are in contact. These 2 springs can also be used to simulated friction.
 *
 * @tparam Cell a connectable cell class.
 * Required methods for Cell:
 * - Vec getPosition()
 * - Vec getVelocity()
 * - Vec getAngularVelocity()
 * - Basis getOrientation()
 * - Rotation getOrientationRotation()
 * - double getInertia()
 * - void receiveForce(double intensity, Vec direction, bool compressive)
 * - void receiveTorque(Vec acc)
 * An implementaiton of these methods is available in the Orientable and Movable classes.
 */
template <typename Cell> struct SpringConnection {
	double COLLISION_DAMPING_RATIO = 0.5;
	double ADH_DAMPING_RATIO = 1.0;
	double ANG_ADH_COEF = 10.0;
	double ADH_CONSTANT = 1.0;  // factor by which all adhesion forces is multiplied
	double MAX_TS_INCL =
	    0.1;  // max angle before we need to reproject our torsion joint rotation

	ordered_pair<Cell *> cells;
	Spring collision;
	Spring adhesion;
	std::pair<double, double>
	    midpoint;         // contact disk's distance to center (viewed from each cell)
	double sqradius = 0;  // squared radius of the contact disk
	double area = 0, adhArea = 0;  // area of the contact disk
	Vector3D direction;            // normalized direction from cell 0 to cell 1
	double dist;                   // distance btwn the two cells
	std::pair<Joint, Joint> flex, tors;
	bool adhesionEnabled = true, frictionEnabled = false, flexEnabled = false,
	     torsEnabled = false, unbreakable = false;
	double adhCoef = 0.5;

	SpringConnection(){};
	SpringConnection(ordered_pair<Cell *> c) : cells(c) { init(); };

	std::pair<double, double> computeMidpoints(double distanceBtwnCenters) {
		// return the current contact disk's center distance to each cells centers
		if (dist <= Config::DOUBLE_EPSILON) return {0, 0};

		auto biggestCell = cells.first->getBody().getBoundingBoxRadius() >=
		                           cells.second->getBody().getBoundingBoxRadius() ?
		                       cells.first :
		                       cells.second;
		auto smallestCell = biggestCell == cells.first ? cells.second : cells.first;

		double biggestCellMidpoint =
		    0.5 * (distanceBtwnCenters +
		           (std::pow(biggestCell->getBody().getBoundingBoxRadius(), 2) -
		            std::pow(smallestCell->getBody().getBoundingBoxRadius(), 2)) /
		               distanceBtwnCenters);
		double smallestCellMidpoint = distanceBtwnCenters - biggestCellMidpoint;
		if (biggestCell == cells.first)
			return {biggestCellMidpoint, smallestCellMidpoint};
		else
			return {smallestCellMidpoint, biggestCellMidpoint};
	}

	void updateDirection() {
		direction =
		    cells.second->getBody().getPosition() - cells.first->getBody().getPosition();
		dist = direction.length();
		if (dist > 0) direction /= dist;
	}

	void updateCollisionParams() {
		collision.restLength =
		    cells.first->getBoundingBoxRadius() + cells.second->getBoundingBoxRadius();
		collision.k =
		    (cells.first->getBoundingBoxRadius() * cells.first->getBody().getStiffness() +
		     cells.second->getBoundingBoxRadius() * cells.second->getBody().getStiffness()) /
		    collision.restLength;
		collision.c = dampingFromRatio(
		    COLLISION_DAMPING_RATIO,
		    cells.first->getBody().getMass() + cells.second->getBody().getMass(),
		    collision.k);
		midpoint = computeMidpoints(dist);
		sqradius = max(0.0, std::pow(cells.first->getBody().getBoundingBoxRadius(), 2) -
		                        midpoint.first * midpoint.first);
		area = M_PI * sqradius;
	}

	void updateAdhesionParams() {
		if (!unbreakable) {
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
		adhesion.k = ADH_CONSTANT * adhCoef;
		adhesion.c = dampingFromRatio(
		    ADH_DAMPING_RATIO,
		    cells.first->getBody().getMass() + cells.second->getBody().getMass(), adhesion.k);
	}

	void initJoints() {
		auto ortho = direction.ortho();
		// rotations for joints (cell base to connection) =
		// cellBasis -> worldBasis + worldBasis -> connectionBasis
		flex.first.r = cells.first->getBody().getOrientationRotation().inverted() +
		               Vec::getRotation(Basis<Vec>(), Basis<Vec>(direction, ortho));
		flex.second.r = cells.second->getBody().getOrientationRotation().inverted() +
		                Vec::getRotation(Basis<Vec>(), Basis<Vec>(-direction, ortho));
		tors.first.r = flex.first.r;
		tors.second.r = flex.second.r;

		updateFlexParams();
		updateTorsParams();
	}

	void updateTorsParams() {
		tors.first.updateDirection(cells.first->getBody().getOrientation().Y,
		                           cells.first->getBody().getOrientationRotation());
		tors.second.updateDirection(cells.second->getBody().getOrientation().Y,
		                            cells.second->getBody().getOrientationRotation());
		tors.first.k = adhesion.k * area;
		tors.second.k = adhesion.k * area;
		tors.first.c = dampingFromRatio(ADH_DAMPING_RATIO,
		                                cells.first->getBody().getMomentOfInertia() +
		                                    cells.second->getBody().getMomentOfInertia(),
		                                tors.first.k);
		tors.second.c = tors.first.c;
	}

	void updateFlexParams() {
		flex.first.target = direction;
		flex.first.updateDelta();
		flex.second.target = -direction;
		flex.second.updateDelta();
		flex.first.updateDirection(cells.first->getBody().getOrientation().X,
		                           cells.first->getBody().getOrientationRotation());
		flex.second.updateDirection(cells.second->getBody().getOrientation().X,
		                            cells.second->getBody().getOrientationRotation());
		flex.first.k = adhesion.k * area * ANG_ADH_COEF;
		flex.second.k = adhesion.k * area * ANG_ADH_COEF;
		flex.first.c = dampingFromRatio(ADH_DAMPING_RATIO,
		                                cells.first->getBody().getMomentOfInertia() +
		                                    cells.second->getBody().getMomentOfInertia(),
		                                flex.first.k);
		flex.second.c = flex.first.c;
	}

	void init() {
		updateDirection();
		collision.prevLength = dist;
		collision.length = dist;
		updateCollisionParams();
		if (adhesionEnabled) {
			adhesion.length = dist;
			adhesion.prevLength = dist;
			adhesion.restLength = 0;
			initJoints();
		}
	}

	template <int n> void updateJointsForces(double dt) {
		Joint &torsNode = n == 0 ? tors.first : tors.second;
		Joint &torsOther = n == 0 ? tors.second : tors.first;
		Joint &flexNode = n == 0 ? flex.first : flex.second;
		const auto &cell = cells.template get<n>();
		const auto &other = cells.template get < n == 0 ? 1 : 0 > ();
		const double sign = n == 0 ? 1 : -1;

		if (flexNode.maxTetaAutoCorrect &&
		    flexNode.delta.teta > flexNode.maxTeta) {  // if we passed flex break angle
			double dif = flexNode.delta.teta - flexNode.maxTeta;
			flexNode.r = flexNode.r + Rotation<Vec>(flexNode.delta.n, dif);
			flexNode.direction =
			    flexNode.direction.rotated(Rotation<Vec>(flexNode.delta.n, dif));
			flexNode.r = cell->getBody().getOrientationRotation().inverted() +
			             Vec::getRotation(Basis<Vec>(), Basis<Vec>(flexNode.direction,
			                                                       flexNode.direction.ortho()));
		}
		flexNode.delta.n.normalize();
		double torque =
		    flexNode.k * flexNode.delta.teta +
		    flexNode.c * ((flexNode.delta.teta - flexNode.prevDelta.teta) / dt);  // -kx - cv
		Vec vFlex = flexNode.delta.n * torque;                                    // torque
		Vec ortho = direction.ortho(flexNode.delta.n).normalized();  // force direction
		Vec force = sign * ortho * torque / dist;

		cell->getBody().receiveForce(-force);
		other->getBody().receiveForce(force);

		cell->getBody().receiveTorque(vFlex);
		flexNode.prevDelta = flexNode.delta;
		if (torsEnabled) {
			// updating torsion joint (needs to stay perp to direction)
			double scalar = torsNode.direction.dot(direction);
			// if the angle between our torsion spring and direction is too far from 90°,
			// we reproject & recompute it
			if (abs(scalar) > MAX_TS_INCL) {
				torsNode.r = cell->getBody().getOrientationRotation().inverted() +
				             Vec::getRotation(Basis<Vec>(Vec(1, 0, 0), Vec(0, 1, 0)),
				                              Basis<Vec>(direction, torsNode.direction));
			} else
				torsNode.direction = torsNode.direction.normalized() - scalar * direction;
			// updating targets
			torsNode.target =
			    torsOther.direction;  // we want torsion springs to stay aligned with each other
			torsNode.updateDelta();
			// torsion torque
			double torsionTorque = torsNode.k * torsNode.delta.teta;  // - torsNode.c *
			// cell->getAngularVelocity().dot(torsNode.delta.teta.n)
			Vec vTorsion = torsionTorque * torsNode.delta.n;
			cell->getBody().receiveTorque(vTorsion);
		}
	}

	void update(double dt) {
		updateDirection();
		// updating collision and adhesion springs
		collision.updateLength(dist);
		updateCollisionParams();
		collision.applyForce(cells.first->getBody(), cells.second->getBody(), direction, dt);
		if (adhesionEnabled) {
			adhesion.updateLength(dist);
			updateAdhesionParams();
			adhesion.applyForce(cells.first->getBody(), cells.second->getBody(), direction, dt);
			if (flexEnabled) updateFlexParams();
			if (torsEnabled) updateTorsParams();
			if (flexEnabled || torsEnabled) {
				updateJointsForces<0>(dt);
				updateJointsForces<1>(dt);
			}
		}
	}
};
}  // namespace MecaCell
#endif
