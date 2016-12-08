#ifndef MECACELL_GENERICCONNECTIONPLUGIN_HPP
#define MECACELL_GENERICCONNECTIONPLUGIN_HPP
#include <future>
#include <memory>
#include <utility>
#include <vector>
#include "utilities/grid.hpp"
#include "utilities/ordered_hash_map.hpp"
#include "utilities/ordered_pair.hpp"

namespace MecaCell {
/**
 * @brief Embedded plugin that will handle the collision detection routines. Parallel
 * code.
 */
template <typename Cell, template <class> class GenericConnection>
struct GenericConnectionBodyPlugin {
	const size_t MIN_CHUNK_SIZE = 5;
	const size_t AVG_TASKS_PER_THREAD = 3;
	ordered_hash_map<ordered_pair<Cell *>,
	                 std::unique_ptr<GenericConnection<Cell>>>
	    connections;  /// where we keep track of all the
	/// connections. The ordered_hash_map is to enforce determinism.

	double currentAvgCellSize = 40.0;  /// used to size the space partitioning grid
	double currentMaxCellSize = 40.0;  /// used to size the space partitioning grid
	const double gridCellRatio = 4.0;  /// grid size relative to the currentAvgCellSize
	std::mutex connectionMutex;

	/* *******
	 * HOOKS
	 * *******/
	template <typename W> void preBehaviorUpdate(W *w) {
		// logger<INF>("pre 0");
		updateCellCellConnections(*w);
		w->threadpool.autoChunks(w->cells, MIN_CHUNK_SIZE, AVG_TASKS_PER_THREAD,
		                         [dt = w->getDt()](auto &c) { c->body.updateInternals(dt); });
		w->threadpool.waitUntilLast();
		// logger<INF>("pre 1");
	}
	template <typename W> void postBehaviorUpdate(W *w) {
		// logger<INF>("post 0");
		checkForCellCellConnections(*w);
		w->threadpool.autoChunks(
		    w->cells, MIN_CHUNK_SIZE, AVG_TASKS_PER_THREAD,
		    [dt = w->getDt()](auto &c) { c->body.updatePositionsAndOrientations(dt); });
		w->threadpool.waitUntilLast();
		for (auto &c : w->cells) {
			c->resetForce();
			c->body.resetTorque();
		}
		// logger<INF>("post 1");
	}

	////////////////////////////////////////////////////////////////////////////
	/**
	 * @brief create and register a contact surface connection between twoe cells
	 *
	 * @param cells the cells that are in contact
	 */
	void createConnection(const ordered_pair<Cell *> &cells) {
		connections[cells] = std::make_unique<GenericConnection<Cell>>(cells);
		cells.first->connectedCells.insert(cells.second);
		cells.second->connectedCells.insert(cells.first);
		auto *newConnection = connections.at(cells).get();
		cells.first->body.cellConnections.push_back(newConnection);
		cells.second->body.cellConnections.push_back(newConnection);
	}
	/**
	 * @brief Disconnect two cells
	 *
	 * @param cells the pair of cell to disconnect
	 * @param conn the connection ptr
	 */
	void disconnect(const ordered_pair<Cell *> &cells, GenericConnection<Cell> *conn) {
		eraseFromVector(conn, cells.first->body.cellConnections);
		eraseFromVector(conn, cells.second->body.cellConnections);
		cells.first->eraseConnectedCell(cells.second);
		cells.second->eraseConnectedCell(cells.first);
		connections.erase(cells);
	}
	/**
	 * @brief we check for any cell cell collision or adhesion
	 *
	 * @tparam W world type
	 * @param world world instance
	 */
	template <typename W> void checkForCellCellConnections(W &world) {
		// TODO: try with mutexes instead of colored grid (every batch is a task but there is
		// a shared_mutex for newConnections)
		const double NEW_CONNECTION_THRESHOLD = 1.0 - 1e-10;
		Grid<Cell *> grid(std::max(currentAvgCellSize * gridCellRatio, currentMaxCellSize));
		for (const auto &c : world.cells) grid.insert(c);
		auto gridCells = grid.getThreadSafeGrid();
		for (auto &color : gridCells) {
			std::vector<std::future<std::vector<ordered_pair<Cell *>>>>
			    newConnectionsFutures;  // we collect all them futures
			for (auto &batch : color)
				newConnectionsFutures.push_back(world.threadpool.enqueueWithFuture([&] {
					std::vector<ordered_pair<Cell *>> newConns;
					for (size_t i = 0; i < batch.size(); ++i) {
						for (size_t j = i + 1; j < batch.size(); ++j) {
							auto op = make_ordered_cell_pair(batch[i], batch[j]);
							Vec AB = op.second->position - op.first->position;
							double sqDistance = AB.sqlength();
							if (sqDistance < std::pow(op.first->body.getBoundingBoxRadius() +
							                              op.second->body.getBoundingBoxRadius(),
							                          2)) {
								if (!op.first->isConnectedTo(op.second) && op.first != op.second) {
									double dist = sqrt(sqDistance);
									Vec dir = AB / dist;
									auto d0 = op.first->body.getPreciseMembraneDistance(dir);
									auto d1 = op.second->body.getPreciseMembraneDistance(-dir);
									if (dist < NEW_CONNECTION_THRESHOLD * (d0 + d1)) newConns.push_back(op);
								}
							}
						}
					}
					return newConns;
				}));
			for (auto &ncf : newConnectionsFutures)  // and wait for their accomplishment
				for (const auto &nc : ncf.get()) createConnection(nc);
		}
	}

	/**
	 * @brief  updates (compute forces) the connections and delete the resolved ones (when
	 * cells are not in contact anymore)
	 *
	 * @param w the world
	 * @param W world type
	 */
	template <typename W> void updateCellCellConnections(W &w) {
		std::vector<std::pair<ordered_pair<Cell *>, GenericConnection<Cell> *>> toDisconnect;
		for (auto &c : connections) {
			c.second->update(w.getDt());
		}
		for (auto &con : connections)
			if (!con.second->fixedAdhesion && con.second->area <= 0)
				toDisconnect.push_back(std::make_pair(con.first, con.second.get()));
		for (auto &c : toDisconnect) disconnect(c.first, c.second);
	}

	template <typename W> void preDeleteDeadCellsUpdate(W *w) {
		for (auto &c : w->cells) {
			if (c->isDead()) {
				auto cctmp(c->body.cellConnections);
				for (auto &connection : cctmp) {
					auto *c0 = connection->cells.first;
					auto *c1 = connection->cells.second;
					eraseFromVector(connection, c0->body.cellConnections);
					eraseFromVector(connection, c1->body.cellConnections);
					c0->connectedCells.erase(c1);
					c1->connectedCells.erase(c0);
					assert(c0->id != c1->id);
					connections.erase(connection->cells);
				}
			}
		}
	}
};
}

#endif
