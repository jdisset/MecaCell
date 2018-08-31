#ifndef MECACELL_PBDBODY_HPP
#define MECACELL_PBDBODY_HPP
#include <vector>
#include "PBDPlugin.hpp"
#include "mecacell.h"

namespace MecaCell {
template <typename Cell> class PBDBody_singleParticle {
	Cell* cell = nullptr;

 public:
	using embedded_plugin_t = PBDPlugin<Cell>;
	std::array<PBD::Particle<Vec>, 1> particles;

	void solveInnerConstraints(...) {}  // no inner constraints for a 1 particle body

	PBDBody_singleParticle(Cell* c) : cell(c) {}
	PBDBody_singleParticle(Cell* c, const Vec& p) : cell(c) { setPosition(p); }

	Vec getPosition() {
		Vec pos{Vec::zero()};
		for (const auto& p : particles) pos += p.position;
		return pos / static_cast<num_t>(particles.size());
	}

	void setPosition(const Vec& c) {
		particles[0].position = c;
		particles[0].predicted = c;
	}

	void receiveForce(const Vec& f) { particles[0].forces += f; }
	void resetForces() { particles[0].forces = Vec::zero(); }
	void setVelocity(const Vec& v) { particles[0].velocity = v; }
	void setRadius(const num_t& r) { particles[0].radius = r; }
	num_t getBoundingBoxRadius() const { return particles[0].radius; }
};
}  // namespace MecaCell

#endif
