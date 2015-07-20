#ifndef INTEGRATORS_HPP
#define INTEGRATORS_HPP

// Integration schemes
// using structs instead of lambda templates (c++14 feature :-/ )
namespace MecaCell {

struct Verlet {
	template <typename C> void operator()(C &c, const double &dt) {

		if (c.isMovementEnabled()) {
			// position
			auto oldVel = c.getVelocity();
			c.setVelocity(c.getVelocity() + c.getForce() * dt / c.getMass());
			c.setPosition(c.getPosition() + (c.getVelocity() + oldVel) * dt * 0.5);

			// orientation
			oldVel = c.getAngularVelocity();
			c.setAngularVelocity(c.getAngularVelocity() + c.getTorque() * dt / c.getMomentOfInertia());
			c.setOrientationRotation(c.getOrientationRotation() + (c.getAngularVelocity() + oldVel) * dt * 0.5);
			c.updateCurrentOrientation();
		}
	}
};
struct Euler {
	template <typename C> void operator()(C &c, const double &dt) {

		if (c.isMovementEnabled()) {
			// position
			c.setVelocity(c.getVelocity() + c.getForce() * dt / c.getMass());
			c.setPosition(c.getPosition() + c.getVelocity() * dt);

			// orientation
			c.setAngularVelocity(c.getAngularVelocity() + c.getTorque() * dt / c.getMomentOfInertia());
			c.setOrientationRotation(c.getOrientationRotation() + c.getAngularVelocity() * dt);
			c.updateCurrentOrientation();
		}
	}
};
}
#endif
