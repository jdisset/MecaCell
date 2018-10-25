#ifndef MECACELL_PBD_PLUGIN_HPP
#define MECACELL_PBD_PLUGIN_HPP
#include "integrators.hpp"
#include "pbd.hpp"
#include "utilities/grid.hpp"

//#define REINSERT

namespace MecaCell {
template <typename Cell> struct PBDPlugin {
	// supposed to work with a PBDBody

	bool reinsertCellsInGrid = true;
	using AABB_t = typename Grid<Cell *>::AABB_t;
	size_t iterations = 3;
	size_t constraintGenerationFreq = 1;
	double gridSize = 150;
	Grid<Cell *> grid{gridSize};

	PBD::ConstraintContainer<PBD::CollisionConstraint<Vec>> constraints;
	std::unordered_set<ordered_pair<Cell *>> constraintsMap;
	size_t prevCollConstraintsSize = 0;

	std::unordered_map<Cell *, AABB_t> BBMap;
	std::unordered_map<Cell *, num_t> pressureMap;

	void setGridSize(double g) {
		gridSize = g;
		grid = Grid<Cell *>(gridSize);
	}

	template <typename W> void updateGrid(W *w) {
		// update the grid without clearing it
		for (const auto &c : w->cells) {
			if (!BBMap.count(c)) {  // new cell
				auto AABB = grid.getAABB(c);
				BBMap[c] = AABB;
				grid.insert(c, AABB);
			} else {  // cell is already in the grid
				auto AABB = grid.getAABB(c);
				auto prevAABB = BBMap[c];
				if (prevAABB != AABB) {
					// cell moved significantly
					grid.remove(c, prevAABB);
					grid.insert(c, AABB);
					BBMap[c] = AABB;
				}
			}
		}
	}

	template <typename W> void reinsertAllCellsInGrid(W *w) {
		grid.clear();
		for (const auto &c : w->cells) grid.insert(c);
	}

	template <typename W> void refreshConstraints(W *w) {
		// constraintsMap.clear();
		constraints.clear();
		constraints.reserve<PBD::CollisionConstraint<Vec>>(
		    static_cast<size_t>(static_cast<double>(prevCollConstraintsSize) * 1.1));

		pressureMap.clear();
		for (auto &c : w->cells) pressureMap[c] = 0;

		auto &orderedVec = grid.getOrderedVec();
		for (auto &gridCellPair : orderedVec) {
			const auto &gridCell = gridCellPair.second;
			for (size_t i = 0; i < gridCell.size(); ++i) {
				for (size_t j = i + 1; j < gridCell.size(); ++j) {
					pressureMap[gridCell[i]]++;
					pressureMap[gridCell[j]]++;
					// if (grid.AABBCollision(gridCell[i], gridCell[j])) {
					// ordered_pair<Cell *> op{gridCell[i], gridCell[j]};
					// if (!constraintsMap.count(op)) {
					// constraintsMap.insert(op);
					for (auto &p0 : gridCell[i]->getBody().particles) {
						for (auto &p1 : gridCell[j]->getBody().particles) {
							constraints.addConstraint(PBD::CollisionConstraint<Vec>(
							    {{&p0.predicted, &p1.predicted}}, {{p0.w, p1.w}}, p0.radius + p1.radius,
							    std::min(gridCell[i]->getBody().constraintStiffness,
							             gridCell[j]->getBody().constraintStiffness)));
						}
						//}
						//}
					}
				}
			}
		}
		prevCollConstraintsSize = constraints.size<PBD::CollisionConstraint<Vec>>();
	}

	template <typename W> void solveConstraints(W *w) {
		const auto &dt = w->getDt();
		for (auto &c : w->cells) PBD::eulerIntegration(c->body.particles, dt);
		for (unsigned int i = 0; i < iterations; ++i) {
			for (auto &c : w->cells) c->body.solveInnerConstraints();
			PBD::projectConstraints(constraints);
		}
	}

	template <typename W> void updateParticles(W *w) {
		const auto &dt = w->getDt();
		for (auto &c : w->cells) {
			PBD::velocitiesAndPositionsUpdate(c->body.particles, dt);
			c->getBody().resetForces();
		}
	}
	template <typename W> void endUpdate(W *w) {
		if (w->getNbUpdates() % constraintGenerationFreq == 0) {
			if (reinsertCellsInGrid)
				reinsertAllCellsInGrid(w);
			else
				updateGrid(w);
		}
		refreshConstraints(w);
		solveConstraints(w);
		updateParticles(w);
	}

	// TODO : remove dead cells from grid if REINSERT is enabled
};
}  // namespace MecaCell
#endif
