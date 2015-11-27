#ifndef CONNECTION_H
#define CONNECTION_H
#include "tools.h"

#define MAX_TS_INCL \
	0.1  // max angle before we need to reproject our torsion joint rotation

#define DBG DEBUG(connection)
namespace MecaCell {
////////////////////////////////////////////////////////////////////
//                SPRING STRUCTURE
////////////////////////////////////////////////////////////////////
// This is just a classic "linear" spring
struct Spring {
	float_t k = 1.0;           // stiffness
	float_t currentK = 1.0;    // stiffness
	float_t c = 1.0;           // damp coef
	float_t restLength = 1.0;  // rest length
	float_t length = 1.0;      // current length
	float_t prevLength = 1.0;
	float_t minLengthRatio = 0.5;  // max compression
	Vec direction;                 // current direction from node 0 to node 1
	bool updatedSinceLastComputeForce = false;

	Spring(){};
	Spring(const float_t &K, const float_t &C, const float_t &L)
	    : k(K), c(C), restLength(L), length(L){};

	void updateLengthDirection(const Vec &p0, const Vec &p1) {
		direction = p1 - p0;
		length = direction.length();
		if (length > 0) direction /= length;
		updatedSinceLastComputeForce = true;
	}
	void setRestLength(float_t L) { restLength = L; }
	inline float_t getRestLength() const { return restLength; }
};

////////////////////////////////////////////////////////////////////
//                       JOINT STRUCTURE
////////////////////////////////////////////////////////////////////
// flexible joint. Can be used for flexure (torque + force) or torsion (torque only)
struct Joint {
	float_t k = 1.0;  // angular stiffness
	float_t currentK = 1.0;
	float_t c = 1.0;                // damp
	float_t maxTeta = M_PI / 30.0;  // maximum angle
	Rotation<Vec> r;                // rotation from node to joint
	Rotation<Vec> delta;            // current rotation
	Rotation<Vec> prevDelta;
	Vec direction;                   // current direction
	Vec target;                      // targeted direction
	bool maxTetaAutoCorrect = true;  // do we need to handle maxTeta?
	bool targetUpdateEnabled = true;
	Joint(){};

	Joint(const float_t &K, const float_t &C, const float_t &MTETA, bool handleMteta = true)
	    : k(K), c(C), maxTeta(MTETA), maxTetaAutoCorrect(handleMteta) {}

