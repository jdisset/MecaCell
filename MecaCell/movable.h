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
	double mass = 1.0;
	double baseMass = 1.0;
	double totalForce = 0;

 public:
	/**********************************************
	 *               CONSTRUCTOR
	 **********************************************/
	Movable() {}
	Movable(Vec pos) : position(pos) {}
	Movable(Vec pos, double m) : position(pos), mass(m) {}
	/**********************************************
	 *                GET & SET
	 **********************************************/
	Vec getPosition() const { return position; }
	Vec getPrevPosition() const { return prevposition; }
	Vec getVelocity() const { return velocity; }
	Vec getForce() const { return force; }
	double getMass() const { return mass; }
	double getBaseMass() const { return baseMass; }
	void setPosition(const Vec& p) { position = p; }
	void setPrevPosition(const Vec& p) { prevposition = p; }
	void setVelocity(const Vec& v) { velocity = v; }
	void setForce(const Vec& f) { force = f; }
	void setMass(const double m) { mass = m; }
	void setBaseMass(const double m) { baseMass = m; }
	/**********************************************
	 *                 UPDATES
	 **********************************************/
	void receiveForce(const double& intensity, const Vec& direction, const bool& compressive) {
		force += direction * intensity;
		totalForce += compressive ? intensity : -intensity;
	}
	void receiveForce(const Vec& f) { force += f; }
	void resetVelocity() { velocity = Vec::zero(); }
	void resetForce() {
		totalForce = 0;
		force = Vec::zero();
	}
};
}
#endif
