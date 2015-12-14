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
	struct ContactJoint {
		Rotation<Vec> r;      // rotation from node to joint
		Rotation<Vec> delta;  // current rotation
		Vec direction;        // current direction
		Vec target;           // targeted direction
	};

	ordered_pair<Cell *> cells;
	Vec normal;
	std::pair<float_t, float_t> midpoint;
	std::pair<ContactJoint, ContactJoint> fj, tj;  // flex and torsion
	bool adhesionEnabled = true;
	bool frictionEnabled = false;
	float_t damping = 0.5;
	float_t sqradius, adhRadius = 0;
	float_t area, adhArea = 0;
	float_t centersDist, prevCentersDist = 0;
	float_t staticFrictionCoef = 7.0, dynamicFrictionCoef = 5.0;
	float_t adhCoef = 1;
	const float_t adhStrength = 0.1;
	float_t bondMaxL = 0.3;  // max length a surface bond can reach before breaking

	float_t distToMaxL = 1.0;  // at wich distance from contact do the bonds reach their
	                           // maxL (related to the average curvature of
	                           // the membrane around contact surface)
	float_t normalForce = 0.0;

	const float_t restEpsilon =
	    1e-10;  // minimal speed to consider the connected Cells not to be at rest
	bool atRest = false;

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
		// A VERIFIER : là je fais commme si le rayon avait rétréci (midpoint * normal). Mais
		// en fait c'est peut-être pas juste,
		// faut peut être considérer la sphere dans son rayon "au repos" (radius * normal)
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
		auto speed = max(0.0, prevCentersDist - centersDist) / dt;
		auto speed2 = max(0.0, -prevCentersDist + centersDist) / dt;
		const auto pressureDamping = dampingFromRatio(
		    0.5, cells.first->getMass() + cells.second->getMass(), normalForce * 0.5);
		normalForce -= speed2 * pressureDamping;
		cells.first->receiveForce(normalForce, -normal, true);
		cells.second->receiveForce(normalForce, normal, true);
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
				fAdh -= speed * damping;
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
