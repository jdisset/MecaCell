#ifndef CELLCELLCONNECTIONMANAGER_HPP
#define CELLCELLCONNECTIONMANAGER_HPP
#include "tools.h"
#include "connection.h"
#include "model.h"
#include "modelconnection.hpp"
#include <unordered_map>
#include <utility>
#include <vector>

#undef DBG
#define DBG DEBUG(CCCM)
namespace MecaCell {
template <typename Cell> struct CellCellConnectionManager_map {
	using ConnectionType = Connection<Cell *>;
	using ModelConnectionType = CellModelConnection<Cell>;
	using CellCellConnectionContainer = unordered_map<ordered_pair<Cell *>, ConnectionType>;

	vector<ConnectionType *> cellConnections;
	// connections creation
	// template<typename... Args>
	template <typename... Args>
	static inline void createConnection(CellCellConnectionContainer &container,
	                                    ordered_pair<Cell *> cells,
	                                    Args &&... connectionArgs) {
		container.emplace(
		    make_pair(cells, ConnectionType(make_pair(cells.first, cells.second),
		                                    std::forward<Args>(connectionArgs)...)));
		cells.first->connectedCells.insert(cells.second);
		cells.second->connectedCells.insert(cells.first);
		ConnectionType *newConnection = &container.at(cells);
		cells.first->membrane.cccm.cellConnections.push_back(newConnection);
		cells.second->membrane.cccm.cellConnections.push_back(newConnection);
		newConnection->updateLengthDirection();
	}
	// deletions, should be ok...
	static inline void disconnect(CellCellConnectionContainer &container, Cell *c0,
	                              Cell *c1, ConnectionType *connection = nullptr) {
		// c0->membrane.cccm.cellConnections.erase(c1);
		// c1->membrane.cccm.cellConnections.erase(c0);
		eraseFromVector(connection, c0->membrane.cccm.cellConnections);
		eraseFromVector(connection, c1->membrane.cccm.cellConnections);
		c0->connectedCells.erase(c1);
		c1->connectedCells.erase(c0);
		container.erase(make_ordered_cell_pair(c0, c1));
	}
	// Accessors for iterations
	static inline ConnectionType &getConnection(
	    pair<const ordered_pair<Cell *>, ConnectionType> &connIteration) {
		return connIteration.second;
	}
	static inline ConnectionType &getConnection(
	    const pair<Cell *, ConnectionType *> &connIteration) {
		return *(connIteration.second);
	}
	static inline ConnectionType &getConnection(ConnectionType *connIteration) {
		return *connIteration;
	}

	static inline bool areConnected(Cell *c0, Cell *c1) {
		return c0->connectedCells.count(c1);
	}
};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

template <typename Cell> struct CellCellConnectionManager_vector {
	using ConnectionType = Connection<Cell *>;
	using ModelConnectionType = CellModelConnection<Cell>;
	using CellCellConnectionContainer = vector<ConnectionType *>;

	vector<ConnectionType *> cellConnections;
	template <typename... Args>
	static inline void createConnection(CellCellConnectionContainer &container, Cell *c0,
	                                    Cell *c1, Args &&... connectionArgs) {
		c0->connectedCells.insert(c1);
		c1->connectedCells.insert(c0);
		ConnectionType *newConnection =
		    new ConnectionType(std::forward<Args>(connectionArgs)...);
		container.push_back(newConnection);
		c0->membrane.cccm.cellConnections.push_back(newConnection);
		c1->membrane.cccm.cellConnections.push_back(newConnection);
		newConnection->updateLengthDirection();
	}
	static inline void disconnect(CellCellConnectionContainer &container, Cell *c0,
	                              Cell *c1) {
		ConnectionType *connection = nullptr;
		for (auto con : c0->membrane.cccm.cellConnections) {
			if (make_ordered_pair(c0, c1) ==
			    make_ordered_pair(con->getNode0(), con->getNode1())) {
				disconnect(container, c0, c1, con);
				break;
			}
		}
	}

	static inline void disconnect(CellCellConnectionContainer &container, Cell *c0,
	                              Cell *c1, ConnectionType *connection) {
		eraseFromVector(connection, c0->membrane.cccm.cellConnections);
		eraseFromVector(connection, c1->membrane.cccm.cellConnections);
		eraseFromVector(connection, container);
		delete connection;
		c0->connectedCells.erase(c1);
		c1->connectedCells.erase(c0);
	}
	// Accessors for iterations
	static inline ConnectionType &getConnection(ConnectionType *connIteration) {
		return *connIteration;
	}

	static inline bool areConnected(Cell *c0, Cell *c1) {
		return c0->connectedCells.count(c1);
	}
};
}
#endif
