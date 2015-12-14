#ifndef DUMBMEMBRANE_HPP
#define DUMBMEMBRANE_HPP
#include "model.h"

namespace MecaCell {
template <typename Cell> class DumbMembrane {
 public:
	// connections containers
	using CellCellConnectionContainer = vector<int>;
	using CellModelConnectionContainer = vector<int>;
	using CCCM = DumbMembrane;  // cell cell connections manager
	using CMCM = DumbMembrane;  // cell model connections manager

	static constexpr bool forcesOnMembrane = false;
	// constructors
	DumbMembrane(Cell *) {}
	DumbMembrane(const DumbMembrane &) {}

	// updates
	template <typename Integrator> void updatePositionsAndOrientations(double dt) {}
	void updateStats(){};
	void resetForces(){};

	// collisions & connections
	template <typename SpacePartition>
	static void checkForCellModelCollisions(vector<Cell *> &, unordered_map<string, Model>,
	                                        CellModelConnectionContainer &,
	                                        SpacePartition &) {}
	template <typename SpacePartition>
	static void checkForCellCellConnections(vector<Cell *> &, CellCellConnectionContainer &,
	                                        SpacePartition &) {}
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
