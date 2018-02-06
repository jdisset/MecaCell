#ifndef MECACELL_GENERICCONNECTIONPLUGIN_HPP
#define MECACELL_GENERICCONNECTIONPLUGIN_HPP
#include <future>
#include <memory>
#include <utility>
#include <vector>
#include "utilities/grid.hpp"
#include "utilities/ordered_hash_map.hpp"
#include "utilities/ordered_pair.hpp"
//#include "utilities/external/tsl/ordered_map.h"

namespace MecaCell {
/**
 * @brief Embedded plugin that will handle the collision detection routines. Parallel
 * code.
 */
static double GRIDSIZE = 120;
template <typename Cell, template <class> class GenericConnection>
struct GenericConnectionBodyPlugin {
	const size_t MIN_CHUNK_SIZE = 5;
	const size_t AVG_TASKS_PER_THREAD = 3;
	// tsl::ordered_map<ordered_pair<Cell *>, std::unique_ptr<GenericConnection<Cell>>>
	// connections;
	ordered_hash_map<ordered_pair<Cell *>,
	                 std::unique_ptr<GenericConnection<Cell>>>
	    connections;  /// where we keep track of all the
	/// connections. The ordered_hash_map is to enforce determinism.

	std::mutex connectionMutex;

	bool collisionCheck = true;

	// a callback called at every new connection
	std::function<void(GenericConnection<Cell> *)> newConnectionCallback =
	    [](GenericConnection<Cell> *) {};

	/**
	 * @brief Sets a callback function to be called at each connection creation. Can be used
	 * to override the default connection parameters.
	 *
	 * @param f a function taking a pointer to the newly created GenericConnection. Void
	 * return type.
	 */
	void setNewConnectionCallback(std::function<void(GenericConnection<Cell> *)> f) {
		newConnectionCallback = f;
	}

	/* *******
	 * HOOKS
	 * *******/
	template <typename W> void preBehaviorUpdate(W *w) {
		w->threadpool.autoChunks(w->cells, 100, 5,
		                         [dt = w->getDt()](auto &c) { c->body.updateInternals(dt); });
		w->threadpool.waitUntilLast();
	}

