#ifndef CONNECTION_H
#define CONNECTION_H
#include "tools.h"

#define MAX_TS_INCL 0.1 // max angle before we need to reproject our torsion joint rotation

namespace MecaCell {
////////////////////////////////////////////////////////////////////
//                SPRING STRUCTURE
////////////////////////////////////////////////////////////////////
// This is just a classic "linear" spring
struct Spring {
	double k = 1.0;      // stiffness
	double c = 1.0;      // damp coef
	double l = 1.0;      // rest length
	double length = 1.0; // current length
	double prevLength = 1.0;
	double minLengthRatio = 0.5; // max compression
	Vec direction;               // current direction from node 0 to node 1

	Spring(){};
	Spring(const double &K, const double &C, const double &L) : k(K), c(C), l(L), length(L){};

	void updateLengthDirection(const Vec &p0, const Vec &p1) {
		direction = p1 - p0;
		length = direction.length();
		direction /= length;
	}
};

////////////////////////////////////////////////////////////////////
//                       JOINT STRUCTURE
////////////////////////////////////////////////////////////////////
// flexible joint. Can be used for flexure (torque + force) or torsion (torque only)
struct Joint {
	double k = 1.0;        // angular stiffness
	double c = 1.0;        // damp
	double maxTeta = M_PI; // maximum angle
	Rotation<Vec> r;       // rotation from node to joint
	Rotation<Vec> delta;   // current rotation
	Rotation<Vec> prevDelta;
	Vec direction; // current direction
	Vec target;    // targeted direction
	Joint(){};

	Joint(const double &K, const double &C, const double &MTETA) : k(K), c(C), maxTeta(MTETA) {}

