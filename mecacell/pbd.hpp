#pragma once
#include <array>
#include <cmath>
#include <tuple>
#include <utility>
#include <vector>
#include "utilities/exportable.hpp"

namespace PBD {
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
	Vec_t forces;
	num_t radius = 1.0f;
	num_t w = 1.0;  // w = 1 / mass

	EXPORTABLE(Particle, KV(position), KV(velocity), KV(radius));
};

//   --- Core Procedures ---

// computeDX returns the position corrections, given a constraint value, it's gradient
// and the inverse weights of the particles. The gradient should be a row vector.
template <typename Vec_t, typename num_t, size_t N>
std::array<Vec_t, N> computeDX(const num_t &con, const std::array<Vec_t, N> &gradient,
                               const std::array<num_t, N> &w) {
	std::array<Vec_t, N> mdc;
	for (size_t i = 0; i < mdc.size(); ++i) mdc[i] = w[i] * gradient[i];

	std::array<Vec_t, N> res;
	auto gradientDotMdc = gradient[0].dot(mdc[0]);
	for (size_t i = 1; i < res.size(); ++i) gradientDotMdc += gradient[i].dot(mdc[i]);
	auto conOverG = con / gradientDotMdc;
	for (size_t i = 0; i < res.size(); ++i) res[i] = conOverG * mdc[i];
	return res;
}

