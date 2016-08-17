#ifndef CELLCELLCONNECTIONMANAGER_HPP
#define CELLCELLCONNECTIONMANAGER_HPP
#include <utility>
#include <vector>
#include "utilities/ordered_hash_map.hpp"
#include "utilities/ordered_pair.hpp"
#include "utilities/utils.h"

namespace MecaCell {
template <typename Cell, typename CCC>
struct CellCellConnectionManager_map {
	using ConnectionType = CCC;
	using CellCellConnectionContainer =
	    ordered_hash_map<ordered_pair<Cell *>, std::unique_ptr<ConnectionType>>;

	vector<ConnectionType *> cellConnections;
	// connections creation
	template <typename... Args>
	static inline void createConnection(CellCellConnectionContainer &container,
	                                    ordered_pair<Cell *> cells,
	                                    Args &&... connectionArgs) {
		container[cells] = std::unique_ptr<ConnectionType>(
		    new ConnectionType(std::forward<Args>(connectionArgs)...));
		cells.first->connectedCells.insert(cells.second);
		cells.second->connectedCells.insert(cells.first);
		ConnectionType *newConnection = container.at(cells).get();
		cells.first->membrane.cccm.cellConnections.push_back(newConnection);
		cells.second->membrane.cccm.cellConnections.push_back(newConnection);
	}
	// deletions, should be ok...
	static inline void disconnect(CellCellConnectionContainer &container, Cell *c0,
	                              Cell *c1, ConnectionType *connection) {
		eraseFromVector(connection, c0->membrane.cccm.cellConnections);
		eraseFromVector(connection, c1->membrane.cccm.cellConnections);
		c0->connectedCells.erase(c1);
		c1->connectedCells.erase(c0);
		assert(c0->id != c1->id);
		container.erase(make_ordered_cell_pair(c0, c1));
	}
	// Accessors for iterations
	static inline ConnectionType *getConnection(
	    pair<ordered_pair<Cell *>, std::unique_ptr<ConnectionType>> &connIteration) {
		return connIteration.second.get();
	}
	static inline ConnectionType *getConnection(
	    const pair<Cell *, ConnectionType *> &connIteration) {
		return connIteration.second;
	}
	static inline ConnectionType *getConnection(ConnectionType *connIteration) {
		return connIteration;
	}

	static inline bool areConnected(Cell *c0, Cell *c1) {
		return c0->connectedCells.count(c1);
	}
};
}
#endif
