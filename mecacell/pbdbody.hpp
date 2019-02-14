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

	PBD::ConstraintContainer<PBD::DistanceConstraint_REF<Vec>,
	                         PBD::AlignmentConstraint<Vec>>
	    constraints;

	num_t length = 1.0;
	num_t individualChainLength = 0.0;

	std::array<PBD::Particle<Vec>, N> particles;

	void solveInnerConstraints() { PBD::projectConstraints(constraints); }

	void generateConstraints() {
		constraints.clear();
		constraints.reserve<PBD::DistanceConstraint_REF<Vec>>(N - 1);
		setLength(length);

		// distance constraints between each particles
		for (size_t i = 0; i < N - 1; ++i) {
			constraints.addConstraint(PBD::DistanceConstraint_REF<Vec>(
			    {{&particles[i].predicted, &particles[i + 1].predicted}},
			    {{particles[i].w, particles[i + 1].w}}, individualChainLength,
			    distanceStiffness));
		}

		// alignment constraints
		if constexpr (N > 2) {
			for (size_t i = 0; i < N - 2; ++i) {
				constraints.addConstraint(PBD::AlignmentConstraint<Vec>(
				    {{&particles[i].predicted, &particles[i + 1].predicted,
				      &particles[i + 2].predicted}},
				    {{particles[i].w, particles[i + 1].w, particles[i + 2].w}},
				    bendingStiffness));
			}
		}
	}

	void setLength(num_t l) {
		length = l;
		if constexpr (N > 1)
			individualChainLength = (length - particles[0].radius - particles[N - 1].radius) /
			                        static_cast<num_t>(N - 1);
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

	std::pair<Vec, Vec> getAABB() {
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
		generateConstraints();
	}

	num_t getBoundingBoxRadius() const { return particles[0].radius; }

	/*// serialization*/
	// ExportableAlias<PBDBody_particles, std::vector<Vec>> particlesPos{[](const auto& a) {
	// std::vector<Vec> v;
	// v.reserve(a.particles.size());
	// for (const auto& p : a.particles) v.push_back(p.position);
	// return v;
	//}};

	EXPORTABLE(PBDBody_particles, KV(particles));
};

template <typename C> using PBDBody_singleParticle = MecaCell::PBDBody_particles<1, C>;

}  // namespace MecaCell

#endif
