#ifndef INTEGRATORS_HPP
#define INTEGRATORS_HPP

// Integration schemes
// using structs instead of lambda templates (c++14 feature :-/ )
namespace MecaCell {

// A mix of Verlet for position and Euler for orientation. Pretty fast, accurate enough.
struct VerletEuler {
	template <typename C> void operator()(C& c, const double& dt) {
		// position (Verlet)
		auto xtemp = c.getPosition();
		c.setVelocity((c.getPosition() - c.getPrevPosition()) / dt);
		c.setPosition(c.getPosition() + (c.getPosition() - c.getPrevPosition()) +
		              (c.getForce() / c.getMass()) * dt * dt);
		c.setPrevPosition(xtemp);

		// orientation (Euler)
		c.setAngularVelocity(c.getAngularVelocity() + (c.getTorque() / c.getMomentOfInertia()) * dt);
		c.setOrientationRotation(c.getOrientationRotation() + c.getAngularVelocity() * dt);
		c.updateCurrentOrientation();
	}
};
}
#endif
