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
	double mass = Config::DEFAULT_CELL_MASS;
	double invMass = 1.0 / Config::DEFAULT_CELL_MASS;
	double totalForce = 0;

 public:
	Movable() {}
	Movable(Vec pos) : position(pos) {}
	Movable(Vec pos, double m) : position(pos), mass(m) {}
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
	double getMass() const { return mass; }
	double getInvMass() const { return invMass; }
	void setPosition(const Vec &p) { position = p; }
	void setPrevposition(const Vec &p) { prevposition = p; }
	void setVelocity(const Vec &v) { velocity = v; }
	void setForce(const Vec &f) { force = f; }
	void setMass(const double m) {
		mass = m;
		invMass = (m != 0) ? (1.0 / m) : 0;
	}
	void setInvMass(const double w) {
		invMass = w;
		mass = (w != 0) ? (1.0 / w) : 0;
	}
	/**********************************************
	 *                 UPDATES
	 **********************************************/
	void receiveForce(const double &intensity, const Vec &direction,
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
