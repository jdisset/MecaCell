#ifndef MECACELL_PBDBODY_HPP
#define MECACELL_PBDBODY_HPP
#include <vector>
#include "PBDPlugin.hpp"
#include "mecacell.h"
#include "utilities/exportable.hpp"

namespace MecaCell {

template <size_t N> struct PBDBody_particles {
	static_assert(N > 0);

	size_t id = 0;

	static const constexpr size_t N_PARTICLES = N;
	using embedded_plugin_t = PBDPlugin<PBDBody_particles<N>>;
	num_t distanceStiffness = 0.8;  // for collisions with other cells
	num_t innerDistanceStiffness = 1.0;
	num_t bendingStiffness = 0.0;

	// PBD::ConstraintContainer<PBD::DistanceConstraint_REF<Vec>> constraints;

	num_t length = 1.0;
	num_t individualChainLength = 0.0;

	std::array<PBD::Particle<Vec>, N> particles;

	Vec getCOM() {
		Vec COM = Vec::zero();
		for (size_t i = 0; i < N; ++i) COM += particles[i].predicted;
		return COM / static_cast<num_t>(N);
	}

	void solveInnerConstraints() {
		if constexpr (N > 1) {
			for (size_t i = 0; i < N - 1; ++i) {
				PBD::ElasticConstraint::solve(
				    {{&particles[i].predicted, &particles[i + 1].predicted}},
				    {{particles[i].w, particles[i + 1].w}}, individualChainLength,
				    innerDistanceStiffness);
			}
		}
	}

	void setLength(num_t l) {
		length = l;
		if constexpr (N > 1) {
			individualChainLength = (length - particles[0].radius - particles[N - 1].radius) /
			                        static_cast<num_t>(N - 1);
		}
	}

	PBDBody_particles() {}

	PBDBody_particles(const Vec& p) { setPosition(p); }

	num_t getActualLength() {
		num_t l = 0;
		for (size_t i = 0; i < N - 1; ++i)
			l += (particles[i + 1].position - particles[i].position).norm();
		return l;
	}

	Vec getPosition() const {
		// returns the position of the first particle
		return particles[0].position;
	}

	Vec getVelocity() const {
		// returns the average velocity
		if constexpr (N == 0)
			return particles[0].velocity;
		else {
			Vec sum = Vec::zero();
			for (const auto& p : particles) sum += p.velocity;
			return sum / static_cast<num_t>(N);
		}
	}

	std::pair<Vec, Vec> getAABB() const {
		// returns an axis aligned bounding box
		const constexpr num_t LOWEST = std::numeric_limits<num_t>::lowest();
		const constexpr num_t HIGHEST = std::numeric_limits<num_t>::max();
		Vec A{HIGHEST, HIGHEST, HIGHEST};
		Vec B{LOWEST, LOWEST, LOWEST};
		for (const auto& p : particles) {
			const auto& x = p.position.x();
			if (x - p.radius < A.x()) A.xRef() = x - p.radius;
			if (x + p.radius > B.x()) B.xRef() = x + p.radius;

			const auto& y = p.position.y();
			if (y - p.radius < A.y()) A.yRef() = y - p.radius;
			if (y + p.radius > B.y()) B.yRef() = y + p.radius;

			const auto& z = p.position.z();
			if (z - p.radius < A.z()) A.zRef() = z - p.radius;
			if (z + p.radius > B.z()) B.zRef() = z + p.radius;
		}
		return std::make_pair(A, B);
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

	void resetProjections() {
		for (auto& p : particles) p.predicted = p.position;
	}

	void receiveForce(const Vec& f) {
		for (auto& p : particles) p.forces += f;
	}

	void resetForces() {
		for (auto& p : particles) p.forces = Vec::zero();
	}

	void setVelocity(const Vec& v) {
		for (auto& p : particles) p.velocity = v;
	}

	void setRadius(const num_t& r) {
		for (auto& p : particles) p.radius = r;
	}

	num_t getBoundingBoxRadius() const { return particles[0].radius; }

	EXPORTABLE(PBDBody_particles, KV(particles));
};

using PBDBody_singleParticle = MecaCell::PBDBody_particles<1>;

}  // namespace MecaCell

#endif
