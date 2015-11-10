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
	bool movementEnabled = true;
	float_t mass = 1.0;
	float_t baseMass = 1.0;
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
		const float_t rndInt = roundN(intensity);
		force += roundN(direction) * rndInt;
		totalForce += compressive ? rndInt : -rndInt;
	}
	void receiveForce(const Vec &f) { force += f; }
	void resetVelocity() { velocity = Vec::zero(); }
	void resetForce() {
		totalForce = 0;
		force = Vec::zero();
	}
};
}
#endif
