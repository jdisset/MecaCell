#ifndef MOVABLE_H
#define MOVABLE_H
#include "utilities/utils.hpp"

namespace MecaCell {
class Movable {
 protected:
	Vec position = Vec::zero();
	Vec prevposition = Vec::zero();
	Vec velocity = Vec::zero();
	Vec force = Vec::zero();
	bool movementEnabled = true;
	num_t mass = Config::DEFAULT_CELL_MASS;
	num_t invMass = 1.0 / Config::DEFAULT_CELL_MASS;
	num_t totalForce = 0;

 public:
	Movable() {}
	Movable(Vec pos) : position(pos) {}
	Movable(Vec pos, num_t m) : position(pos), mass(m) {}
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
	num_t getMass() const { return mass; }
	num_t getInvMass() const { return invMass; }
	void setPosition(const Vec &p) { position = p; }
	void setPrevposition(const Vec &p) { prevposition = p; }
	void setVelocity(const Vec &v) { velocity = v; }
	void setForce(const Vec &f) { force = f; }
	void setMass(const num_t m) {
		mass = m;
		invMass = (m != 0) ? (1.0 / m) : 0;
	}
	void setInvMass(const num_t w) {
		invMass = w;
		mass = (w != 0) ? (1.0 / w) : 0;
	}
	/**********************************************
	 *                 UPDATES
	 **********************************************/
	void receiveForce(const num_t &intensity, const Vec &direction,
	                  const bool &compressive) {
		force += direction * intensity;
		totalForce += compressive ? intensity : -intensity;
	}
	void receiveForce(const Vec &f) { force += f; }
	void resetVelocity() { velocity = Vec::zero(); }
	void resetForce() {
		totalForce = 0;
		force = Vec::zero();
	}
};
}  // namespace MecaCell
#endif
