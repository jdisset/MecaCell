#ifndef MECACELL_PBD_PLUGIN_HPP
#define MECACELL_PBD_PLUGIN_HPP
#include "integrators.hpp"
#include "pbd.hpp"
#include "utilities/grid.hpp"
#include "utilities/simple2dgrid.hpp"

namespace MecaCell {
template <typename Body> struct PBDPlugin {
	using body_t = Body;  // meant to work with a PBDBody

	using AABB_t = typename Grid<body_t *>::AABB_t;

	bool frictionEnabled = true;
	bool AABBCollisionEnabled = true;
	bool precisePreCollisionCheck = false;
	size_t iterations = 1;
	size_t constraintGenerationFreq = 1;
	num_t sleepingEpsilon = 1e-5;
	num_t gridSize = 10;
	num_t minSampleSize = 0.1;
	num_t kineticFrictionCoef = 0.0;
	num_t staticFrictionCoef = 0.0;

	// using grid_t = Grid<body_t *>;
	// grid_t grid{gridSize};

	using grid_t = Simple2DGrid<body_t *>;

	grid_t grid{-500, -500, 500, 500, gridSize};

	std::vector<std::tuple<PBD::Particle<Vec> *, PBD::Particle<Vec> *, num_t>> constraints;

	std::vector<std::function<void(void)>> constraintSolveHook;
	std::vector<std::function<void(void)>> frictionSolveHook;

	void setGridSize(num_t g) {
		gridSize = g;
		// grid = grid_t(gridSize);
		grid = grid_t(-500, -500, 500, 500, gridSize);
	}

	// helper to get the grid discretized Axis Aligned Bounding Box of a cell
	typename grid_t::AABB_t inline AABB(const body_t &b) const {
		return grid.getAABB(b.getAABB());
	}

	template <typename T, size_t N> std::string artos(const std::array<T, N> &a) const {
		std::ostringstream o;
		o << "[ ";
		for (const auto &i : a) o << i << " ";
		o << "]";
		return o.str();
	}

	template <typename W> void reinsertAllCellsInGrid(W *w) {
		grid.clear();
		for (const auto &c : w->bodies) {
			auto bb = AABB(c);
			grid.insert(c, bb);
		}
	}

	template <typename W> void reinsertAllCellsInGrid_withSample(W *w) {
		assert(w->bodies.size() == c.bodies.size());
		grid.clear();
		std::uniform_real_distribution<num_t> d(0.0, 1.0);
		const size_t S = w->bodies.size();
		for (size_t i = 0; i < S; ++i) {
			num_t diceRoll = d(Config::globalRand());
			if (diceRoll < std::max(minSampleSize, w->cells[i].activityLevel))
				grid.insert(&w->bodies[i], AABB(w->bodies[i]));
		}
	}

	template <typename W> void refreshConstraints(W *) {
		size_t prevConstraintsSize = constraints.size();
		constraints.clear();
		constraints.reserve(
		    static_cast<size_t>(static_cast<num_t>(prevConstraintsSize) * 1.1));

		auto &orderedVec = grid.getOrderedVec();
		for (auto &gridCellPair : orderedVec) {
			const auto &gridCell = gridCellPair.second.get();
			const size_t GRIDSIZE = gridCell.size();
			for (size_t i = 0; i < GRIDSIZE; ++i) {
				for (size_t j = i + 1; j < GRIDSIZE; ++j) {
					if (!AABBCollisionEnabled ||
					    (AABBCollisionEnabled &&
					     grid.AABBCollision(AABB(*gridCell[i]), AABB(*gridCell[j])))) {
						for (auto &p0 : gridCell[i]->particles) {
							for (auto &p1 : gridCell[j]->particles) {
								if (!precisePreCollisionCheck ||
								    (p0.predicted - p1.predicted).squaredNorm() <
								        std::pow(p0.radius + p1.radius, 2)) {
									constraints.push_back(
									    std::make_tuple(&p0, &p1,
									                    std::min(gridCell[i]->distanceStiffness,
									                             gridCell[j]->distanceStiffness)));
								}
							}
						}
					}
				}
			}
		}
	}

	template <typename W> void updateParticles(W *w) {
		const auto &dt = w->getDt();
		for (auto &b : w->bodies) {
			PBD::velocitiesAndPositionsUpdate(b.particles, dt, sleepingEpsilon);
			b.resetForces();
		}
	}

	template <typename W> void pbdUpdateRoutine(W *w) {
		// std::cerr << " --- PBDUPDATE 0" << std::endl;
		const auto &dt = w->getDt();

		// euler
		for (auto &b : w->bodies) {
			PBD::eulerIntegration(b.particles, dt);
			b.resetForces();
		}
		// std::cerr << " --- PBDUPDATE 1" << std::endl;

		// generate collisions
		if (w->getNbUpdates() % constraintGenerationFreq == 0) {
			reinsertAllCellsInGrid_withSample(w);
			refreshConstraints(w);
		}
		// std::cerr << " --- PBDUPDATE 2" << std::endl;
		// project constraints
		for (unsigned int i = 0; i < iterations; ++i) {
			// std::cerr << " ---  --- PBD_ITERATE 0" << std::endl;
			for (auto &b : w->bodies) b.solveInnerConstraints();
			// std::cerr << " ---  --- PBD_ITERATE 1" << std::endl;
			for (auto &con : constraintSolveHook) con();
			// std::cerr << " ---  --- PBD_ITERATE 2" << std::endl;
			// collision constraints
			for (auto &c : constraints) {
				const auto &p0 = std::get<0>(c);
				const auto &p1 = std::get<1>(c);
				const auto &distStiff = std::get<2>(c);
				PBD::CollisionConstraint::solve<Vec>({{&(p0->predicted), &(p1->predicted)}},
				                                     {{p0->w, p1->w}}, p0->radius + p1->radius,
				                                     distStiff);
			}
			// std::cerr << " ---  --- PBD_ITERATE 3" << std::endl;
		}
		// std::cerr << " --- PBDUPDATE 3" << std::endl;

		for (auto &con : frictionSolveHook) con();

		// std::cerr << " --- PBDUPDATE 4" << std::endl;
		if (frictionEnabled) {
			for (auto &c : constraints) {
				const auto &p0 = std::get<0>(c);
				const auto &p1 = std::get<1>(c);
				PBD::FrictionConstraint::solve(*p0, *p1, staticFrictionCoef, kineticFrictionCoef);
			}
		}
		// std::cerr << " --- PBDUPDATE 5" << std::endl;
		updateParticles(w);
		// std::cerr << " --- PBDUPDATE 6" << std::endl;
	}

	template <typename W> void endUpdate(W *w) { pbdUpdateRoutine(w); }
	// TODO : remove dead cells from grid if REINSERT is enabled
};  // namespace MecaCell
}  // namespace MecaCell
#endif
