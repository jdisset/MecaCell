#ifndef MECACELL_WORLD_H
#define MECACELL_WORLD_H
#include <deque>
#include <vector>
#include <algorithm>
#include <map>
#include "connection.h"
#include "grid.hpp"
#include "model.h"

using namespace std;
namespace MecaCell {
template <typename Cell, typename Integrator> class BasicWorld {

protected:
	Integrator updateCellPos;

	double dt = 1.0 / 45.0;

	// current update ID
	int frame = 0;

	// list of cells having commited apoptosis
	vector<Cell *> cellsToDestroy;

	// hashmap containing cells
	Grid<Cell *> grid = Grid<Cell *>(5.0 * DEFAULT_CELL_RADIUS);

	// model grid containting pair<model_ptr, face_id>
	Grid<pair<Model *, size_t>> modelGrid = Grid<pair<Model *, size_t>>(150);

	// enabled collisions
	bool cellCellCollisions = true;
	bool cellModelCollisions = true;

	// physics parameters
	Vec g = Vec::zero();
	double viscosityCoef = 0.001;

public:
	using cell_type = Cell;
	using integrator_type = Integrator;
	using connect_type = Connection<Cell *>;
	using model_type = Model;

	// Raw pointers! Why?
	// because it is impossible to use unique_ptr here
	// because shared_ptr would slow down the app
	// because it is not a difficult case of memory management
	vector<connect_type *> connections;

	// all the cells are in this container
	vector<Cell *> cells;

	// cellModelConnections :
	// modelName -> [ {Cell_ptr, faceId} -> pair<FTconnection,Sconnection> ]
	// A cell / model connection is composed of two sub connections : one for flexion and torsion,
	// and one for compression and elongation, which is always perpendicular to the surface
	unordered_map<string, map<pair<Cell *, size_t>, pair<Connection<ModelConnectionPoint, Cell *>,
	                                                     Connection<ModelConnectionPoint, Cell *>>>>
	    cellModelConnections;

	// all models are stored in this map, using their name as the key
	unordered_map<string, Model> models;

	/**********************************************
	 *                 GET & SET                  *
	 *********************************************/
	Vec getG() const { return g; }
	void setG(const Vec &v) { g = v; }
	const Grid<Cell *> &getCellGrid() { return grid; }
	const Grid<pair<Model *, size_t>> &getModelGrid() { return modelGrid; }
	double getViscosityCoef() const { return viscosityCoef; }
	void setViscosityCoef(const double d) { viscosityCoef = d; }

	/**********************************************
	 *             MAIN UPDATE ROUTINE            *
	 *********************************************/
	void update() {

		if (cells.size() > 0) {
			resetForces();
			computeForces();
			updatePositionsAndOrientations();
			if (cellModelCollisions) {
				performCellModelCollisions();
			}
			if (cellCellCollisions) {
				grid.clear();
				for (const auto &c : cells)
					grid.insert(c);
				updateConnectionsLengthAndDirection();
				cellCollisions();
				deleteImpossibleConnections();
			}
			updateBehavior();
			destroyCells();
		}
		++frame;
	}

	/**********************************************
	 *             UPDATE SUBROUTINES             *
	 *********************************************/

	/******************************
	 *           FORCES           *
	 ******************************/

	void computeForces() {
		// connections
		for (auto &con : connections)
			con->computeForces(dt);
		for (auto &n : cellModelConnections) {
			for (auto &p : n.second) {
				//p.second.first.computeForces(dt);
				p.second.second.computeForces(dt);
			}
		}

		for (auto &c : cells) {
			// friction
			c->receiveForce(-6.0 * M_PI * viscosityCoef * c->getRadius() * c->getVelocity());
			// gravity
			c->receiveForce(g);
		}
	}

	void resetForces() {
		for (auto cIt = cells.begin(); cIt < cells.end(); ++cIt) {
			(*cIt)->resetForce();
			(*cIt)->resetTorque();
		}
	}

	void applyGravity() {
		for (auto cIt = cells.begin(); cIt < cells.end(); ++cIt)
			(*cIt)->receiveForce(g * (*cIt)->getMass());
	}

	void updateConnectionsLengthAndDirection() {
		for (auto &c : connections) {
			c->updateLengthDirection();
		}
	}

	void updatePositionsAndOrientations() {
		for (auto cIt = cells.begin(); cIt < cells.end(); ++cIt) {
			Cell *c = *cIt;
			updateCellPos(*c, dt);
			c->markAsNotTested();
		}
	}

	/******************************
	 *           MODELS           *
	 ******************************/
	void addModel(const string &name, const string &path) {
		models.emplace(name, path);
		models.at(name).name = name;
	}

	void insertInGrid(Model &m) {
		for (size_t i = 0; i < m.faces.size(); ++i) {
			auto &f = m.faces[i];
			modelGrid.insert({&m, i}, m.vertices[f.indices[0]], m.vertices[f.indices[1]], m.vertices[f.indices[2]]);
		}
	}
	/******************************
	 *         COLLISIONS         *
	 ******************************/
	void performCellModelCollisions() {
		bool modelChange = false;
		for (auto &m : models) {
			if (m.second.changedSinceLastCheck()) {
				modelChange = true;
			}
		}
		if (modelChange) {
			modelGrid.clear();
			for (auto &m : models) {
				insertInGrid(m.second);
			}
		}
		for (auto &c : cells) {
			vector<pair<Model *, size_t>> toTest = modelGrid.retrieve(c->getPosition(), c->getRadius());
			for (const auto &mf : toTest) {
				// checking if cell c is in contact with model face mf
				const Vec &p0 = mf.first->vertices[mf.first->faces[mf.second].indices[0]];
				const Vec &p1 = mf.first->vertices[mf.first->faces[mf.second].indices[1]];
				const Vec &p2 = mf.first->vertices[mf.first->faces[mf.second].indices[2]];
				pair<bool, Vec> projec = projectionIntriangle(p0, p1, p2, c->getPosition());
				if (projec.first && (c->getPosition() - projec.second).sqlength() < pow(c->getRadius(), 2)) {
					// collision
					if (!cellModelConnections.count(mf.first->name) ||
					    !cellModelConnections.at(mf.first->name).count({c, mf.second})) {
						// new collision
						cerr << "new collision" << endl;
						double adh = c->getAdhesionWithModel(mf.first->name);
						double l = Cell::getConnectionLength(c->getRadius(), adh);
						double maxTeta = mix(0.0, M_PI / 2.0, adh);
						cellModelConnections[mf.first->name].emplace(
						    pair<pair<Cell *, size_t>, pair<Connection<ModelConnectionPoint, Cell *>,
						                                    Connection<ModelConnectionPoint, Cell *>>>(
						        // emplace {key, value}
						        {c, mf.second}, // key = {cell, face id}
						        {
						            // value = {connection, connection} (one w flexion/torsion, the other w compression)
						            Connection<ModelConnectionPoint, Cell *>(
						                {ModelConnectionPoint(mf.first, projec.second, mf.second), c}, // N0, N1
						                {Joint(c->getAngularStiffness(),
						                       dampingFromRatio(c->getDampRatio(), c->getMomentOfInertia(),
						                                        c->getAngularStiffness()),
						                       maxTeta),
						                 Joint(c->getAngularStiffness(),
						                       dampingFromRatio(c->getDampRatio(), c->getMomentOfInertia(),
						                                        c->getAngularStiffness()),
						                       maxTeta)} // Flex Joint
						                ,
						                {Joint(c->getAngularStiffness(),
						                       dampingFromRatio(c->getDampRatio(), c->getMomentOfInertia(),
						                                        c->getAngularStiffness()),
						                       maxTeta),
						                 Joint(c->getAngularStiffness(),
						                       dampingFromRatio(c->getDampRatio(), c->getMomentOfInertia(),
						                                        c->getAngularStiffness()),
						                       maxTeta)} // Torsion Joint
						                ),
						            Connection<ModelConnectionPoint, Cell *>(
						                {ModelConnectionPoint(mf.first, projec.second, mf.second), c}, // N0, N1
						                Spring(c->getStiffness(),
						                       dampingFromRatio(c->getDampRatio(), c->getMass(), c->getStiffness()), l))
						            // end value pair & end emplace
						        }));

						cellModelConnections[mf.first->name];
					}
				}
			}
		}
	}

	void cellCollisions() {
		for (auto &c : cells) {
			vector<Cell *> toTest = grid.retrieve(c);
			connect_type *s = nullptr;
			for (const auto &c2 : toTest) {
				if (!c2->alreadyTested()) {
					c->connection(c2, connections);
				}
			}
			c->markAsTested();
		}
	}

	void deleteImpossibleConnections() {
		// erase and delete connections longer than their max length
		connections.erase(remove_if(connections.begin(), connections.end(), [&](connect_type *c) {
			                  double maxL = c->getNode0()->getRadius() + c->getNode1()->getRadius();
			                  if (c->getLength() > maxL) {
				                  c->getNode0()->removeConnection(c->getNode1(), c);
				                  delete c;
				                  return true;
			                  }
			                  return false;
			                }), connections.end());
		for (auto &c : cells) {
			deleteOverlapingConnections(c);
		}
	}

	// deleteOverlapingConnections
	// deletes all connections going through cells
	void deleteOverlapingConnections(Cell *cell) {
		vector<connect_type *> &vec = cell->getRWConnections();
		for (auto c0It = vec.begin(); c0It < vec.end();) {
			bool deleted = false; // tells if c0 was deleted inside the inner loop (so we know
			                      // if we have to increment c0It)
			connect_type *c0 = *c0It;
			if (c0->getNode1() != nullptr) { // if this is not a wall connection
				Vec c0dir;
				Cell *other0 = nullptr;
				double r0;
				if (c0->getNode0() == cell) {
					c0dir = c0->getDirection();
					r0 = c0->getNode1()->getRadius();
					other0 = c0->getNode1();
				} else {
					c0dir = -c0->getDirection();
					r0 = c0->getNode0()->getRadius();
					other0 = c0->getNode0();
				}
				Vec c0v = c0dir * c0->getLength();
				double c0SqLength = pow(c0->getLength(), 2);
				for (auto c1It = c0It + 1; c1It < vec.end();) {
					connect_type *c1 = *c1It;
					if (c1->getNode1() != nullptr) {
						Vec c1dir;
						Cell *other1 = nullptr;
						double r1;
						if (c1->getNode0() == cell) {
							c1dir = c1->getDirection();
							r1 = c1->getNode1()->getRadius();
							other1 = c1->getNode1();
						} else {
							c1dir = -c1->getDirection();
							r1 = c1->getNode0()->getRadius();
							other1 = c1->getNode0();
						}
						Vec c1v = c1dir * c1->getLength();
						double c1SqLength = pow(c1->getLength(), 2);
						double scal01 = c0v.dot(c1dir);
						double scal10 = c1v.dot(c0dir);
						if (scal01 > 0 && c0SqLength < c1SqLength && (c0SqLength - scal01 * scal01) < r0 * r0 * 0.6) {
							c1It = vec.erase(c1It);
							other1->eraseConnection(c1);
							cell->eraseCell(other1);
							other1->eraseCell(cell);
							connections.erase(remove(connections.begin(), connections.end(), c1), connections.end());
							delete c1;
						} else if (scal10 > 0 && c1SqLength < c0SqLength &&
						           (c1SqLength - scal10 * scal10) < r1 * r1 * 0.6) {
							c0It = vec.erase(c0It);
							other0->eraseConnection(c0);
							cell->eraseCell(other0);
							other0->eraseCell(cell);
							connections.erase(remove(connections.begin(), connections.end(), c0), connections.end());
							deleted = true;
							delete c0;
							break; // we need to exit the inner loop, c0 doesn't exist anymore.
						} else {
							++c1It;
						}
					} else {
						++c1It;
					}
				}
			}
			if (!deleted) ++c0It;
		}
	}

	void updateBehavior() {
		for (size_t i = 0; i < cells.size(); ++i) {
			addCell(cells[i]->updateBehavior(dt));
		}
	}

	~BasicWorld() {
		destroyCells();
		while (!cells.empty())
			delete cells.back(), cells.pop_back();
		while (!connections.empty())
			delete connections.back(), connections.pop_back();
	}

	void disableCellCellCollisions() { cellCellCollisions = false; }

	int getNbUpdates() const { return frame; }

	void addCell(Cell *c) {
		if (c != NULL) cells.push_back(c);
	}

	void destroyCells() {
		for (auto i = cells.begin(); i != cells.end();) {
			if ((*i)->isDead()) {
				auto c = *i;
				c->eraseAndDeleteAllConnections(connections);
				i = cells.erase(i);
				delete c;
			} else {
				++i;
			}
		}
	}

	void reset() {
		for (unsigned int i = 0; i < cells.size(); i++) {
			cells[i]->resetForce();
			cells[i]->resetVelocity();
			cells[i]->resetAngularVelocity();
			cells[i]->resetTorque();
		}
	}
};
}

#endif
