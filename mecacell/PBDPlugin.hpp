#ifndef MECACELL_PBD_PLUGIN_HPP
#define MECACELL_PBD_PLUGIN_HPP
#include "integrators.hpp"
#include "pbd.hpp"
#include "utilities/grid.hpp"

//#define REINSERT

namespace MecaCell {
template <typename Cell> struct PBDPlugin {
	// supposed to work with a PBDBody

	bool cohesionEnabled = true;
	bool frictionEnabled = true;
	bool reinsertCellsInGrid = true;
	bool AABBCollisionEnabled = true;
	bool precisePreCollisionCheck = false;
	using AABB_t = typename Grid<Cell *>::AABB_t;
	size_t iterations = 1;
	size_t constraintGenerationFreq = 1;
	num_t sleepingEpsilon = 1e-5;
	num_t gridSize = 150;
	num_t minSampleSize = 0.1;
	num_t kineticFrictionCoef = 0.0;
	num_t staticFrictionCoef = 0.0;
	num_t cohesionStiffness = 0.0;
	Grid<Cell *> grid{gridSize};

	using collision_c = PBD::CollisionConstraint<Vec>;
	using fixed_c = PBD::FixedToPointConstraint<PBD::Particle<Vec>>;
	using friction_c = PBD::FrictionConstraint<PBD::Particle<Vec>>;
	PBD::ConstraintContainer<collision_c> collisionConstraints;
	PBD::ConstraintContainer<fixed_c> cohesionConstraints;
	PBD::ConstraintContainer<friction_c> frictionConstraints;

	size_t prevCollConstraintsSize = 0;

	std::vector<std::function<void(void)>> constraintSolveHook;
	std::vector<std::function<void(void)>> frictionSolveHook;

	std::unordered_map<Cell *, AABB_t> BBMap;

	void setGridSize(num_t g) {
		gridSize = g;
		grid = Grid<Cell *>(gridSize);
	}

	// helper to get the grid discretized Axis Aligned Bounding Box of a cell
	typename Grid<Cell *>::AABB_t inline AABB(Cell *c) {
		return grid.getAABB(c->getBody().getAABB());
	}

	template <typename W> void updateGrid(W *w) {
		// update the grid without clearing it
		for (const auto &c : w->cells) {
			if (!BBMap.count(c)) {  // new cell
				auto AABBox = AABB(c);
				BBMap[c] = AABBox;
				grid.insert(c, AABBox);
			} else {  // cell is already in the grid
				auto AABBox = AABB(c);
				auto prevAABBox = BBMap[c];
				if (prevAABBox != AABBox) {
					// cell moved significantly
					grid.remove(c, prevAABBox);
					grid.insert(c, AABBox);
					BBMap[c] = AABBox;
				}
			}
		}
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
		for (const auto &c : w->cells) {
			auto bb = AABB(c);
			grid.insert(c, bb);
		}
	}

	template <typename W> void reinsertAllCellsInGrid_withSample(W *w) {
		grid.clear();
		std::uniform_real_distribution<num_t> d(0.0, 1.0);
		for (const auto &c : w->cells) {
			num_t diceRoll = d(Config::globalRand());
			if (diceRoll < std::max(minSampleSize, c->activityLevel)) grid.insert(c, AABB(c));
		}
	}

	template <typename W> void refreshConstraints(W *) {
		collisionConstraints.clear();
		frictionConstraints.clear();
		collisionConstraints.reserve<collision_c>(
		    static_cast<size_t>(static_cast<num_t>(prevCollConstraintsSize) * 1.1));
		frictionConstraints.reserve<friction_c>(
		    static_cast<size_t>(static_cast<num_t>(prevCollConstraintsSize) * 1.1));
		auto &orderedVec = grid.getOrderedVec();
		for (auto &gridCellPair : orderedVec) {
			const auto &gridCell = gridCellPair.second;

			// compute COM for cohesion
			Vec COM = Vec::zero();
			if (cohesionEnabled) {
				for (auto &c : gridCell) COM += c->getBody().getCOM();
				if (gridCell.size() > 0) COM = COM / static_cast<num_t>(gridCell.size());
			}

			for (size_t i = 0; i < gridCell.size(); ++i) {
				if (cohesionEnabled) {
					for (auto &p : gridCell[i]->getBody().particles)
						cohesionConstraints.addConstraint(fixed_c(p, COM, cohesionStiffness));
				}

				for (size_t j = i + 1; j < gridCell.size(); ++j) {
					if (!AABBCollisionEnabled ||
					    (AABBCollisionEnabled &&
					     grid.AABBCollision(AABB(gridCell[i]), AABB(gridCell[j])))) {
						// TODO: finer AABB by using particles AABB
						for (auto &p0 : gridCell[i]->getBody().particles) {
							for (auto &p1 : gridCell[j]->getBody().particles) {
								if (!precisePreCollisionCheck ||
								    (p0.predicted - p1.predicted).squaredNorm() <
								        std::pow(p0.radius + p1.radius, 2)) {
									collisionConstraints.addConstraint(
									    collision_c({{&p0.predicted, &p1.predicted}}, {{p0.w, p1.w}},
									                p0.radius + p1.radius,
									                std::min(gridCell[i]->getBody().distanceStiffness,
									                         gridCell[j]->getBody().distanceStiffness)));

									// friction
									if (frictionEnabled)
										frictionConstraints.addConstraint(
										    friction_c(p0, p1, staticFrictionCoef, kineticFrictionCoef));
								}
							}
						}
					}
				}
			}
		}
		prevCollConstraintsSize = collisionConstraints.size<PBD::CollisionConstraint<Vec>>();
	}

	template <typename W> void updateParticles(W *w) {
		const auto &dt = w->getDt();
		for (auto &c : w->cells) {
			PBD::velocitiesAndPositionsUpdate(c->body.particles, dt, sleepingEpsilon);
			c->getBody().resetForces();
		}
	}

	template <typename W> void pbdUpdateRoutine(W *w) {
		const auto &dt = w->getDt();

		// euler
		for (auto &c : w->cells) {
			PBD::eulerIntegration(c->body.particles, dt);
			c->getBody().resetForces();
		}

		// generate collisions
		if (w->getNbUpdates() % constraintGenerationFreq == 0) {
			if (reinsertCellsInGrid)
				reinsertAllCellsInGrid_withSample(w);
			else
				updateGrid(w);
			refreshConstraints(w);
		}

		// project constraints
		for (unsigned int i = 0; i < iterations; ++i) {
			for (auto &c : w->cells) c->body.solveInnerConstraints();
			for (auto &con : constraintSolveHook) con();
			if (cohesionEnabled) PBD::projectConstraints(cohesionConstraints);
			PBD::projectConstraints(collisionConstraints);
		}
		for (auto &con : frictionSolveHook) con();
		if (frictionEnabled) PBD::projectConstraints(frictionConstraints);
		updateParticles(w);
	}

	template <typename W> void endUpdate(W *w) { pbdUpdateRoutine(w); }
	// TODO : remove dead cells from grid if REINSERT is enabled
};
}  // namespace MecaCell
#endif
