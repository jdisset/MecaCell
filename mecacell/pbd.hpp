#pragma once
#include <array>
#include <cmath>
#include <tuple>
#include <utility>
#include <vector>

#include "utilities/exportable.hpp"

namespace PBD {
EXPORTABLE_NAMESPACE_DEFINITIONS

using num_t = double;
/*
  _______ _               _____  ____  _____
 |__   __(_)             |  __ \|  _ \|  __ \*
    | |   _ _ __  _   _  | |__) | |_) | |  | |
    | |  | | '_ \| | | | |  ___/|  _ <| |  | |
    | |  | | | | | |_| | | |    | |_) | |__| |
    |_|  |_|_| |_|\__, | |_|    |____/|_____/
                   __/ |
                  |___/
*/

//   --- Base Structs ---
//
//   For most of its core methods, tinyPBD requires a vector type that supports addition,
//   multuplication by a scalar and dot product.

// ConstraintContainer is a simple generic container that should be used for storing and
// iterating over the constraints. Designed for zero runtime overhead. It is just a tuple
// of vectors, with each vector dedicated to contain one type of constraints.
template <typename... CTypes> struct ConstraintContainer {
	using container_t = std::tuple<std::vector<CTypes>...>;

	// TypeId : helper struct to get the index of a certain type in a tuple
	template <class T, class Tuple> struct TypeId;
	template <class T, class... Types> struct TypeId<T, std::tuple<T, Types...>> {
		static const constexpr size_t value = 0;
	};
	template <class T, class U, class... Types> struct TypeId<T, std::tuple<U, Types...>> {
		static const constexpr size_t value = 1 + TypeId<T, std::tuple<Types...>>::value;
	};

	container_t constraints;

	template <typename T> void addConstraint(const T &constraint) {
		std::get<TypeId<std::vector<T>, container_t>::value>(constraints)
		    .push_back(constraint);
	}
	template <typename T> void reserve(size_t s) {
		std::get<TypeId<std::vector<T>, container_t>::value>(constraints).reserve(s);
	}
	template <typename T> size_t size() {
		return std::get<TypeId<std::vector<T>, container_t>::value>(constraints).size();
	}
	template <size_t i = 0> size_t size() { return std::get<i>(constraints).size(); }

	// Iterates over each sub vector.
	template <typename... Args, typename Func, std::size_t... Idx>
	void forEachType(Func &&f, std::index_sequence<Idx...>) const {
		(void)std::initializer_list<int>{
		    (std::forward<Func>(f)(std::get<Idx>(constraints)), void(), 0)...};
	}
	template <typename Func> void forEachType(Func &&f) const {
		forEachType(std::forward<Func>(f), std::index_sequence_for<CTypes...>{});
	}
	void clear() { constraints = container_t(); }
};

// Particle struct. If you want to use the particle-based euler integration, your objects
// should either inherit from this struct or have the same members.
template <typename Vec_t> struct Particle {
	Vec_t position;
	Vec_t predicted;
	Vec_t velocity;
	// Vec_t forces;
	num_t radius = 1.0f;
	num_t w = 1.0;  // w = 1 / mass

	EXPORTABLE(Particle, KV(position), KV(velocity), KV(radius));
};

//   --- Core Procedures ---

// computeDX returns the position corrections, given a constraint value, it's gradient
// and the inverse weights of the particles. The gradient should be a row vector.
template <typename Vec_t, typename num_t>
inline Vec_t computeDX(const num_t &con, const Vec_t &gradient, const num_t w) {
	auto mdc = w * gradient;
	return (con / gradient.dot(mdc)) * mdc;
}
template <typename Vec_t, typename num_t, size_t N>
std::array<Vec_t, N> computeDX(const num_t &con, const std::array<Vec_t, N> &gradient,
                               const std::array<num_t, N> &w) {
	std::array<Vec_t, N> mdc;
	for (size_t i = 0; i < N; ++i) mdc[i] = w[i] * gradient[i];

	std::array<Vec_t, N> res;
	auto gradientDotMdc = gradient[0].dot(mdc[0]);
	for (size_t i = 1; i < N; ++i) gradientDotMdc += gradient[i].dot(mdc[i]);
	auto conOverG = con / gradientDotMdc;
	for (size_t i = 0; i < N; ++i) res[i] = conOverG * mdc[i];
	return res;
}

// Simple iteration over each constraints stored in a ConstraintContainer
template <typename... T>
void projectConstraints(const ConstraintContainer<T...> &constraints) {
	constraints.forEachType([](const auto &constraintVec) {
		for (const auto &c : constraintVec) c.solve();
	});
}

// Euler integration for pbd::particles based objects
template <typename P> void eulerIntegration(P &particles, num_t dt) {
	for (auto &p : particles) {
		p.predicted = p.position + dt * p.velocity;
	}
}

// Euler integration for matrix based representations
template <typename M, typename W>  // W (the weights) can be a matrix or a num_t
void eulerIntegration(const M &positions, M &velocities, const M &forces, M &predicted,
                      const W &w, num_t dt) {
	velocities = velocities + dt * w * forces;
	predicted = positions + dt * velocities;
}

// Position and velocities update for pbd::particles based objects
template <typename P>
void velocitiesAndPositionsUpdate(P &particles, num_t dt,
                                  num_t sleepingEpsilon = 0.0001) {
	for (auto &p : particles) {
		if ((p.predicted - p.position).squaredNorm() < sleepingEpsilon)
			p.predicted = p.position;
	}
	for (auto &p : particles) {
		p.velocity = (p.predicted - p.position) / dt;
		p.position = p.predicted;
	}
}

template <typename M>
void velocitiesAndPositionsUpdate(M &positions, M &velocities, const M &predicted,
                                  num_t dt) {
	velocities = (predicted - positions) / dt;
	positions = predicted;
}

// Main step function for the PBD update loop. Takes a list of particles, a list of
// constraints, the delta time and the number of iterations. Update particles positions
// and velocities. (Remember that you probably want to reset your forces between steps).

// particle struct version:
template <typename... T, typename P>
void step(P &particles, const ConstraintContainer<T...> &constraints, num_t dt,
          unsigned int iterations = 1) {
	eulerIntegration(particles, dt);
	for (unsigned int i = 0; i < iterations; ++i) PBD::projectConstraints(constraints);
	velocitiesAndPositionsUpdate(particles, dt);
}
// matrix based version:
template <typename... T, typename M, typename W>
void step(M &positions, M &velocities, const M &forces, M &predicted, const W &w,
          const ConstraintContainer<T...> &constraints, num_t dt,
          unsigned int iterations = 1) {
	eulerIntegration(positions, velocities, forces, predicted, w, dt);
	for (unsigned int i = 0; i < iterations; ++i) PBD::projectConstraints(constraints);
	velocitiesAndPositionsUpdate(positions, velocities, predicted, dt);
}

/*
   ____                _             _       _
  / ___|___  _ __  ___| |_ _ __ __ _(_)_ __ | |_  ___
 | |   / _ \| '_ \/ __| __| '__/ _` | | '_ \| __|/ __|
 | |__| (_) | | | \__ \ |_| | | (_| | | | | | |_ \__ \
  \____\___/|_| |_|___/\__|_|  \__,_|_|_| |_|\__||___/
 -----------------------------------------------------------------------------
 Below are the "official" tinyPBD constraints. A constraint is a struct that defines a
 void solve() method. It is expected that this method directly updates positions.
*/

// Generalistic distance constraint between 2 points
template <bool mustBeEqual = true> struct DistanceConstraint {
	template <typename P>
	static inline void solve(P &p0, P &p1, const num_t &d, num_t k = 1.0f) {
		auto v = p1.predicted - p0.predicted;

		const auto &resolve = [&](const num_t &l) {
			num_t constraint = l - d;
			decltype(P::predicted) n{1, 0, 0};  // n = normal. (default to {1,0,...} if l == 1)
			if (l > 0) n = v / l;
			auto tmp = (k * n * constraint) / (p0.w + p1.w);
			p0.predicted += p0.w * tmp;
			p1.predicted -= p1.w * tmp;
		};

		if constexpr (mustBeEqual) {
			resolve(v.norm());
		} else {
			num_t l = v.squaredNorm();
			if (l - d * d < 0) {
				l = std::sqrt(l);
				resolve(l);
			}
		}
	}
};
// Elastic constraint between 2 particles
using ElasticConstraint = DistanceConstraint<true>;
// Collision constraint between 2 particles
using CollisionConstraint = DistanceConstraint<false>;

// Generalistic distance constraint between 1 paricle and 1 fixed point
template <typename P> struct FixedToPointConstraint {
	using V = decltype(P::predicted);
	P &p;
	V fixedP;
	num_t k;

	FixedToPointConstraint(P &particle, const V &fixedPoint, num_t stiffness)
	    : p(particle), fixedP(fixedPoint), k(stiffness) {}

	void solve() const {
		auto dx = p.predicted - fixedP;
		p.predicted += dx * k;
	}
};  // namespace PBD

template <typename vec_t>
vec_t computeDeltaPerp(const std::array<vec_t *, 2> &X, const std::array<vec_t *, 2> &pX,
                       const vec_t &n) {
	auto A = (*(X[0]) - *(pX[0])) - (*(X[1]) - *(pX[1]));
	return A - (n * A.dot(n));
}

struct FrictionConstraint {
	template <typename P>
	static inline void solve(P &p0, P &p1, const num_t &staticFriction,
	                         const num_t &kineticFriction) {
		auto p0p1 = p1.predicted - p0.predicted;
		num_t l = p0p1.squaredNorm();
		num_t penetration = std::pow(p0.radius + p1.radius, 2) - l;
		if (penetration > 0) {
			l = sqrt(l);
			penetration = p0.radius + p1.radius - l;
			auto I = p0.predicted - p0.position;
			auto J = p1.predicted - p1.position;
			auto A = I - J;
			auto n = p0p1 / l;
			auto tengential = A - (A.dot(n) * n);
			auto tengentialNorm = tengential.norm();

			auto w0 = (p0.w / (p0.w + p1.w));

			auto dx0 = tengential * w0;
			if (tengentialNorm > (staticFriction * penetration))
				dx0 *= std::min(kineticFriction * penetration / tengentialNorm, 1.0);

			auto dx1 = -(p1.w / (p0.w + p1.w)) * dx0;

			p0.predicted = p0.predicted - dx0;
			p1.predicted = p1.predicted - dx1;
		}
	}
};

struct GroundConstraint {
	template <typename Vec_t>
	static inline void solve(const Vec_t &O, const Vec_t &N, Vec_t *X, num_t W, num_t d,
	                         num_t k) {
		// std::cerr << " .... groundConstraint 0" << std::endl;
		num_t constraintValue = (*X - O).dot(N) - d;
		if (constraintValue < 0) {
			auto dx = k * PBD::computeDX(constraintValue, -N, W);
			(*X) += dx;
		}
		// std::cerr << " .... groundConstraint 1" << std::endl;
	}
};

struct GroundFrictionConstraint {
	template <typename Part, typename Vec_t>
	static inline void solve(Part &p, const Vec_t &O, const Vec_t &N, num_t sF = 0.0,
	                         num_t kF = 0.0) {
		num_t penetration1 = -(p.position - O).dot(N) + p.radius;
		num_t penetration2 = -(p.predicted - O).dot(N) + p.radius;
		num_t penetration = std::max(penetration1, penetration2);
		if (penetration > 0) {
			auto dx = p.predicted - p.position;
			auto T = dx - (dx.dot(N) * N);  // tangential component
			num_t TNorm = T.norm();

			if (TNorm <= sF * penetration)
				dx = T;
			else
				dx = T * (std::min(kF * penetration / TNorm, 1.0));

			p.predicted -= dx;
		}
	}
};

template <typename Vec_t> struct PlaneConstraint {
	const Vec_t P;  // point on the plane
	const Vec_t N;  // normal of the ground (particles should be above)
	Vec_t *X;       // pointer to the particle position
	const num_t W;  // inverse of mass
	const num_t k;  // constraint stiffness

	PlaneConstraint(const Vec_t &p, const Vec_t &n, Vec_t *x, num_t w, num_t stiffness)
	    : P(p), N(n), X(x), W(w), k(stiffness) {}

	void solve() const {
		num_t constraintValue = (*X - P).dot(N);
		auto dx = k * PBD::computeDX(constraintValue, -N, W);
		(*X) += dx;
	}
};

}  // namespace PBD
