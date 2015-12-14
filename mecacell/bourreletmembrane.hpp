#ifndef BOURRELETMEMBRANE_HPP
#define BOURRELETMEMBRANE_HPP
#include "movable.h"
#include "orientable.h"
#include "model.h"
#include <array>
// SOLUTION :
// pour le début, pas la peine de réellement simuler les bourrelets
// une simple expansion progressive du rayon (en fction du volume et de la surface totale)
// suffira !! ça simplifie bcp bcp de choses.
// /!\ : penser à détecter quand l'augmentation du rayon ne change plus rien
// (quand la cellule est entièrement encerclée), histoire de pas augmenter pour rien
// aussi je sais pas si ça peut servir mais le rapport entre la variation du rayon et la variation 
// du volume donne une indication sur le volume disponible autour de la cellule...

namespace MecaCell {
template <typename Cell, unsigned int N = 20> class BourreletMembrane {
	struct CollisionPlane {
		Cell *other = nullptr;
		double midpoint = 0;
		double radius = 0;
		Vec normal;  // normal oriented outward to this cell's center
	};
	Cell *cell;
	vector<CollisionPlane> colPlanes;

	bool collisionPlaneIntersect(const CollisionPlane p0, const CollisionPlane &p1) {
		// on a 2 cercles A et B. On cherche à savoir s'ils s'intersectent
		// => projeté du centre de A sur le plan de B est < au rayon de A et vice versa.
		Vec Acenter = cell->getPosition() + p0.midpoint * p0.normal;
		Vec Bcenter = cell->getPosition() + p1.midpoint * p1.normal;
		return fabs((Acenter - Bcenter).dot(p1.normal)) < p0.radius &&
		       fabs((Bcenter - Acenter).dot(p0.normal)) < p1.radius;
	}

	int getNbBourrelets() {
		if (colPlanes.size() == 0) return 0;
		if (colPlanes.size() <= 2) return 1;
		vector<std::pair<size_t, size_t>> intersectingPairs;
		for (size_t i = 0; i < colPlanes.size(); ++i) {
			for (size_t j = i; j < colPlanes.size(); ++j) {
				if (collisionPlaneIntersect(colPlanes[i], colPlanes[j])) {
					intersectingPairs.emplace_back(i, j);
				}
			}
		}
		// now we have all the intersecting pairs
		if (intersectingPairs.size() <= 2) return 1;
		// the result is the number of shortest cycles greater than 2
		// and where there is only no intersection betweeen intersections
	};

 public:
	// connections containers
	using CellCellConnectionContainer = vector<int>;
	using CellModelConnectionContainer = vector<int>;
	using CCCM = BourreletMembrane;  // cell cell connections manager
	using CMCM = BourreletMembrane;  // cell model connections manager

	static constexpr bool forcesOnMembrane = false;
	// constructors
	BourreletMembrane(Cell *c) : cell(c) {}

	BourreletMembrane(const BourreletMembrane &) {}

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
	                                        CellCellConnectionContainer &,
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
