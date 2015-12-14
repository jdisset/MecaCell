#ifndef GASMEMBRANE_HPP
#define GASMEMBRANE_HPP
#include "movable.h"
#include "orientable.h"
#include "model.h"
#include <array>

namespace MecaCell {
struct MCP : public Movable, Orientable {  // membrane control points
	MCP(Vec p) : Movable(p) {}
};
template <typename Cell, unsigned int N = 20> class DeformableMembrane {
	struct CollisionPlane {
		ordered_pair<Cell> cells;

	};
	std::array<MCP, N> mcps;
	Cell *cell;

 public:
	// connections containers
	using CellCellConnectionContainer = vector<int>;
	using CellModelConnectionContainer = vector<int>;
	using CCCM = DeformableMembrane;  // cell cell connections manager
	using CMCM = DeformableMembrane;  // cell model connections manager

	static constexpr bool forcesOnMembrane = false;
	// constructors
	DeformableMembrane(Cell *c) : cell(c) {
		auto vec = getSpherePointsPacking(N);
		for (size_t i = 0; i < vec.size(); ++i) {
			mcps[i] = MCP(vec[i]);
		}
	}

	DeformableMembrane(const DeformableMembrane &) {}

	// updates
	template <typename Integrator> void updatePositionsAndOrientations(double dt) {
		// center's position is computed after MCP are atualised (it's just the average)
	}
	void updateStats(){};
	void resetForces(){};

	// collisions & connections
	template <typename SpacePartition>
	static void checkForCellModelCollisions(vector<Cell *> &, unordered_map<string, Model>,
	                                        CellModelConnectionContainer &,
	                                        SpacePartition &) {}
	template <typename SpacePartition>
	static void checkForCellCellConnections(vector<Cell *> &cells,
	                                        CellCellConnectionContainer &connections,
	                                        SpacePartition &grid) {
		unordered_set<ordered_pair<Cell *>> newConnections;
		grid.clear();
		for (const auto &c : cells) grid.insert(c);
		auto gridCells = grid.getThreadSafeGrid();
		for (auto &batch : gridCells) {
			for (size_t i = 0; i < batch.size(); ++i) {
				for (size_t j = 0; j < batch[i].size(); ++j) {
					for (size_t k = j + 1; k < batch[i].size(); ++k) {
						auto op = make_ordered_cell_pair(batch[i][j], batch[i][k]);
						Vec AB = op.second->getPosition() - op.first->getPosition();
						if (pow(0.5 * (op.first.getBoundingBoxRadius() +
						               op.second.getBoundingBoxRadius()),
						        2) < AB.sqlength()) {
							// probable collision
							float_t L = AB.length();
							Vec ABnorm = AB / L;
							float_t exactA = op.getPreciseMembraneDistance(ABnorm);
							float_t exactB = op.getPreciseMembraneDistance(-ABnorm);
							if (L < exactA + exactB) {
								// collision !
								// we create a new collision plane
							}
						}
					}
				}
			}
		}
	}
	static void updateCellModelConnections(CellModelConnectionContainer &, float_t) {}
	static void updateCellCellConnections(CellCellConnectionContainer &, float_t) {}
	static inline void disconnectAndDeleteAllConnections(Cell *,
	                                                     CellCellConnectionContainer &) {}
	// get
	float_t getPressure() const { return 1.0; }
	float_t getBoundingBoxRadius() const { return 1.0; }
	float_t getPreciseMembraneDistance(const Vec &) const { return 1.0; }
	float_t getVolume() const { return 1.0; }
	float_t getBaseVolume() const { return 1.0; }
};
}
#endif
