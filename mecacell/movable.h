#ifndef MOVABLE_H
#define MOVABLE_H
#include "tools.h"

namespace MecaCell {
class Movable {
 protected:
	Vec position = Vec::zero();
	Vec prevposition = Vec::zero();
	Vec velocity = Vec::zero();
	Vec force = Vec::zero();
	Vec extForce = Vec::zero();  // not reset
	bool movementEnabled = true;
	float_t mass = DEFAULT_CELL_MASS;
	float_t baseMass = DEFAULT_CELL_MASS;
	float_t totalForce = 0;

 public:
	/**********************************************
	 *               CONSTRUCTOR
	 **********************************************/
	Movable() {}
	Movable(Vec pos) : position(pos) {}
	Movable(Vec pos, float_t m) : position(pos), mass(m) {}
	/**********************************************
	 *                GET & SET
	 **********************************************/
	bool isMovementEnabled() { return movementEnabled; }
	void disableMovement() { movementEnabled = false; }
	void enableMovement() { movementEnabled = true; }
	Vec getPosition() const { return position; }
	Vec getPrevposition() const { return prevposition; }
	Vec getVelocity() const { return velocity; }
	Vec getForce() const { return force; }
	float_t getMass() const { return mass; }
	float_t getBaseMass() const { return baseMass; }
	void setPosition(const Vec &p) { position = p; }
	void setPrevposition(const Vec &p) { prevposition = p; }
	void setVelocity(const Vec &v) { velocity = v; }
	void setForce(const Vec &f) { force = f; }
	void setMass(const float_t m) { mass = m; }
	void setBaseMass(const float_t m) { baseMass = m; }
	/**********************************************
	 *                 UPDATES
	 **********************************************/
	void receiveForce(const float_t &intensity, const Vec &direction,
	                  const bool &compressive) {
		force += direction * intensity;
		totalForce += compressive ? intensity : -intensity;
	}
	void receiveForce(const Vec &f) { force += f; }
	void receiveExternalForce(const Vec &f) { extForce += f; }
	void applyExternalForces() { force += extForce; }
	void resetExternalForces() { extForce = Vec::zero(); }
	void resetVelocity() { velocity = Vec::zero(); }
	void resetForce() {
		totalForce = 0;
		force = Vec::zero();
	}
};
}
#endif
