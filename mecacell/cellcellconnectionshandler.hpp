#ifndef CELLCELLCONNECTIONHANDLER_HPP
#define CELLCELLCONNECTIONHANDLER_HPP
#include <utility>
#include <vector>
#include "grid.hpp"

template <typename W> struct CellCellConnectionsHandler {
	using cell_t = typename W::cell_t;
	using CellCellConnectionContainer = typename cell_t::CellCellConnectionContainer;

	static constexpr double DEFAULT_CELL_RADIUS = 40;

	Grid<cell_t *> cellSpacePartition =
	    Grid<cell_t *>(4.5 * DEFAULT_CELL_RADIUS);  // space partition hashmap for cells

	W *world;
	CellCellConnectionsHandler(W *w) : world(w) {}

	std::vector<std::pair<cell_t *, cell_t *>> getConnectedCellsList() {
		unique_vector<ordered_pair<cell_t *>> uniquePairs;
		for (auto &c : world->cells) {
			for (auto &other : c->getConnectedCells())
				uniquePairs.insert(make_ordered_cell_pair(c, other));
		}
		std::vector<std::pair<cell_t *, cell_t *>> result;
		for (auto &p : uniquePairs) result.push_back(make_pair(p.first, p.second));
		return result;
	}
	size_t getNbOfCellCellConnections() { return getConnectedCellsList().size(); }
};
#endif
