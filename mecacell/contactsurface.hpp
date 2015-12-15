#ifndef CONTACTSURFACE_HPP
#define CONTACTSURFACE_HPP
#include "tools.h"
#include <utility>
#include <cmath>

#define TRACE 1

namespace MecaCell {
using std::isnan;
#define DBG DEBUG(contactSurface)
template <typename Cell> struct ContactSurface {
	struct SimpleJoint {
		float_t maxTeta = M_PI / 30.0;  // maximum angle
		Rotation<Vec> r;                // rotation from node to joint
		Rotation<Vec> delta;            // current rotation
		Rotation<Vec> prevDelta;
		float_t elongation = 0.0, prevElongation = 0.0;
		Vec direction;  // current direction
		Vec target;     // targeted direction
		// current direction is computed using a reference Vector v rotated with rotation rot
		void updateDirection(const Vec &v, const Rotation<Vec> &rot) {
			direction = v.rotated(r.rotated(rot));
		}
		void updateDelta() { delta = Vec::getRotation(direction, target); }
	};

	void initJoints() {
		Vec ortho = normal.ortho();
		// rotations for joints (cell base to connection) =
		// cellBasis -> worldBasis + worldBasis -> connectionBasis
		joints.first.r = cells.first->getOrientationRotation().inverted() +
		                 Vec::getRotation(Basis<Vec>(), Basis<Vec>(normal, ortho));
		joints.second.r = cells.second->getOrientationRotation().inverted() +
		                  Vec::getRotation(Basis<Vec>(), Basis<Vec>(-normal, ortho));
	}

	void updateJointsDirection() {
		// joint's current direction
		joints.first.updateDirection(cells.first->getOrientation().X,
		                             cells.first->getOrientationRotation());
		joints.second.updateDirection(cells.second->getOrientation().X,
		                              cells.second->getOrientationRotation());
	}

	template <int n> void updateJoints(float_t) {
		constexpr int otherN = n == 0 ? 1 : 0;
		SimpleJoint &fjNode = std::get<n>(joints);
		const auto &node = cells.template get<n>();
		const auto &other = cells.template get<otherN>();
		const float_t sign = n == 0 ? 1 : -1;
		const float_t midp = std::get<n>(midpoint);
		const float_t othermidp = std::get<otherN>(midpoint);

		fjNode.target = normal * sign;
		fjNode.updateDelta();
		fjNode.maxTeta = midp > 0 ? atan(bondMaxL / midp) : 0.0;
		if (fjNode.delta.teta > fjNode.maxTeta) {  // if we passed flex break angle
			float dif = fjNode.delta.teta - fjNode.maxTeta;
			if (dif > 5.0 * fjNode.maxTeta) {
				initJoints();
			} else {
				fjNode.r = fjNode.r + Rotation<Vec>(fjNode.delta.n, dif);
				fjNode.direction = fjNode.direction.rotated(Rotation<Vec>(fjNode.delta.n, dif));
				fjNode.r = node->getOrientationRotation().inverted() +
				           Vec::getRotation(Basis<Vec>(), Basis<Vec>(fjNode.direction,
				                                                     fjNode.direction.ortho()));
			}
		}

		// flex torque and force
		float_t d = centersDist;
		float_t torque =
		    200.0 * adhArea * adhCoef * adhStrength * fjNode.delta.teta +
		    damping * 100.0 * (fjNode.delta.teta - fjNode.prevDelta.teta);  // -kx - cv
		Vec vFlex = fjNode.delta.n * torque;                                // torque
		Vec ortho = normal.ortho(fjNode.delta.n).normalized();              // force direction
		Vec force = sign * ortho * torque / d;

		node->receiveForce(-force);
		other->receiveForce(force);

		node->receiveTorque(vFlex);
		fjNode.prevDelta = fjNode.delta;
		// Vec ortho = sign * normal.ortho(fjNode.delta.n).normalized();  // force direction
		//// force's point of application is at the center of the current surface
		//// (normal*midpoint)
		// fjNode.prevElongation = fjNode.elongation;
		// fjNode.elongation = tan(fjNode.delta.teta) * midp;
		// if (fjNode.elongation > bondMaxL) {
		// DBG << RED << "elongation = " << fjNode.elongation << endl;
		// DBG << " max Teta = " << fjNode.maxTeta << ", max L= " << bondMaxL << endl;
		// DBG << " delta teta = " << fjNode.delta.teta << endl;
		//}
		//// auto speed = max(0.0, (fjNode.elongation - fjNode.prevElongation) / dt);
		// auto speed = 0;  // tan((fjNode.delta.teta - fjNode.prevDelta.teta) / dt) * midp;
		// const auto rollDamping =
		// dampingFromRatio(damping, cells.first->getMass() + cells.second->getMass(),
		// adhArea * adhStrength * adhCoef);
		// auto halfForce = 0.5 * (fjNode.elongation * adhArea * adhCoef * adhStrength * ortho
		// -
		// speed * rollDamping);
		// node->receiveTorque((midp * normal * sign).cross(-halfForce));
		// node->receiveForce(-halfForce);
		//// other->receiveTorque((othermidp * normal * sign).cross(halfForce));
		//// other->receiveForce(halfForce);
		// fjNode.prevDelta = fjNode.delta;
	}

	ordered_pair<Cell *> cells;
	Vec normal;
	std::pair<float_t, float_t> midpoint;
	std::pair<SimpleJoint, SimpleJoint> joints;
	bool adhesionEnabled = true;
	bool frictionEnabled = false;
	float_t damping = 0.3;
	float_t sqradius, adhRadius = 0;
	float_t area, adhArea = 0;
	float_t centersDist, prevCentersDist = 0;
	float_t staticFrictionCoef = 7.0, dynamicFrictionCoef = 5.0;
	float_t adhCoef = 1;
	const float_t adhStrength = 0.01;  // base strength
	float_t bondMaxL = 1.5;    // max length a surface bond can reach before breaking
	float_t distToMaxL = 1.0;  // at wich distance from contact do the bonds reach their
	                           // maxL (related to the average curvature of
	                           // the membrane around contact surface)
	float_t normalForce = 0.0;
	const float_t restEpsilon =
	    1e-10;  // minimal speed to consider the connected Cells not to be at rest
	bool atRest = false;

	double projCentersDist = 0.0;
	double prevProjCentersDist = 0.0;

	/*void updateICB(float_t dt) {*/
	//// we update the orientation of our contact surface in the cells basis
	// DBG << YELLOW << "<--- Updating ICB ---> " << endl;
	// auto &i0 = icb.first;
	// i0.currentBasis = cells.first->getOrientation().rotated(
	// i0.r.rotated(cells.first->getOrientationRotation()));
	// auto &i1 = icb.second;
	// i1.currentBasis = cells.second->getOrientation().rotated(
	// i1.r.rotated(cells.second->getOrientationRotation()));
	//// now we can compute the distance between the two centers
	//// projected onto the current plane

	// float_t nr0 = normal.dot(i0.currentBasis.X);
	// float_t nr1 = -normal.dot(i1.currentBasis.X);
	// if (nr0 > 0 && nr1 > 0) {
	// auto p0 = cells.first->getPosition();
	// auto o0 = p0 + normal * midpoint.first;
	// auto proj0 = p0 + (normal.dot(o0 - p0) / nr0) * i0.currentBasis.X;
	// auto p1 = cells.second->getPosition();
	// auto o1 = p1 - normal * midpoint.second;
	// auto proj1 = p1 + (-normal.dot(o1 - p1) / nr1) * i1.currentBasis.X;
	// auto c0c1 = proj1 - proj0;
	// prevProjCentersDist = projCentersDist;
	// projCentersDist = c0c1.length();
	// auto c0c1dir = (c0c1 / projCentersDist);
	// Vec retForce;
	// auto speed = max(0.0, (prevProjCentersDist - projCentersDist) / dt);
	// const auto damp =
	// dampingFromRatio(damping, cells.first->getMass() + cells.second->getMass(),
	// adhArea * adhCoef * adhStrength);
	// if (projCentersDist > bondMaxL) {
	// retForce =
	//(bondMaxL * adhCoef * adhStrength * adhArea - speed * damp) * 0.5 * c0c1dir;
	//// there is going to be some springs breaking
	// auto halfLengthDiff = 0.5 * (projCentersDist - bondMaxL);
	// auto newProj0 = proj0 + c0c1dir * halfLengthDiff;
	// auto newProj1 = proj1 - c0c1dir * halfLengthDiff;
	//// now we update the basis with the new projections
	// DBG << "reprojecting" << endl;
	//// i0.r = i0.r + Vec::getRotation(i0.currentBasis.X, (newProj0 -
	//// p0).normalized());
	// auto npp0 = (newProj0 - p0).normalized();
	// i0.r = cells.first->getOrientationRotation().inverted() +
	// Vec::getRotation(Basis<Vec>(), Basis<Vec>(npp0, npp0.ortho()));
	//// i1.r = i1.r + Vec::getRotation((newProj1 - p1).normalized(),
	//// i1.currentBasis.X);
	//// and we apply the force
	//} else {
	// retForce = (projCentersDist * adhCoef * adhStrength * adhArea - speed * damp) *
	// 0.5 * c0c1dir;
	//}

	// DBG << "retForce = " << retForce << endl;
	// cells.first->receiveTorque((proj0 - p0).cross(retForce));
	// cells.first->receiveForce(retForce);
	// cells.second->receiveTorque((proj1 - p1).cross(-retForce));
	// cells.second->receiveForce(-retForce);
	//} else {
	// i0.r = i0.r + Vec::getRotation(i0.currentBasis.X, normal);
	// i1.r = i1.r + Vec::getRotation(i1.currentBasis.X, -normal);
	//}
	/*}*/

	bool nanIsInTheAir() {
		return isnan_v(cells.first->getPosition()) || isnan_v(cells.first->getForce()) ||
		       isnan_v(cells.first->getTorque()) || isnan_v(cells.second->getPosition()) ||
		       isnan_v(cells.second->getForce()) || isnan_v(cells.second->getTorque()) ||
		       isnan(sqradius) || isnan(adhRadius) || isnan(area) || isnan(adhArea) ||
		       isnan(centersDist) || isnan(prevCentersDist) || isnan(normalForce) ||
		       isnan(midpoint.first) || isnan(midpoint.second) || isnan_v(normal);
	}

	ContactSurface(){};
	ContactSurface(ordered_pair<Cell *> c) : cells(c) {
		Vec AB = cells.second->getPosition() - cells.first->getPosition();
		centersDist = AB.length();
		normal = centersDist >= 0 ? AB / centersDist : Vec(1, 0, 0);
		midpoint = make_pair(centersDist * 0.5, centersDist * 0.5);
		sqradius = pow(cells.first->getMembrane().getDynamicRadius(), 2) -
		           midpoint.first * midpoint.first;
		area = sqradius * M_PI;
		initJoints();
	}

	std::string toString() {
		std::stringstream ss;
		ss << "Contact Surface : " << endl;
		ss << cells.first->toString() << endl;
		ss << cells.second->toString() << endl;
		ss << " normal = " << normal << endl;
		ss << " area = " << area << ", adhArea = " << adhArea << endl;
		ss << " sqradius = " << sqradius << endl;
		ss << " normalForce = " << normalForce << endl;
		ss << " midpoint = [" << midpoint.first << " ; " << midpoint.second << "]" << endl;
		ss << " atRest = " << atRest << endl;
		return ss.str();
	}

	void applyFriction() {
		if (frictionEnabled) {
			if (nanIsInTheAir()) {
				DBG << " ---------------------------------" << endl;
				DBG << "beginning : " << endl;
				DBG << toString() << endl;
				throw(0);
			}
			if (atRest) {  // static friction
				float_t staticF = staticFrictionCoef * normalForce * 2.0;
				Vec torqueCrossed0 = cells.first->getTorque().cross(midpoint.first * normal);
				Vec forceFromTorqueAtCenter0 = torqueCrossed0.sqlength() != 0.0 ?
				                                   torqueCrossed0.normalized() *
				                                       cells.first->getTorque().length() /
				                                       abs(midpoint.first) :
				                                   Vec::zero();
				Vec tangentialForceFromTorqueAtCenter0 =
				    forceFromTorqueAtCenter0 - forceFromTorqueAtCenter0.dot(normal) * normal;
				Vec torqueCrossed1 = cells.second->getTorque().cross(-midpoint.first * normal);
				Vec forceFromTorqueAtCenter1 = torqueCrossed1.sqlength() != 0.0 ?
				                                   torqueCrossed1.normalized() *
				                                       cells.second->getTorque().length() /
				                                       abs(midpoint.second) :
				                                   Vec::zero();
				Vec tangentialForceFromTorqueAtCenter1 =
				    forceFromTorqueAtCenter1 - forceFromTorqueAtCenter1.dot(normal) * normal;
				Vec tangentialForce =
				    (cells.first->getForce() - cells.first->getForce().dot(normal) * normal +
				     tangentialForceFromTorqueAtCenter0) -
				    (cells.second->getForce() - cells.second->getForce().dot(normal) * normal +
				     tangentialForceFromTorqueAtCenter1);
				if (tangentialForce.sqlength() > staticF * staticF) {
					atRest = false;
				} else {
					// this force is applied at the center of the contact surface.
					auto halfTF = tangentialForce * 0.5;
					cells.first->receiveTorque((midpoint.first * normal).cross(halfTF));
					cells.first->receiveForce(-halfTF);
					cells.second->receiveTorque((-midpoint.second * normal).cross(-halfTF));
					cells.second->receiveForce(halfTF);
					if (nanIsInTheAir()) {
						DBG << "staticF = " << staticF << endl;
						DBG << " cells.first->getTorque = " << cells.first->getTorque() << endl;
						DBG << "forceFromTorqueAtCenter0 = " << forceFromTorqueAtCenter0 << endl;
						DBG << "tangentialForceFromTorqueAtCenter0 = "
						    << tangentialForceFromTorqueAtCenter0 << endl;
						DBG << "forceFromTorqueAtCenter1 = " << forceFromTorqueAtCenter1 << endl;
						DBG << "tangentialForceFromTorqueAtCenter1 = "
						    << tangentialForceFromTorqueAtCenter1 << endl;
						DBG << "tangentialForce = " << tangentialForce << endl;
					}
				}
			}
			if (!atRest) {  // dynamic friction
				Vec tangentialVelocity01 =
				    (cells.first->getVelocity() -
				     cells.first->getVelocity().dot(normal) * normal +
				     cells.first->getAngularVelocity().cross(
				         cells.first->getMembrane().getDeducedRadius() * normal)) -
				    (cells.second->getVelocity() -
				     cells.second->getVelocity().dot(normal) * normal +
				     cells.second->getAngularVelocity().cross(
				         -cells.second->getMembrane().getDeducedRadius() * normal));
				if (tangentialVelocity01.sqlength() < restEpsilon) {
					atRest = true;
				} else {
					auto Fd0 =
					    -dynamicFrictionCoef * normalForce * tangentialVelocity01.normalized();
					auto Fd1 = -Fd0;
					// this force is applied at the center of the contact surface.
					cells.first->receiveTorque((midpoint.first * normal).cross(Fd0));
					cells.first->receiveForce(Fd0);
					cells.second->receiveTorque((-midpoint.second * normal).cross(Fd1));
					cells.second->receiveForce(Fd1);
					if (nanIsInTheAir()) {
						DBG << "c0 velocity = " << cells.first->getVelocity()
						    << ", c0 angular vel = " << cells.first->getAngularVelocity() << endl;
						DBG << "c1 velocity = " << cells.second->getVelocity()
						    << ", c1 angular vel = " << cells.second->getAngularVelocity() << endl;
						DBG << "tangentialVelocity = " << tangentialVelocity01 << endl;
						DBG << " c0 deduced radius = "
						    << cells.first->getMembrane().getDeducedRadius()
						    << ", c1 deduced radius = "
						    << cells.second->getMembrane().getDeducedRadius() << endl;
						DBG << "Fd0 = " << Fd0 << endl;
						DBG << "tangentialVelocity01= " << tangentialVelocity01 << endl;
						DBG << toString() << endl;
					}
				}
			}
		}
	}

	void applyPressureAndAdhesionForces(float_t dt) {
		normalForce = 0.5 * (max(0.0, cells.first->getMembrane().pressure * area) +
		                     max(0.0, cells.second->getMembrane().pressure * area));
		auto speed = (prevCentersDist - centersDist) / dt;
		const auto pressureDamping = dampingFromRatio(
		    damping, cells.first->getMass() + cells.second->getMass(), normalForce * 0.5);
		normalForce -= max(0.0, -speed) * pressureDamping;
		cells.first->receiveForce(normalForce, -normal, true);
		cells.second->receiveForce(normalForce, normal, true);
		speed = max(0.0, speed);

		updateJointsDirection();
		updateJoints<0>(dt);
		updateJoints<1>(dt);

#if TRACE > 2
		DBG << CYAN << "|--------------------------------| " << endl;
		DBG << CYAN << cells.first->id << "<-> " << cells.second->id
		    << " (d = " << (cells.first->getPosition() - cells.second->getPosition()).length()
		    << ")" << endl;
		DBG << "Pressure force = " << normalForce << endl;
#endif
		// adhesion : normal to contact
		if (adhesionEnabled) {
#if TRACE > 2
			DBG << YELLOW << "<--- Adhesion Forces ---> " << endl;
#endif
			if (area < adhArea) {
#if TRACE > 2
				DBG << MAGENTA << " cells were teared appart !" << endl;
#endif
				// we removed part of the surface
				float_t radius = sqrt(sqradius);
				float_t deltaRadius = adhRadius - radius;
				float_t truncatedConeVol = 0;  // volume of the part where the bonds broke
				float_t maxCylinderVol = 0;    // volume of the part where the bonds broke
#if TRACE > 2
				DBG << " deltaRadius = " << deltaRadius << endl;
#endif
				auto prevAdhArea = adhArea;
				if (deltaRadius > distToMaxL) {
					float_t tcRadius = distToMaxL;
					truncatedConeVol = (tcRadius * tcRadius + sqradius) * bondMaxL * M_PI / 3.0;
					maxCylinderVol = bondMaxL * (adhArea - area);
					// some bonds were broken :
					auto prevAdhRadius = adhArea;
					adhRadius = radius + distToMaxL;
					adhArea = adhRadius * adhRadius * M_PI;
#if TRACE > 2
					DBG << RED << " deltaRadiux > distToMaxL (" << distToMaxL << ")" << endl;
					DBG << " prevAdhRadius = " << prevAdhRadius << ", new = " << adhRadius << endl;
#endif
				} else {
					truncatedConeVol = (adhRadius * adhRadius + sqradius) *
					                   (bondMaxL * deltaRadius / distToMaxL) * M_PI / 3.0;
#if TRACE > 2
					DBG << GREEN << " deltaRadiux < distToMaxL (" << distToMaxL << ")" << endl;
#endif
				}
				float_t totalVol = truncatedConeVol + maxCylinderVol;
				float_t fAdh = 0.5 * adhCoef * adhStrength * totalVol;
				fAdh -= speed * dampingFromRatio(damping,
				                                 cells.first->getMass() + cells.second->getMass(),
				                                 adhStrength * adhCoef * (prevAdhArea - area));
				cells.first->receiveForce(fAdh, normal, false);
				cells.second->receiveForce(fAdh, -normal, false);
#if TRACE > 2
				DBG << " truncatedConeVol = " << truncatedConeVol
				    << " ; maxCylinderVol = " << maxCylinderVol << endl;
				DBG << MAGENTA << " fAdh = " << fAdh << endl;
#endif
			} else {
				// more adhesions !
				auto prevAdhRadius = adhRadius;
				adhRadius = sqrt(sqradius);
				adhArea = area;
#if TRACE > 2
				DBG << BLUE << " more adhesions ! : " << RESET
				    << " prevAdhRadius = " << prevAdhRadius << ", new = " << adhRadius << endl;
#endif
			}
		}
	}

	void update() {
		Vec AB = cells.second->getPosition() - cells.first->getPosition();
		float_t sqL = AB.sqlength();
		float_t sqMaxD = pow(cells.first->getMembrane().getDynamicRadius() +
		                         cells.second->getMembrane().getDynamicRadius(),
		                     2);
		if (sqL > sqMaxD) {
			area = 0;
		} else {
			prevCentersDist = centersDist;
			centersDist = sqrt(sqL);
			normal = centersDist != 0 ? AB / centersDist : Vec(1, 0, 0);
			// TODO ; precise midpoint
			midpoint = make_pair(centersDist * 0.5, centersDist * 0.5);
			sqradius = pow(cells.first->getMembrane().getDynamicRadius(), 2) -
			           midpoint.first * midpoint.first;
			area = sqradius * M_PI;
		}
	}
};
}
#endif