	// current direction is computed using a reference Vector v rotated with rotation rot
	void updateDirection(const Vec &v, const Rotation<Vec> &rot) {
		direction = v.rotated(r.rotated(rot));
	}
	void updateDelta() { delta = Vec::getRotation(direction, target); }
	void setCurrentKCoef(float_t kc) { currentK = k * kc; }
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
// - float_t getInertia()
// - void receiveForce(float_t intensity, Vec direction, bool compressive)
// - void receiveTorque(Vec acc)
template <typename N0, typename N1 = N0> struct Connection {
	pair<N0, N1> connected;     // the two connected nodes
	Spring sc;                  // basic spring
	pair<Joint, Joint> fj, tj;  // flexure and torsion joints (1 per node)

	bool scEnabled = true, fjEnabled = true, tjEnabled = false;
	/**********************************************
	 *               CONSTRUCTOR
	 **********************************************/
	Connection(const pair<N0, N1> &n, const Spring &S)
	    : connected{n}, sc(S), fjEnabled(false), tjEnabled(false) {
		initS();
	}
	Connection(const pair<N0, N1> &n, const pair<Joint, Joint> &FJ,
	           const pair<Joint, Joint> &TJ)
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
		sc.updateLengthDirection(ptr(connected.first)->getPosition(),
		                         ptr(connected.second)->getPosition());
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

		// joint's current direction
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
	inline Spring &getSc() { return sc; }
	inline pair<Joint, Joint> &getFlex() { return fj; }
	inline pair<Joint, Joint> &getTorsion() { return tj; }
	inline const N0 &getConstNode0() const { return connected.first; }
	inline const N1 &getConstNode1() const { return connected.second; }
	inline N0 &getNode0() { return connected.first; }
	inline N1 &getNode1() { return connected.second; }
	inline float_t getLength() const { return sc.length; }
	inline Vec getDirection() const { return sc.direction; }

	/**********************************************
	 *              UPDATES
	 **********************************************/
	void updateLengthDirection() {
		sc.updateLengthDirection(ptr(connected.first)->getPosition(),
		                         ptr(connected.second)->getPosition());
	}
	void computeForces(float_t dt) {
		// BASIC SPRING
		if (!sc.updatedSinceLastComputeForce) {
			sc.updateLengthDirection(ptr(connected.first)->getPosition(),
			                         ptr(connected.second)->getPosition());
		}
		if (scEnabled) {
			float_t x = sc.length - sc.restLength;  // actual compression / elongation
			float_t minlength = sc.minLengthRatio * sc.restLength;
			if (sc.length < minlength) {
				float_t d = minlength - sc.length;
				Vec component0 =
				    ptr(connected.first)->getVelocity().dot(sc.direction) * sc.direction;
				Vec tangent0 = ptr(connected.first)->getVelocity() - component0;
				Vec component1 =
				    ptr(connected.second)->getVelocity().dot(sc.direction) * sc.direction;
				Vec tangent1 = ptr(connected.second)->getVelocity() - component1;
				ptr(connected.first)
				    ->setPosition(ptr(connected.first)->getPosition() - sc.direction * d / 2.0);
				ptr(connected.second)
				    ->setPosition(ptr(connected.second)->getPosition() + sc.direction * d / 2.0);
				ptr(connected.first)->setVelocity(tangent0 + component1);
				ptr(connected.second)->setVelocity(tangent1 + component0);
				sc.length = minlength;
			}
			bool compression = x < 0;
			float_t v = (sc.length - sc.prevLength) / dt;

			float_t k = sc.currentK;  // compression ? sc.k : sc.k * 0.2;
			float_t f = (-k * x - sc.c * v) / 2.0;
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
		sc.updatedSinceLastComputeForce = false;
	}

	template <int n> void updateFT() {
		Joint &tjNode = n == 0 ? tj.first : tj.second;
		Joint &tjOther = n == 0 ? tj.second : tj.first;
		Joint &fjNode = n == 0 ? fj.first : fj.second;
		const auto &node = ptr(get<n>(connected));
		const auto &other = ptr(get < n == 0 ? 1 : 0 > (connected));
		const float_t sign = n == 0 ? 1 : -1;

		if (fjEnabled) {
			if (fjNode.targetUpdateEnabled) fjNode.target = sc.direction * sign;
			fjNode.updateDelta();
			if (fjNode.maxTetaAutoCorrect &&
			    fjNode.delta.teta > fjNode.maxTeta) {  // if we passed flex break angle
				float dif = fjNode.delta.teta - fjNode.maxTeta;
				fjNode.r = fjNode.r + Rotation<Vec>(fjNode.delta.n, dif);
				fjNode.direction = fjNode.direction.rotated(Rotation<Vec>(fjNode.delta.n, dif));
				fjNode.r = node->getOrientationRotation().inverted() +
				           Vec::getRotation(Basis<Vec>(), Basis<Vec>(fjNode.direction,
				                                                     fjNode.direction.ortho()));
			}
			// flex torque and force
			float_t d = scEnabled ? sc.length : (ptr(connected.first)->getPosition() -
			                                     ptr(connected.second)->getPosition())
			                                        .length();
			//std::cerr << "currentK = " << fjNode.currentK << std::endl;
			float_t torque =
			    fjNode.currentK * fjNode.delta.teta +
			    fjNode.c * (fjNode.delta.teta - fjNode.prevDelta.teta);   // -kx - cv
			Vec vFlex = fjNode.delta.n * torque;                          // torque
			Vec ortho = sc.direction.ortho(fjNode.delta.n).normalized();  // force direction
			Vec force = sign * ortho * torque / d;

			node->receiveForce(-force);
			other->receiveForce(force);

			node->receiveTorque(vFlex);
			fjNode.prevDelta = fjNode.delta;
		}
		if (tjEnabled) {
			// updating torsion joint (needs to stay perp to sc.direction)
			float_t scalar = tjNode.direction.dot(sc.direction);
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
			tjNode.target =
			    tjOther.direction;  // we want torsion springs to stay aligned with each other
			tjNode.updateDelta();
			// torsion torque
			float_t torque =
			    tjNode.currentK *
			    tjNode.delta
			        .teta;  // - tjNode.c * node->getAngularVelocity().dot(tjNode.delta.teta.n)
			Vec vTorsion = torque * tjNode.delta.n;
			node->receiveTorque(vTorsion);
		}
	}
};
}
#endif
