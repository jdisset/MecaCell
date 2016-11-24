#ifndef CONTACTSURFACEBODY_HPP
#define CONTACTSURFACEBODY_HPP
#include <future>
#include <memory>
#include "contactsurface.hpp"
#include "integrators.hpp"
#include "orientable.h"
#include "utilities/grid.hpp"
#include "utilities/ordered_hash_map.hpp"
#include "utilities/ordered_pair.hpp"

namespace MecaCell {

/**
 * @brief this struct models and handles everything related to
 * surface computations between two colliding cells
 */

/**
 * @brief Embedded plugin that will handle the collision detection routines. Parallel
 * code.
 */
template <typename Cell> struct ContactSurfaceBodyPlugin {
	const size_t MIN_CHUNK_SIZE = 5;
	const size_t AVG_TASKS_PER_THREAD = 3;
	ordered_hash_map<ordered_pair<Cell *>,
	                 std::unique_ptr<ContactSurface<Cell>>>
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
		                         [dt = w->getDt()](auto &c) {
			                         c->body.computeCurrentAreaAndVolume();
			                         c->body.updateDynamicRadius(dt);
			                       });
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
		connections[cells] = std::make_unique<ContactSurface<Cell>>(cells);
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
	void disconnect(const ordered_pair<Cell *> &cells, ContactSurface<Cell> *conn) {
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
		// logger<INF>("ccc 0");
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
							if (sqDistance <
							    std::pow(op.first->body.dynamicRadius + op.second->body.dynamicRadius,
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
		// logger<INF>("ccc 2");
	}

	/**
	 * @brief  updates (compute forces) the connections and delete the resolved ones (when
	 * cells are not in contact anymore)
	 *
	 * @param w the world
	 * @param W world type
	 */
	template <typename W> void updateCellCellConnections(W &w) {
		// logger<INF>("updt ccc 0");
		std::vector<std::pair<ordered_pair<Cell *>, ContactSurface<Cell> *>> toDisconnect;
		// w.threadpool.autoChunks(
		// connections, MIN_CHUNK_SIZE, AVG_TASKS_PER_THREAD,
		//[dt = w.getDt()](auto &conPair) { conPair.second->update(dt); });

		for (auto &c : connections) {
			c.second->update(w.getDt());
		}
		// w.threadpool.waitUntilLast();
		for (auto &con : connections)
			if (!con.second->fixedAdhesion && con.second->area <= 0)
				toDisconnect.push_back(std::make_pair(con.first, con.second.get()));
		for (auto &c : toDisconnect) disconnect(c.first, c.second);
		// logger<INF>("updt ccc 1");
	}

	template <typename W> void preDeleteDeadCellsUpdate(W *w) {
		for (auto &c : w->cells) {
			if (c->isDead()) {
                auto cctmp (c->body.cellConnections);
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

template <typename Cell> class ContactSurfaceBody : public Orientable {
	friend class ContactSurfaceBodyPlugin<Cell>;

	Cell *cell = nullptr;
	std::vector<ContactSurface<Cell> *> cellConnections;

	// params
	double incompressibility = 0.1;
	double membraneStiffness = 15;

	double baseRadius = 40;  /// base radius is the base rest radius. It serves as reference
	                         /// in order to grow
	double restRadius = baseRadius;  /// radiius of the cell when at rest
	// dynamic target radius:
	double dynamicRadius = restRadius;
	double prevDynamicRadius = dynamicRadius;
	static constexpr double MAX_DYN_RADIUS_RATIO = 2.0;
	double currentArea = 4.0 * M_PI * restRadius * restRadius;
	double restVolume = (4.0 * M_PI / 3.0) * restRadius * restRadius;
	double currentVolume = restVolume;
	double pressure = 0;
	bool volumeConservationEnabled = true;

 public:
	using embedded_plugin_t = ContactSurfaceBodyPlugin<Cell>;

	ContactSurfaceBody(Cell *c) : cell(c){};

	void setVolumeConservationEnabled(bool v) { volumeConservationEnabled = v; }
	void setRestVolume(double v) { restVolume = v; }
	void setRestRadius(double r) { restRadius = r; }
	double getDynamicRadius() const { return dynamicRadius; }
	double getBoundingBoxRadius() const { return dynamicRadius; };
	std::tuple<Cell *, double> getConnectedCellAndMembraneDistance(const Vec &d) const {
		// /!\ assumes that d is normalized
		Cell *closestCell = nullptr;
		double closestDist = dynamicRadius;
		for (auto &con : cellConnections) {
			auto normal = cell == con->cells.first ? -con->normal : con->normal;
			double dot = normal.dot(d);
			if (dot < 0) {
				const auto &midpoint =
				    cell == con->cells.first ? con->midpoint.first : con->midpoint.second;
				double l = -midpoint / dot;
				if (l < closestDist) {
					closestDist = l;
					closestCell = con->cells.first == cell ? con->cells.second : con->cells.first;
				}
			}
		}
		return std::make_tuple(closestCell, closestDist);
	}

	void computeCurrentAreaAndVolume() {
		restVolume = (4.0 * M_PI / 3.0) * restRadius * restRadius * restRadius;
		double volumeLoss = 0;
		double surfaceLoss = 0;
		// cell connections
		for (auto &con : cellConnections) {
			auto &midpoint =
			    cell == con->cells.first ? con->midpoint.first : con->midpoint.second;
			auto h = dynamicRadius - midpoint;
			volumeLoss += (M_PI * h / 6.0) * (3.0 * con->sqradius + h * h);
			surfaceLoss += 2.0 * M_PI * dynamicRadius * h - con->area;
		}
		// TODO : soustraire les overlapps
		double baseVol = (4.0 * M_PI / 3.0) * dynamicRadius * dynamicRadius * dynamicRadius;
		currentVolume = baseVol - volumeLoss;
		const double minVol = 0.1 * restVolume;
		const double minArea =
		    0.1 * getRestArea();  // garde fou en attendant de soustraire les overlaps
		if (currentVolume < minVol) currentVolume = minVol;
		if (currentArea < minArea) currentArea = minArea;
	}

	void updateDynamicRadius(double dt) {
		if (volumeConservationEnabled) {
			double dA = currentArea - getRestArea();
			double dV = restVolume - currentVolume;
			auto Fv = incompressibility * dV;
			auto Fa = membraneStiffness * dA;
			pressure = Fv / currentArea;
			double dynSpeed = (dynamicRadius - prevDynamicRadius) / dt;
			double c = 5.0;
			dynamicRadius += dt * dt * (Fv - Fa - dynSpeed * c);
			if (dynamicRadius > restRadius * MAX_DYN_RADIUS_RATIO)
				dynamicRadius = restRadius * MAX_DYN_RADIUS_RATIO;
			else if (dynamicRadius < restRadius)
				dynamicRadius = restRadius;
		} else {
			dynamicRadius = restRadius;
		}
	}

	/**
	 * @brief uses
	 *
	 * @tparam Integrator
	 * @param dt
	 */
	template <typename Integrator = Euler> void updatePositionsAndOrientations(double dt) {
		Integrator::updatePosition(*cell, dt);
		Integrator::updateOrientation(*this, dt);
	}

	void solidifyAdhesions() {
		for (auto &c : cellConnections) c->fixedAdhesion = true;
	}
	void releaseAdhesions() {
		for (auto &c : cellConnections) c->fixedAdhesion = false;
	}

	inline Cell *getConnectedCell(const Vec &d) const {
		return get<0>(getConnectedCellAndMembraneDistance(d));
	}
	inline double getPreciseMembraneDistance(const Vec &d) const {
		return get<1>(getConnectedCellAndMembraneDistance(d));
	}
	inline double getRestArea() const { return 4.0 * M_PI * restRadius * restRadius; }
	inline void computeRestVolume() {
		restVolume = (4.0 * M_PI / 3.0) * restRadius * restRadius * restRadius;
	}
	inline double getBaseVolume() const {
		return (4.0 * M_PI / 3.0) * baseRadius * baseRadius * baseRadius;
	}
	inline double getRestMomentOfInertia() const {
		return 0.4 * cell->getMass() * restRadius * restRadius;
	}

	inline double getMomentOfInertia() const { return getRestMomentOfInertia(); }
	inline double getVolumeVariation() const { return restVolume - currentVolume; }

	double getVolume() const { return restVolume; }
	// SET
	void setIncompressibility(double i) { incompressibility = i; }
	void setStiffness(double k) { membraneStiffness = k; }
	void setDynamicRadius (double r) { dynamicRadius = r; }
	double getPressure() const { return pressure; }
	double getRestRadius (void) const { return restRadius; }
};
}
#endif