	template <typename W> void endUpdate(W *w) {
		unsigned int nbIterations = 2;

		if (collisionCheck) checkForCellCellConnections(*w);

		std::vector<std::pair<Vec, Vec>> savedForces;
		savedForces.reserve(w->cells.size());  // first we save the cell forces and torque
		for (auto &c : w->cells) {
			savedForces.push_back(
			    std::make_pair<Vec, Vec>(c->getBody().getForce(), c->getBody().getTorque()));
			c->getBody().resetForce();
			c->getBody().resetTorque();
		}

		updateCellCellConnections(*w);
		double subdt = w->getDt() / static_cast<double>(nbIterations);
		for (unsigned int i = 0; i < nbIterations; ++i) {
			// for (auto &c : connections) c.second->update(subdt);
			w->threadpool.autoChunks(connections, 100, 5,
			                         [subdt](auto &c) { c.second->update(subdt); });
			w->threadpool.waitUntilLast();
			for (size_t j = 0; j < w->cells.size(); ++j) {
				auto &c = w->cells[j];
				c->body.receiveForce(savedForces[j].first);
				c->body.receiveTorque(savedForces[j].second);
			}
			w->allForcesHaveBeenAppliedToCells();
			for (auto &c : w->cells) {
				c->body.updatePositionsAndOrientations(subdt);
				c->body.resetForce();
				c->body.resetTorque();
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////
	/**
	 * @brief create and register a contact surface connection between twoe cells
	 *
	 * @param cells the cells that are in contact
	 */
	void createConnection(const ordered_pair<Cell *> &cells) {
		auto ptr = std::make_unique<GenericConnection<Cell>>(cells);
		auto *newConnection = ptr.get();
		connections[cells] = std::move(ptr);
		cells.first->connectedCells.insert(cells.second);
		cells.second->connectedCells.insert(cells.first);
		cells.first->body.cellConnections.push_back(newConnection);
		cells.second->body.cellConnections.push_back(newConnection);
		newConnectionCallback(newConnection);
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
		assert(!connections.count(cells));
	}
	void deleteImpossibleConnections() {
		for (auto &c : connections) {
		}
	}
	/**
	 * @brief we check for any cell cell collision or adhesion
	 *
	 * @tparam W world type
	 * @param world world instance
	 */
	template <typename W> void checkForCellCellConnections(W &world) {
		// TODO: try with mutexes instead of colored grid (every batch is a task but there
		// is
		// a shared_mutex for newConnections)
		const double NEW_CONNECTION_THRESHOLD = 1.0 - 1e-10;
		if (GRIDSIZE > 0) {
			Grid<Cell *> grid(GRIDSIZE);
			for (const auto &c : world.cells) grid.insert(c);
			int minCellPerBatch = 0;
			if (world.threadpool.getNbThreads() > 0)
				minCellPerBatch = world.cells.size() / (10 * 8.0);
			auto gridCells = grid.getThreadSafeGrid(minCellPerBatch);
			for (auto &color : gridCells) {
				std::vector<std::future<std::vector<ordered_pair<Cell *>>>>
				    newConnectionsFutures;  // we collect all the futures
				newConnectionsFutures.reserve(color.size());
				for (auto &batch : color)
					if (batch.size() > 1)
						newConnectionsFutures.push_back(world.threadpool.enqueueWithFuture([&] {
							std::vector<ordered_pair<Cell *>> newConns;
							for (size_t i = 0; i < batch.size(); ++i) {
								for (size_t j = i + 1; j < batch.size(); ++j) {
									auto op = make_ordered_cell_pair(batch[i], batch[j]);
									Vec AB = op.second->getPosition() - op.first->getPosition();
									double sqDistance = AB.sqlength();
									if (sqDistance < std::pow(op.first->body.getBoundingBoxRadius() +
									                              op.second->body.getBoundingBoxRadius(),
									                          2)) {
										if (!op.first->isConnectedTo(op.second) && op.first != op.second) {
											double dist = sqrt(sqDistance);
											Vec dir = AB / dist;
											auto d0 = op.first->body.getPreciseMembraneDistance(dir);
											auto d1 = op.second->body.getPreciseMembraneDistance(-dir);
											if (dist < NEW_CONNECTION_THRESHOLD * (d0 + d1))
												newConns.push_back(op);
										}
									}
								}
							}
							return std::move(newConns);
						}));
				for (auto &ncf : newConnectionsFutures)  // and wait for their accomplishment
					for (const auto &nc : ncf.get()) createConnection(nc);
			}
		} else {
			auto &batch = world.cells;
			for (size_t i = 0; i < batch.size(); ++i) {
				for (size_t j = i + 1; j < batch.size(); ++j) {
					auto op = make_ordered_cell_pair(batch[i], batch[j]);
					Vec AB = op.second->getPosition() - op.first->getPosition();
					double sqDistance = AB.sqlength();
					if (sqDistance < std::pow(op.first->body.getBoundingBoxRadius() +
					                              op.second->body.getBoundingBoxRadius(),
					                          2)) {
						if (!op.first->isConnectedTo(op.second) && op.first != op.second) {
							double dist = sqrt(sqDistance);
							Vec dir = AB / dist;
							auto d0 = op.first->body.getPreciseMembraneDistance(dir);
							auto d1 = op.second->body.getPreciseMembraneDistance(-dir);
							if (dist < NEW_CONNECTION_THRESHOLD * (d0 + d1)) createConnection(op);
						}
					}
				}
			}
		}
	}

	/**
	 * @brief  updates (compute forces) the connections and delete the resolved ones (when
	 * cells are not in contact anymore)
	 *
	 * @param the world
	 * @param W world type
	 */
	template <typename W> void updateCellCellConnections(W &) {
		std::vector<std::pair<ordered_pair<Cell *>, GenericConnection<Cell> *>> toDisconnect;
		for (auto &con : connections) {
			if (!con.second->unbreakable) {
				if (con.second->area <= 0) {
					toDisconnect.push_back(std::make_pair(con.first, con.second.get()));
				} else if (con.first.first->getBody().getConnectedCell(con.second->direction) !=
				               con.first.second ||
				           con.first.second->getBody().getConnectedCell(-con.second->direction) !=
				               con.first.first) {
					toDisconnect.push_back(std::make_pair(con.first, con.second.get()));
				}
			}
		}
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
}  // namespace MecaCell

#endif