	// current direction is computed using a reference Vector v rotated with rotation rot
	void updateDirection(const Vec &v, const Rotation<Vec> &rot) { direction = v.rotated(r.rotated(rot)); }
	void updateDelta() { delta = Vec::getRotation(direction, target); }
};

////////////////////////////////////////////////////////////////////
//                      CONNECTION CLASS
////////////////////////////////////////////////////////////////////
// Connects 2 "connectable" nodes with 1 "classic" spring,
// 1 flexure and 1 torsion joint per node. By default those 3 types of
// connections are enabled but one can easily choose to use only a subset
// by setting scEnabled, fjEnabled and tjEnabled accordingly.
// A connectable node can directly inherit from Connectable or be a complete
// separate implementation.
// Required methods for a connectable class are:
// - Vec getPosition()
// - Vec getVelocity()
// - Vec getAngularVelocity()
// - Basis getOrientation()
// - Rotation getOrientationRotation()
// - double getInertia()
// - void receiveForce(double intensity, Vec direction, bool compressive)
// - void receiveTorque(Vec acc)
template <typename N0, typename N1 = N0> class Connection {
private:
	bool scEnabled = true, fjEnabled = true, tjEnabled = false;
	pair<N0, N1> connected;    // the two connected nodes
	Spring sc;                 // basic spring
	pair<Joint, Joint> fj, tj; // flexure and torsion joints (1 per node)

public:
	/**********************************************
	 *               CONSTRUCTOR
	 **********************************************/
	Connection(const pair<N0, N1> &n, const Spring &S)
	    : connected{n}, sc(S), fjEnabled(false), tjEnabled(false) {
		initS();
	}
	Connection(const pair<N0, N1> &n, const pair<Joint, Joint> &FJ, const pair<Joint, Joint> &TJ)
	    : connected{n}, fj(FJ), tj(TJ), scEnabled(false) {
		initS();
		initFJ();
	}
	Connection(const pair<N0, N1> &n, const Spring &SC, const pair<Joint, Joint> &FJ,
	           const pair<Joint, Joint> &TJ)
	    : connected{n}, sc(SC), fj(FJ), tj(TJ) {
		initS();
		initFJ();
	}

	void initS() {
		sc.updateLengthDirection(ptr(connected.first)->getPosition(), ptr(connected.second)->getPosition());
		sc.prevLength = sc.length;
	}
	void initFJ() {
		Vec ortho = sc.direction.ortho();
		// rotations for joints (cell base to connection) =
		// cellBasis -> worldBasis + worldBasis -> connectionBasis
		fj.first.r = ptr(connected.first)->getOrientationRotation().inverted() +
		             Vec::getRotation(Basis<Vec>(), Basis<Vec>(sc.direction, ortho));
		fj.second.r = ptr(connected.second)->getOrientationRotation().inverted() +
		              Vec::getRotation(Basis<Vec>(), Basis<Vec>(-sc.direction, ortho));
		tj.first.r = fj.first.r;
		tj.second.r = fj.second.r;

		// joint current direction
		fj.first.updateDirection(ptr(connected.first)->getOrientation().X,
		                         ptr(connected.first)->getOrientationRotation());
		fj.second.updateDirection(ptr(connected.second)->getOrientation().X,
		                          ptr(connected.second)->getOrientationRotation());
		tj.first.updateDirection(ptr(connected.first)->getOrientation().Y,
		                         ptr(connected.first)->getOrientationRotation());
		tj.second.updateDirection(ptr(connected.second)->getOrientation().Y,
		                          ptr(connected.second)->getOrientationRotation());
	}
	/**********************************************
	 *                GET & SET
	 **********************************************/
	Spring &getSc() { return sc; }
	pair<Joint, Joint> &getFlex() { return fj; }
	pair<Joint, Joint> &getTorsion() { return tj; }
	N0 &getNode0() { return connected.first; }
	N1 &getNode1() { return connected.second; }
	float getLength() { return sc.length; }
	void setBaseLength(const double d) { sc.l = d; }
	Vec getDirection() { return sc.direction; }
	template <typename R, typename T> R &getOtherNode(const T &n) {
		return n == connected.first ? connected.second : connected.first;
	}

	/**********************************************
	 *              UPDATES
	 **********************************************/
	void updateLengthDirection() {
		sc.updateLengthDirection(ptr(connected.first)->getPosition(), ptr(connected.second)->getPosition());
	}
	void computeForces(double dt) {
		// BASIC SPRING
		if (scEnabled) {
			sc.updateLengthDirection(ptr(connected.first)->getPosition(), ptr(connected.second)->getPosition());
			double x = sc.length - sc.l; // actual compression / elongation
			double minlength = sc.minLengthRatio * sc.l;
			if (sc.length < minlength) {
				double d = minlength - sc.length;
				Vec component0 = ptr(connected.first)->getVelocity().dot(sc.direction) * sc.direction;
				Vec tangent0 = ptr(connected.first)->getVelocity() - component0;
				Vec component1 = ptr(connected.second)->getVelocity().dot(sc.direction) * sc.direction;
				Vec tangent1 = ptr(connected.second)->getVelocity() - component1;
				ptr(connected.first)->setPosition(ptr(connected.first)->getPosition() - sc.direction * d / 2.0);
				ptr(connected.second)->setPosition(ptr(connected.second)->getPosition() + sc.direction * d / 2.0);
				ptr(connected.first)->setVelocity(tangent0 + component1);
				ptr(connected.second)->setVelocity(tangent1 + component0);
				sc.length = minlength;
			}
			bool compression = x < 0;
			double v = sc.length - sc.prevLength;
			double f = (-sc.k * x - sc.c * v / dt) / 2.0;
			ptr(connected.first)->receiveForce(f, -sc.direction, compression);
			ptr(connected.second)->receiveForce(f, sc.direction, compression);
			sc.prevLength = sc.length;
		}
		// update directions of both flex and tosion springs
		if (fjEnabled) {
			fj.first.updateDirection(ptr(connected.first)->getOrientation().X,
			                         ptr(connected.first)->getOrientationRotation());
			fj.second.updateDirection(ptr(connected.second)->getOrientation().X,
			                          ptr(connected.second)->getOrientationRotation());
		}
		if (tjEnabled) {
			tj.first.updateDirection(ptr(connected.first)->getOrientation().Y,
			                         ptr(connected.first)->getOrientationRotation());
			tj.second.updateDirection(ptr(connected.second)->getOrientation().Y,
			                          ptr(connected.second)->getOrientationRotation());
		}
		if (tjEnabled || fjEnabled) {
			updateFT<0>();
			updateFT<1>();
		}
	}
	template <int n> void updateFT() {
		Joint &tjNode = n == 0 ? tj.first : tj.second;
		Joint &tjOther = n == 0 ? tj.second : tj.first;
		Joint &fjNode = n == 0 ? fj.first : fj.second;
		const auto &node = ptr(get<n>(connected));
		const auto &other = ptr(get < n == 0 ? 1 : 0 > (connected));
		const double sign = n == 0 ? 1 : -1;

		if (fjEnabled) {
			fjNode.target = sc.direction * sign;
			fjNode.updateDelta();
			if (fjNode.delta.teta > fjNode.maxTeta) { // if we passed flex break angle
				float dif = fjNode.delta.teta - fjNode.maxTeta;
				fjNode.r = fjNode.r + Rotation<Vec>(fjNode.delta.n, dif);
				fjNode.direction = fjNode.direction.rotated(Rotation<Vec>(fjNode.delta.n, dif));
				fjNode.r = node->getOrientationRotation().inverted() +
				           Vec::getRotation(Basis<Vec>(), Basis<Vec>(fjNode.direction, fjNode.direction.ortho()));
			}
			// flex torque and force
			fjNode.delta.n.normalize();
			double d = scEnabled ?
			               sc.length :
			               (ptr(connected.first)->getPosition() - ptr(connected.second)->getPosition()).length();
			double torque =
			    fjNode.k * fjNode.delta.teta + fjNode.c * (fjNode.delta.teta - fjNode.prevDelta.teta); // -kx - cv
			Vec vFlex = fjNode.delta.n * torque;                                                       // torque
			Vec ortho = sc.direction.ortho(fjNode.delta.n).normalized(); // force direction
			Vec force = sign * ortho * torque / d;

			node->receiveForce(-force);
			other->receiveForce(force);

			node->receiveTorque(vFlex);
			fjNode.prevDelta = fjNode.delta;
		}
		if (tjEnabled) {
			// updating torsion joint (needs to stay perp to sc.direction)
			double scalar = tjNode.direction.dot(sc.direction);
			// if the angle between our torsion spring and sc.direction is too far from 90°,
			// we reproject & recompute it
			if (abs(scalar) > MAX_TS_INCL) {
				tjNode.r = node->getOrientationRotation().inverted() +
				           Vec::getRotation(Basis<Vec>(Vec(1, 0, 0), Vec(0, 1, 0)),
				                            Basis<Vec>(sc.direction, tjNode.direction));
			} else {
				tjNode.direction = tjNode.direction.normalized() - scalar * sc.direction;
			}
			// updating targets
			tjNode.target = tjOther.direction; // we want torsion springs to stay aligned with each other
			tjNode.updateDelta();
			// torsion torque
			tjNode.delta.n.normalize();
			double torque =
			    tjNode.k * tjNode.delta.teta; // - tjNode.c * node->getAngularVelocity().dot(tjNode.delta.teta.n)
			Vec vTorsion = torque * tjNode.delta.n;
			node->receiveTorque(vTorsion);
		}
	}
};
}
#endif
