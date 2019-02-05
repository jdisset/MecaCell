#ifndef MECACELL_PBDBODY_HPP
#define MECACELL_PBDBODY_HPP
#include <vector>
#include "PBDPlugin.hpp"
#include "mecacell.h"

namespace MecaCell {

template <size_t N, typename Cell> struct PBDBody_particles {
	static_assert(N > 0);

	Cell* cell = nullptr;

	using embedded_plugin_t = PBDPlugin<Cell>;
	num_t distanceStiffness = 1.0;
	num_t bendingStiffness = 1.0;

	PBD::ConstraintContainer<PBD::DistanceConstraint<Vec>, PBD::AlignmentConstraint<Vec>>
	    constraints;

	num_t length;
	std::array<PBD::Particle<Vec>, N> particles;

	void solveInnerConstraints() { PBD::projectConstraints(constraints); }

	void generateConstraints() {
		constraints.clear();
		num_t l = length / static_cast<num_t>(N);

		// distance constraints between each particles
		for (size_t i = 0; i < N - 1; ++i) {
			constraints.addConstraint(PBD::DistanceConstraint<Vec>(
			    {{&particles[i].predicted, &particles[i + 1].predicted}},
			    {{particles[i].w, particles[i + 1].w}}, l, distanceStiffness));
		}

		// alignment constraints
		for (size_t i = 0; i < N - 2; ++i) {
			constraints.addConstraint(PBD::AlignmentConstraint<Vec>(
			    {{&particles[i].predicted, &particles[i + 1].predicted,
			      &particles[i + 2].predicted}},
			    {{particles[i].w, particles[i + 1].w, particles[i + 2].w}}, bendingStiffness));
		}
	}

	PBDBody_particles(Cell* c) : cell(c) { generateConstraints(); }

	PBDBody_particles(Cell* c, const Vec& p) : cell(c) {
		setPosition(p);
		generateConstraints();
	}

	Vec getPosition() {
		// returns the position of the first particle
		return particles[0].position;
	}

	Vec getVelocity() {
		// returns the average velocity
		if constexpr (N == 0)
			return particles[0].velocity;
		else {
			Vec sum = Vec::zero();
			for (const auto& p : particles) sum += p.velocity;
			return sum / static_cast<num_t>(N);
		}
	}

	void setPosition(const Vec& c, const Vec& n = Vec(1, 0, 0)) {
		num_t l = length / static_cast<num_t>(N);
		particles[0].position = c;
		particles[0].predicted = c;
		for (size_t i = 1; i < N; ++i) {
			particles[i].position = particles[i - 1].position + n * l;
			particles[i].predicted = particles[i].position;
		}
	}

	void receiveForce(const Vec& f) { particles[0].forces += f; }
	void resetForces() { particles[0].forces = Vec::zero(); }
	void setVelocity(const Vec& v) { particles[0].velocity = v; }
	void setRadius(const num_t& r) { particles[0].radius = r; }
	num_t getBoundingBoxRadius() const { return particles[0].radius; }

	// serialization
	ExportableAlias<PBDBody_particles, num_t> radius{
	    [](const auto& a) { return a.getBoundingBoxRadius(); }};
	EXPORTABLE(PBDBody_particles, (radius, particles[0].radius),
	           (position, particles[0].position), (velocity, particles[0].velocity));
};

template <typename C> using PBDBody_singleParticle = MecaCell::PBDBody_particles<1, C>;

}  // namespace MecaCell

#endif
