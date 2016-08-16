#ifndef ORIENTABLE_H
#define ORIENTABLE_H
#include "utilities/utils.h"
namespace MecaCell {
class Orientable {
 protected:
	Vec angularVelocity = Vec::zero();
	Vec torque = Vec::zero();
	Vec extTorque = Vec::zero();
	Basis<Vec> orientation;
	Rotation<Vec> orientationRotation;

 public:
	/**********************************************
	 *               CONSTRUCTOR
	 **********************************************/
	Orientable(){};

	/**********************************************
	 *                GET & SET
	 **********************************************/
	Vec getAngularVelocity() const { return angularVelocity; }
	Vec getTorque() const { return torque; }
	Basis<Vec> getOrientation() const { return orientation; }
	Rotation<Vec> getOrientationRotation() const { return orientationRotation; }
	void setAngularVelocity(const Vec& v) { angularVelocity = v; }
	void setTorque(const Vec& t) { torque = t; }
	void setOrientationRotation(const Rotation<Vec>& r) { orientationRotation = r; }

	/**********************************************
	 *                  UPDATES
	 **********************************************/
	void receiveTorque(const Vec& t) { torque += t; }
	void receiveExternalTorque(const Vec& t) { extTorque += t; }
	void applyExternalTorque() { torque += extTorque; }
	void resetExternalTorque() { extTorque = Vec::zero(); }
	void updateCurrentOrientation() { orientation.updateWithRotation(orientationRotation); }
	void resetTorque() { torque = Vec::zero(); }
	void resetAngularVelocity() { angularVelocity = Vec::zero(); }
};
}
#endif
