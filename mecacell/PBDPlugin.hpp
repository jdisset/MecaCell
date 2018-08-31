#ifndef MECACELL_PBD_PLUGIN_HPP
#define MECACELL_PBD_PLUGIN_HPP
#include "integrators.hpp"
#include "pbd.hpp"
#include "utilities/grid.hpp"

namespace MecaCell {
template <typename Cell> struct PBDPlugin {
	// supposed to work with a PBDBody

	using AABB_t = typename Grid<Cell *>::AABB_t;

	double GRIDSIZE = 130;
	double AVG_CONSTRAINTS_PER_GRIDCELL = 200;

	PBD::ConstraintContainer<PBD::CollisionConstraint<Vec>> constraints;
	size_t iterations = 3;
	Grid<Cell *> grid{GRIDSIZE};

	//std::unordered_map<Cell *, AABB_t> BBMap;

	template <typename W> void endUpdate(W *w) {
		constraints.clear();
		grid.clear();
		auto &orderedVec = grid.getOrderedVec();
		for (const auto &c : w->cells) grid.insert(c);
		constraints.reserve<PBD::CollisionConstraint<Vec>>(orderedVec.size() *
		                                                   AVG_CONSTRAINTS_PER_GRIDCELL);
		for (auto &gridCellPair : orderedVec) {
			const auto &gridCell = gridCellPair.second;
			for (size_t i = 0; i < gridCell.size(); ++i) {
				for (size_t j = i + 1; j < gridCell.size(); ++j) {
					if (grid.AABBCollision(gridCell[i], gridCell[j])) {
						for (auto &p0 : gridCell[i]->getBody().particles) {
							for (auto &p1 : gridCell[j]->getBody().particles) {
								constraints.addConstraint(PBD::CollisionConstraint<Vec>(
								    {{&p0.predicted, &p1.predicted}}, {{p0.w, p1.w}},
								    p0.radius + p1.radius, 0.001));
							}
						}
					}
				}
			}
		}
		auto dt = w->getDt();
		for (auto &c : w->cells) PBD::eulerIntegration(c->body.particles, dt);
		for (unsigned int i = 0; i < iterations; ++i) {
			for (auto &c : w->cells) c->body.solveInnerConstraints();
			PBD::projectConstraints(constraints);
		}
		for (auto &c : w->cells) {
			PBD::velocitiesAndPositionsUpdate(c->body.particles, dt);
			c->getBody().resetForces();
		}
	}
};
}  // namespace MecaCell
#endif
