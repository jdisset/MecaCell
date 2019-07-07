#ifndef INTEGRATORS_HPP
#define INTEGRATORS_HPP
#include "utilities/utils.hpp"
namespace MecaCell {
struct Verlet {
	template <typename C> static void updatePosition(C &c, const num_t &dt) {
		// position
		auto oldVel = c.getVelocity();
		c.setVelocity(c.getVelocity() + c.getForce() * dt * c.getInvMass());
		c.setPrevposition(c.getPosition());
		c.setPosition(c.getPosition() + (c.getVelocity() + oldVel) * dt * 0.5);
	}
	template <typename C>
	static void updateOrientation(C &c, num_t momentOfInertia, const num_t &dt) {
		// orientation
		auto oldVel = c.getAngularVelocity();
		c.setAngularVelocity(c.getAngularVelocity() + c.getTorque() * dt / momentOfInertia);
		c.setOrientationRotation(c.getOrientationRotation() +
		                         (c.getAngularVelocity() + oldVel) * dt * 0.5);
		c.updateCurrentOrientation();
	}
};

struct Euler {
	template <typename C> static void updatePosition(C &c, const num_t &dt) {
		// position
		c.setVelocity(c.getVelocity() + c.getForce() * dt * c.getInvMass());
		c.setPrevposition(c.getPosition());
		c.setPosition(c.getPosition() + c.getVelocity() * dt);
	}

	template <typename C>
	static void updateOrientation(C &c, num_t momentOfInertia, const num_t &dt) {
		// orientation
		c.setAngularVelocity(c.getAngularVelocity() + c.getTorque() * dt / momentOfInertia);
		c.setOrientationRotation(c.getOrientationRotation() + c.getAngularVelocity() * dt);
		c.updateCurrentOrientation();
	}
};
}  // namespace MecaCell
#endif
