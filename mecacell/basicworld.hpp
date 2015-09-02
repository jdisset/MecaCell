#ifndef MECACELL_WORLD_H
#define MECACELL_WORLD_H
#include <deque>
#include <vector>
#include <algorithm>
#include <map>
#include <cstdlib>
#include "connection.h"
#include "grid.hpp"
#include "model.h"

using namespace std;
namespace MecaCell {
template <typename Cell, typename Integrator> class BasicWorld {

protected:
	Integrator updateCellPos;

	double dt = 1.0 / 50.0;

	// current update ID
	int frame = 0;

	// list of cells having commited apoptosis
	vector<Cell *> cellsToDestroy;

	// hashmap containing cells
	Grid<Cell *> grid = Grid<Cell *>(5.0 * DEFAULT_CELL_RADIUS);

	// model grid containting pair<model_ptr, face_id>
	Grid<std::pair<Model *, unsigned int>> modelGrid = Grid<std::pair<Model *, unsigned int>>(100);

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
	using modelConnPair =
	    pair<Connection<ModelConnectionPoint, Cell *>, Connection<ModelConnectionPoint, Cell *>>;
	using modelConn_ptr = unique_ptr<modelConnPair>;

	// Raw pointers! Why?
	// because it is impossible to use unique_ptr here
	// because shared_ptr would slow down the app
	// because it is not a difficult case of memory management
	vector<connect_type *> connections;

	// all the cells are in this container
	vector<Cell *> cells;

	// cellModelConnections :
	// modelName -> Cell_ptr -> faceId -> pair<FTconnection,Sconnection> ]
	// A cell / model connection is composed of two sub connections : one for flexion and torsion,
	// and one for compression and elongation, which is always perpendicular to the surface
	unordered_map<string, unordered_map<Cell *, unordered_map<size_t, modelConn_ptr>>> cellModelConnections;

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
			computeForces();
			updatePositionsAndOrientations();
			if (cellModelCollisions) {
				// updateModelGrid();
				// updateModelConnections();
				// performCellModelCollisions();
			}
			if (cellCellCollisions) {
				grid.clear();
				for (const auto &c : cells)
					grid.insert(c);
				updateConnectionsLengthAndDirection();
				cellCollisions();
				// deleteImpossibleConnections();
			}
			updateBehavior();
			destroyCells();
			updateStats();
			resetForces();
		}
		++frame;
	}

	/**********************************************
	 *             UPDATE SUBROUTINES             *
	 *********************************************/

	/******************************
	 *           FORCES           *
	 ******************************/

	void updateStats() {
		for (auto &c : cells) {
			c->updateStats();
		}
	}

	void setDt(double d) { dt = d; }

	void computeForces() {
		// connections
		for (auto &con : connections)
			con->computeForces(dt);
		for (auto &n : cellModelConnections) {
			// name -> cell* -> faceId -> connection
			for (auto &c : n.second) {
				for (auto &f : c.second) {
					f.second->second.computeForces(dt);
				}
			}
		}

		for (auto &c : cells) {
			// friction
			c->receiveForce(-6.0 * M_PI * viscosityCoef * c->getRadius() * c->getVelocity());
			// gravity
			c->receiveForce(g);
		}
	}

	// void updateModelConnections() {
	//// we need to check for connections needing to be removed or translated
	//// first, do we need to remove them ?
	// for (auto &m : cellModelConnections) {
	//// foreach {modelName, connexionMap} m
	// for (auto &c : m.second) {
	//// foreach {cellPtr, face -> connexion} c
	// auto *cell = c.first;
	// for (auto it = c.second.begin(); it != c.second.end();) {
	// auto &p = (*it); // p =  pair<faceId,unique_ptr<pair<connexion>>>
	//// we first need to update all simple springs so that they always are perpendicular to the surface
	// ModelConnectionPoint &springMCP = p.second->second.getNode0();
	// const Triangle &t = springMCP.model->faces[springMCP.face];
	// auto projec = projectionIntriangle(springMCP.model->vertices[t.indices[0]],
	// springMCP.model->vertices[t.indices[1]],
	// springMCP.model->vertices[t.indices[2]], cell->getPosition());
	// if (projec.first) {
	//// still above the same triangle
	// springMCP.position = projec.second;
	//}
	//// normalement, dans le cas où teta du flexJoint coté model est > à maxTeta,
	//// il faut faire glisser le point d'accroche vers la projection de la cell
	//// (le comportement par défaut est de faire pivoter l'angle de référence).
	//// TODO : plus joli, mieux intégré, moins couteux...
	//// Joint &flex = p.second->first.getFlex().first;
	//// double MTETA = 0.01;
	//// if (flex.delta.teta > MTETA) {
	//// ModelConnectionPoint &fjMCP = p.second->first.getNode0();
	//// const Triangle &ft = fjMCP.model->faces[fjMCP.face];
	//// auto projecfj =
	//// projectionIntriangle(fjMCP.model->vertices[ft.indices[0]], fjMCP.model->vertices[ft.indices[1]],
	//// fjMCP.model->vertices[ft.indices[2]], c->getPosition());
	//// double mvLength =
	//// (projecfj.second - c->getPosition()).length() * (tan(flex.delta.teta) - tan(MTETA));
	//// fjMCP.position += mvLength * (projecfj.second - fjMCP.position).normalized();
	////}

	// if (!projec.first) {
	//// our cell is not above the triangle anymore, we need to recompute where its standard spring
	//// should now lie. Easy : we just recompute with which triangle the cell is now colliding.
	// vector<pair<Model *, size_t>> toTest = modelGrid.retrieve(cell->getPosition(), cell->getRadius());
	// for (auto &mf : toTest) {
	// if (mf.first->name == m.first) { // same model
	// const Vec &p0 = mf.first->vertices[mf.first->faces[mf.second].indices[0]];
	// const Vec &p1 = mf.first->vertices[mf.first->faces[mf.second].indices[1]];
	// const Vec &p2 = mf.first->vertices[mf.first->faces[mf.second].indices[2]];
	//// checking if cell c is in contact with model face mf (triangle p0, p1, p2)
	// pair<bool, Vec> newprojec = projectionIntriangle(p0, p1, p2, c->getPosition());
	//// projec.second = projection coordinates, projec.first = projection inside triangle
	// if (newprojec.first &&
	//(cell->getPosition() - newprojec.second).sqlength() < pow(cell->getRadius(), 2)) {
	//// we found a new triangle colliding for this same model-cell pair
	//}
	//}
	//}
	//}
	//// we need to remove a connection if its length is longer than the cell radius
	// if (p.second->second.getSc().length > cell->getRadius()) {
	// p.second->second.getNode1()->removeModelConnection(p.second.get());
	// it = c.second.erase(it);
	//} else {
	//++it;
	//}
	//}
	//}
	//}
	//// cleanup : we remove entries where a cell is listed but is not actually connected to a face
	// for (auto &m : cellModelConnections) {
	// for (auto it = m.second.begin(); it != m.second.end();) {
	// if (it->second.size() == 0) {
	// it = m.second.erase(it);
	//} else {
	// it++;
	//}
	//}
	//}
	//}

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
			double contactSurface =
			    M_PI * (pow(c->getSc().length, 2) +
			            pow((c->getNode0()->getRadius() + c->getNode1()->getRadius()) / 2.0, 2));
			c->getFlex().first.setCurrentKCoef(contactSurface);
			c->getFlex().second.setCurrentKCoef(contactSurface);
			c->getTorsion().first.setCurrentKCoef(contactSurface);
			c->getTorsion().second.setCurrentKCoef(contactSurface);
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
	void updateModelGrid() {
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
	}

	// void performCellModelCollisions() {
	//// for each cell, we find if a cell - model collision is possible.
	// for (auto &c : cells) {
	// vector<pair<Model *, size_t>> toTest = modelGrid.retrieve(c->getPosition(), c->getRadius());
	// for (const auto &mf : toTest) { // for each pair <model*, faceId> mf potentially colliding with c
	// if (!cellModelConnections.count(mf.first->name) ||
	//! cellModelConnections.at(mf.first->name).count(c) ||
	//! cellModelConnections.at(mf.first->name).at(c).count(mf.second)) {
	// const Vec &p0 = mf.first->vertices[mf.first->faces[mf.second].indices[0]];
	// const Vec &p1 = mf.first->vertices[mf.first->faces[mf.second].indices[1]];
	// const Vec &p2 = mf.first->vertices[mf.first->faces[mf.second].indices[2]];
	//// checking if cell c is in contact with model face mf (triangle p0, p1, p2)
	// pair<bool, Vec> projec = projectionIntriangle(p0, p1, p2, c->getPosition());
	//// projec.second = projection coordinates, projec.first = projection inside triangle
	// if (projec.first && (c->getPosition() - projec.second).sqlength() < pow(c->getRadius(), 2)) {
	//// new collision
	// double adh = 0; // TODO: ask cell for its adhesion with surface
	// double l = Cell::getConnectionLength(c->getRadius(), adh);
	// double maxTeta = mix(0.0, M_PI / 2.0, adh);
	// modelConn_ptr p(new modelConnPair(
	//// {connection, connection} (one w flexion/torsion, the other w compression)
	// Connection<ModelConnectionPoint, Cell *>(
	//{ModelConnectionPoint(mf.first, projec.second, mf.second), c}, // N0, N1
	//{Joint(c->getAngularStiffness(),
	// dampingFromRatio(c->getDampRatio(), c->getMomentOfInertia(),
	// c->getAngularStiffness()),
	// 0.1),
	// Joint(c->getAngularStiffness(),
	// dampingFromRatio(c->getDampRatio(), c->getMomentOfInertia(),
	// c->getAngularStiffness()),
	// maxTeta)} // Flex Joint
	//,
	//{Joint(c->getAngularStiffness(),
	// dampingFromRatio(c->getDampRatio(), c->getMomentOfInertia(),
	// c->getAngularStiffness()),
	// maxTeta),
	// Joint(c->getAngularStiffness(),
	// dampingFromRatio(c->getDampRatio(), c->getMomentOfInertia(),
	// c->getAngularStiffness()),
	// maxTeta)} // Torsion Joint
	//),
	// Connection<ModelConnectionPoint, Cell *>(
	//{ModelConnectionPoint(mf.first, projec.second, mf.second), c}, // N0, N1
	// Spring(c->getStiffness(),
	// dampingFromRatio(c->getDampRatio(), c->getMass(), c->getStiffness() * 1.0), l))
	//// end value pair & end emplace
	//));
	// c->addModelConnection(p.get());
	// cellModelConnections[mf.first->name][c][mf.second] = move(p);
	//}
	//}
	//}
	//}
	//}

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
		double overlapCoef = 0.9;
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
						if (scal01 > 0 && c0SqLength < c1SqLength &&
						    (c0SqLength - scal01 * scal01) < r0 * r0 * overlapCoef) {
							c1It = vec.erase(c1It);
							other1->eraseConnection(c1);
							cell->eraseCell(other1);
							other1->eraseCell(cell);
							connections.erase(remove(connections.begin(), connections.end(), c1), connections.end());
							delete c1;
						} else if (scal10 > 0 && c1SqLength < c0SqLength &&
						           (c1SqLength - scal10 * scal10) < r1 * r1 * overlapCoef) {
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
				for (auto &m : models) {
					if (cellModelConnections.count(m.first) && cellModelConnections.at(m.first).count(c)) {
						cellModelConnections.at(m.first).erase(c);
					}
				}
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