template <typename Vec_t, typename num_t>
Vec_t computeDX(const num_t &con, const Vec_t &gradient, const num_t w) {
	auto mdc = w * gradient;
	auto gradientDotMdc = gradient.dot(mdc);
	auto conOverG = con / gradientDotMdc;
	return conOverG * mdc;
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
		p.velocity += dt * p.w * p.forces;
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
		if ((p.predicted - p.position).squaredNorm() < sleepingEpsilon) {
			p.predicted = p.position;
		}
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
void step(P &particles, const ConstraintContainer<T...> &constraints, double dt,
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
template <typename Vec_t, bool mustBeEqual = true> struct DistanceConstraint {
	static const constexpr unsigned int N = 2;  // nb of particles
	const std::array<Vec_t *, N> X{nullptr,
	                               nullptr};  // pointers to the particles positions
	const std::array<num_t, N> W;             // inverse of mass
	const num_t d;                            // target distance between the 2 points
	const num_t k;                            // stiffness of the constraint

	DistanceConstraint(const std::array<Vec_t *, N> &x, const std::array<num_t, N> &w,
	                   num_t dist, num_t stiffness = 1.0f)
	    : X(x), W(w), d(dist), k(stiffness) {}

	void solve() const {
		auto v = *(X[1]) - *(X[0]);  // v = X1 -> X2
		num_t l = v.squaredNorm();
		num_t constraintValue = l - d * d;
		if (mustBeEqual || constraintValue < 0) {
			l = std::sqrt(l);
			constraintValue = l - d;
			Vec_t n;  // n = normal. (default to {1,0,...} if l == 1)
			if (l > 0)
				n = v / l;
			else
				n[0] = 1;

			// definition of the gradient, stored  in a RowVector form:
			std::array<Vec_t, 2> gradient{{n, -n}};

			// compute dx and update the positions
			auto dx = PBD::computeDX(constraintValue, gradient, W);
			for (size_t i = 0; i < X.size(); ++i) *(X[i]) = *(X[i]) + k * dx[i];
		}
	}
};

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

template <typename P> struct FrictionConstraint {
	P &p0;
	P &p1;
	num_t fs;
	num_t fk;
	FrictionConstraint(P &a, P &b, num_t staticFriction, num_t kineticFriction)
	    : p0(a), p1(b), fs(staticFriction), fk(kineticFriction) {}

	void solve() const {
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
			if (tengentialNorm > (fs * penetration))
				dx0 *= std::min(fk * penetration / tengentialNorm, 1.0);

			auto dx1 = -(p1.w / (p0.w + p1.w)) * dx0;

			p0.predicted = p0.predicted - dx0;
			p1.predicted = p1.predicted - dx1;
		}
	}
};

// Collision between 2 particles with static and kinetic friction
template <typename Vec_t> struct CollisionFrictionConstraint {
	static const constexpr unsigned int N = 2;  // nb of particles
	const std::array<Vec_t *, N> X{
	    nullptr, nullptr};  // pointers to the particles new projected positions
	const std::array<Vec_t *, N> prevX{nullptr,
	                                   nullptr};  // pointers to the particles positions
	const std::array<num_t, N> W;                 // inverse of mass
	const num_t d;                                // target distance between the 2 points
	const num_t fs;                               // static friction coef
	const num_t fk;                               // kinetic friction coef
	const num_t k;                                // stiffness of the constraint

	CollisionFrictionConstraint(const std::array<Vec_t *, N> &x,
	                            const std::array<Vec_t *, N> &prevx,
	                            const std::array<num_t, N> &w, num_t dist,
	                            num_t staticFriction, num_t kineticFriction,
	                            num_t stiffness)
	    : X(x),
	      prevX(prevx),
	      W(w),
	      d(dist),
	      fs(staticFriction),
	      fk(kineticFriction),
	      k(stiffness) {}

	void solve() const {
		auto v = *(X[1]) - *(X[0]);  // v = X1 -> X2
		num_t l = v.squaredNorm();
		num_t constraintValue = l - d * d;
		if (constraintValue < 0) {  // collision
			l = std::sqrt(l);
			constraintValue = l - d;
			Vec_t n;  // n = normal. (default to {1,0,...} if l == 1)
			if (l > 0)
				n = v / l;
			else
				n[0] = 1;

			// definition of the gradient, stored  in a RowVector form:
			std::array<Vec_t, 2> gradient{{n, -n}};

			// compute dx and update the positions
			auto dx = PBD::computeDX(constraintValue, gradient, W);
			for (size_t i = 0; i < X.size(); ++i) *(X[i]) = *(X[i]) + k * dx[i];
			// apply friction correction
			auto newN = (*(X[1]) - *(X[0])).normalized();
			auto dPerp = computeDeltaPerp(X, prevX, newN);
			auto dPNorm = dPerp.norm();
			auto w = W[0] / (W[0] + W[1]);
			auto penetration = -constraintValue;
			Vec_t dx0;
			if (dPNorm < fs * penetration)
				dx0 = w * dPerp;
			else
				dx0 = w * dPerp * (std::min(fk * penetration / dPNorm, 1.0));

			auto dx1 = -(W[1] / (W[1] + W[0])) * dx0;

			//*(X[0]) = *(X[0]) + dx0;
			//*(X[1]) = *(X[1]) + dx1;
		}
	}
};

// distance and stiffness can change
template <typename Vec_t, bool mustBeEqual = true> struct DistanceConstraint_REF {
	static const constexpr unsigned int N = 2;  // nb of particles
	const std::array<Vec_t *, N> X{nullptr,
	                               nullptr};  // pointers to the particles positions
	const std::array<num_t, N> W;             // inverse of mass
	const num_t &d;                           // target distance between the 2 points
	const num_t &k;                           // stiffness of the constraint

	DistanceConstraint_REF(const std::array<Vec_t *, N> &x, const std::array<num_t, N> &w,
	                       const num_t &dist, const num_t &stiffness)
	    : X(x), W(w), d(dist), k(stiffness) {}

	void solve() const {
		DistanceConstraint c{X, W, d, k};
		c.solve();
	}
};

// Constraint for 3 points to be aligned
template <typename Vec_t> struct AlignmentConstraint {
	const std::array<Vec_t *, 3> X{nullptr,
	                               nullptr};  // pointers to the particles positions
	const std::array<num_t, 3> W;             // inverse of mass
	const num_t k;                            // stiffness of the constraint
	AlignmentConstraint(const std::array<Vec_t *, 3> &x, const std::array<num_t, 3> &w,
	                    num_t stiffness = 1.0f)
	    : X(x), W(w), k(stiffness) {}

	void solve() const {
		// TODO
	}
};

template <typename Vec_t> struct GroundConstraint {
	const Vec_t P;  // point on the plane
	const Vec_t N;  // normal of the ground (particles should be above)
	Vec_t *X;       // pointer to the particle position
	const num_t W;  // inverse of mass
	const num_t d;  // target distance between the plane and the particle
	const num_t k;  // constraint stiffness

	GroundConstraint(const Vec_t &p, const Vec_t &n, Vec_t *x, num_t w, num_t dist,
	                 num_t stiffness)
	    : P(p), N(n), X(x), W(w), d(dist), k(stiffness) {}

	void solve() const {
		num_t constraintValue = (*X - P).dot(N) - d;
		if (constraintValue < 0) {
			auto dx = k * PBD::computeDX(constraintValue, -N, W);
			(*X) += dx;
		}
	}
};

template <typename Vec_t, typename Part = Particle<Vec_t>>
struct GroundFrictionConstraint {
	Part &p;
	const Vec_t O;   // point on the plane
	const Vec_t N;   // normal of the ground (particles should be above)
	const num_t sF;  // static friction
	const num_t kF;  // kinetic friction

	GroundFrictionConstraint(Part &p, const Vec_t &o, const Vec_t &n,
	                         num_t staticFriction = 0.0, num_t kineticFriction = 0.0)
	    : p(p), O(o), N(n), sF(staticFriction), kF(kineticFriction) {}

	void solve() const {
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

// Elastic constraint between 2 particles
template <typename Vec_t> using ElasticConstraint = DistanceConstraint<Vec_t, true>;

// Collision constraint between 2 particles
template <typename Vec_t> using CollisionConstraint = DistanceConstraint<Vec_t, false>;

}  // namespace PBD
