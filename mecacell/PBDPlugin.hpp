#ifndef MECACELL_PBD_PLUGIN_HPP
#define MECACELL_PBD_PLUGIN_HPP
#include "integrators.hpp"
#include "pbd.hpp"
#include "utilities/grid.hpp"
#include "utilities/simple2dgrid.hpp"

#define USE_SIMPLE_GRID 1

namespace MecaCell {
template <typename Body> struct PBDPlugin {
	using body_t = Body;  // meant to work with a PBDBody

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

#if USE_SIMPLE_GRID
	num_t bottomLeft_x = -400;
	num_t bottomLeft_z = -400;
	num_t topRight_x = 400;
	num_t topRight_z = 400;
	using grid_t = Simple2DGrid<size_t>;
	grid_t grid{bottomLeft_x, bottomLeft_z, topRight_x, topRight_z, gridSize};
#else
	using grid_t = Grid<size_t>;
	grid_t grid{gridSize};
#endif

	std::vector<std::tuple<PBD::Particle<Vec> *, PBD::Particle<Vec> *, num_t>> constraints;

	std::vector<std::function<void(void)>> constraintSolveHook;
	std::vector<std::function<void(void)>> frictionSolveHook;

	std::vector<typename grid_t::AABB_t> aabbContainer;

	void setGridSize(num_t g) {
		gridSize = g;
#if USE_SIMPLE_GRID
		grid = grid_t(-500, -500, 500, 500, gridSize);
#else
		grid = grid_t(gridSize);
#endif
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

	// more cache friendly than having to retrieve cells[i].activityLevel:
	std::vector<num_t> actLvl;
	template <typename W> void refreshActLvl(W *w) {
		actLvl.clear();
		const size_t S = w->bodies.size();
		actLvl.reserve(S);
		for (size_t i = 0; i < S; ++i) actLvl.push_back(w->cells[i].activityLevel);
	}

	template <typename W> void refreshAABBContainer(W *w) {
		aabbContainer.clear();
		const size_t S = w->bodies.size();
		aabbContainer.reserve(S);
		for (size_t i = 0; i < S; ++i) aabbContainer.push_back(AABB(w->bodies[i]));
	}

	template <typename W> void reinsertAllCellsInGrid(W *w) {
		assert(w->bodies.size() == c.bodies.size());
		refreshAABBContainer(w);
		grid.clear();
		const size_t S = w->bodies.size();
		for (size_t i = 0; i < S; ++i) {
			grid.insert(i, aabbContainer[i]);
		}
	}

	template <typename W> void reinsertAllCellsInGrid_withSample(W *w) {
		assert(w->bodies.size() == c.bodies.size());
		refreshActLvl(w);
		refreshAABBContainer(w);
		grid.clear();
		std::uniform_real_distribution<num_t> d(0.0, 1.0);
		const size_t S = w->bodies.size();
		for (size_t i = 0; i < S; ++i) {
			num_t diceRoll = d(Config::globalRand());
			if (diceRoll < std::max(minSampleSize, actLvl[i])) {
				grid.insert(i, aabbContainer[i]);
			}
		}
	}

	template <typename W> void refreshConstraints(W *w) {
		size_t prevConstraintsSize = constraints.size();
		constraints.clear();
		constraints.reserve(
		    static_cast<size_t>(static_cast<num_t>(prevConstraintsSize) * 1.1));
		auto &orderedVec = grid.getOrderedVec();

		for (auto &gridCellPair : orderedVec) {
#if USE_SIMPLE_GRID
			const auto &gridCell = grid.grid[gridCellPair];
#else
			const auto &gridCell = gridCellPair.second;
#endif
			const size_t GRIDSIZE = gridCell.size();
			for (size_t i = 0; i < GRIDSIZE; ++i) {
				auto &body_i = w->bodies[gridCell[i]];
				for (size_t j = i + 1; j < GRIDSIZE; ++j) {
					auto &body_j = w->bodies[gridCell[j]];
					if (!AABBCollisionEnabled ||
					    (AABBCollisionEnabled && grid.AABBCollision(aabbContainer[gridCell[i]],
					                                                aabbContainer[gridCell[j]]))) {
						// only one particle could be faster: store directly the particle in the
						// grid
						for (auto &p0 : body_i.particles) {
							for (auto &p1 : body_j.particles) {
								if (!precisePreCollisionCheck ||
								    (p0.predicted - p1.predicted).squaredNorm() <
								        std::pow(p0.radius + p1.radius, 2)) {
									constraints.push_back(std::make_tuple(
									    &p0, &p1,
									    std::min(body_i.distanceStiffness, body_j.distanceStiffness)));
								}
							}
						}
					}
				}
			}
		}
	}

	// template <typename W> void refreshConstraints(W *) {
	// size_t prevConstraintsSize = constraints.size();
	// constraints.clear();
	// constraints.reserve(
	// static_cast<size_t>(static_cast<num_t>(prevConstraintsSize) * 1.1));

	// auto &orderedVec = grid.getOrderedVec();
	// for (auto &gridCellPair : orderedVec) {
	//#if USE_SIMPLE_GRID
	// const auto &gridCell = gridCellPair.second.get();
	//#else
	// const auto &gridCell = gridCellPair.second;
	//#endif
	// const size_t GRIDSIZE = gridCell.size();
	// for (size_t i = 0; i < GRIDSIZE; ++i) {
	// for (size_t j = i + 1; j < GRIDSIZE; ++j) {
	// if (!AABBCollisionEnabled ||
	//(AABBCollisionEnabled &&
	// grid.AABBCollision(AABB(*gridCell[i]), AABB(*gridCell[j])))) {
	// for (auto &p0 : gridCell[i]->particles) {
	// for (auto &p1 : gridCell[j]->particles) {
	// if (!precisePreCollisionCheck ||
	//(p0.predicted - p1.predicted).squaredNorm() <
	// std::pow(p0.radius + p1.radius, 2)) {
	// constraints.push_back(
	// std::make_tuple(&p0, &p1,
	// std::min(gridCell[i]->distanceStiffness,
	// gridCell[j]->distanceStiffness)));
	//}
	//}
	//}
	//}
	//}
	//}
	//}
	/*}*/

	template <typename W> void updateParticles(W *w) {
		const auto &dt = w->getDt();
		for (auto &b : w->bodies)
			PBD::velocitiesAndPositionsUpdate(b.particles, dt, sleepingEpsilon);
	}

	template <typename W> void pbdUpdateRoutine(W *w) {
		const auto dt = w->getDt();

		for (auto &b : w->bodies) PBD::eulerIntegration(b.particles, dt);

		// generate collisions
		if (w->getNbUpdates() % constraintGenerationFreq == 0) {
			reinsertAllCellsInGrid_withSample(w);
			refreshConstraints(w);
		}

		// project constraints
		for (unsigned int i = 0; i < iterations; ++i) {
			for (auto &b : w->bodies) b.solveInnerConstraints();
			for (auto &con : constraintSolveHook) con();
			// collision constraints
			for (auto &c : constraints) {
				const auto &p0 = std::get<0>(c);
				const auto &p1 = std::get<1>(c);
				const auto &distStiff = std::get<2>(c);
				PBD::CollisionConstraint::solve(*p0, *p1, p0->radius + p1->radius, distStiff);
			}
		}
		for (auto &con : frictionSolveHook) con();
		if (frictionEnabled) {
			for (auto &c : constraints) {
				const auto &p0 = std::get<0>(c);
				const auto &p1 = std::get<1>(c);
				PBD::FrictionConstraint::solve(*p0, *p1, staticFrictionCoef, kineticFrictionCoef);
			}
		}
		updateParticles(w);
	}

	template <typename W> void endUpdate(W *w) { pbdUpdateRoutine(w); }
	// TODO : remove dead cells from grid if REINSERT is enabled
};  // namespace MecaCell
}  // namespace MecaCell
#endif
