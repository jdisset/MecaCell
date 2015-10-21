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
#include "modelconnection.hpp"
#include <cstdint>
#ifdef PARRALEL
#include <omp.h>
#endif

using namespace std;
namespace std {
template <> struct hash<pair<MecaCell::Model *, unsigned int>> {
	typedef pair<MecaCell::Model *, unsigned int> argument_type;
	typedef uintptr_t result_type;

	result_type operator()(const pair<MecaCell::Model *, unsigned int> &t) const {
		return ((reinterpret_cast<result_type>(t.first) + t.second) *
		        (reinterpret_cast<result_type>(t.first) + t.second + 1) / 2) +
		       t.second;
	}
};
}
namespace MecaCell {
template <typename Cell, typename Integrator> class BasicWorld {
 protected:
	Integrator updateCellPos;

	float_t dt = 1.0 / 50.0;

	// current update ID
	int frame = 0;

	// list of cells having commited apoptosis
	vector<Cell *> cellsToDestroy;

	// hashmap containing cells
	Grid<Cell *> grid = Grid<Cell *>(4.0 * DEFAULT_CELL_RADIUS);

	// model grid containting pair<model_ptr, face_id>
	Grid<std::pair<Model *, unsigned int>> modelGrid =
	    Grid<std::pair<Model *, unsigned int>>(100);

	// enabled collisions
	bool cellCellCollisions = true;
	bool cellModelCollisions = true;

	// physics parameters
	Vec g = Vec::zero();
	float_t viscosityCoef = 0.001;

	// threshold (dot product) above which we consider two connections to be merged
	const float_t MIN_CONNECTION_SIMILARITY = 0.8;

 public:
	using cell_type = Cell;
	using integrator_type = Integrator;
	using connect_type = Connection<Cell *>;
	using model_type = Model;
	using modelConnect_type = CellModelConnection<Cell>;

	// OMG raw pointers! :o
	vector<connect_type *> connections;

	// all the cells are in this container
	vector<Cell *> cells;

	// all models are stored in this map, using their name as the key
	unordered_map<string, Model> models;

	// cells to models connections are stored in a float_t map model* -> cell*
	unordered_map<Model *,
	              unordered_map<Cell *, vector<unique_ptr<CellModelConnection<Cell>>>>>
	    cellModelConnections;

	/**********************************************
	 *                 GET & SET                  *
	 *********************************************/
	Vec getG() const { return g; }
	void setG(const Vec &v) { g = v; }
	const Grid<Cell *> &getCellGrid() { return grid; }
	const Grid<pair<Model *, unsigned int>> &getModelGrid() { return modelGrid; }
	float_t getViscosityCoef() const { return viscosityCoef; }
	void setViscosityCoef(const float_t d) { viscosityCoef = d; }

	/**********************************************
	 *             MAIN UPDATE ROUTINE            *
	 *********************************************/
	void update() {
		if (cells.size() > 0) {
			computeForces();
			updatePositionsAndOrientations();
			if (cellModelCollisions) {
				updateModelGrid();
				checkForCellModellCollisions();
			}
			if (cellCellCollisions) {
				grid.clear();
				for (const auto &c : cells) grid.insert(c);
				updateConnectionsLengthAndDirection();
				cellCollisions();
				deleteImpossibleConnections();
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

	void setDt(float_t d) { dt = d; }

	void computeForces() {
// connections
#ifdef PARALLEL
#pragma omp parallel for
#endif
		for (size_t i = 0; i < connections.size(); ++i) {
			connections[i]->computeForces(dt);
		}
		for (auto &m : cellModelConnections) {
			// model* -> cell* -> vec<connection>
			for (auto &c : m.second) {
				for (auto &cmc : c.second) {
					cmc->computeForces(dt);
				}
			}
		}

		//#pragma omp parallel for
		for (size_t i = 0; i < cells.size(); ++i) {
			auto &c = cells[i];
			// friction
			c->receiveForce(-6.0 * M_PI * viscosityCoef * c->getRadius() * c->getVelocity());
			// gravity
			c->receiveForce(g);
		}
	}

	void resetForces() {
		//#pragma omp parallel for
		for (auto cIt = cells.begin(); cIt < cells.end(); ++cIt) {
			(*cIt)->resetForce();
			(*cIt)->resetTorque();
		}
	}

	void applyGravity() {
		//#pragma omp parallel for
		for (auto cIt = cells.begin(); cIt < cells.end(); ++cIt)
			(*cIt)->receiveForce(g * (*cIt)->getMass());
	}

	void updateConnectionsLengthAndDirection() {
		//#pragma omp parallel for
		for (size_t i = 0; i < connections.size(); ++i) {
			auto &c = connections[i];
			float_t contactSurface =
			    M_PI *
			    (pow(c->getSc().length, 2) +
			     pow((c->getNode0()->getRadius() + c->getNode1()->getRadius()) / 2.0, 2));
			c->getFlex().first.setCurrentKCoef(contactSurface);
			c->getFlex().second.setCurrentKCoef(contactSurface);
			c->getTorsion().first.setCurrentKCoef(contactSurface);
			c->getTorsion().second.setCurrentKCoef(contactSurface);
			c->updateLengthDirection();
		}
	}

	void updatePositionsAndOrientations() {
		//#pragma omp parallel for
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
	void removeModel(const string &name) {
		if (models.count(name)) {
			models.erase(name);
		}
		if (cellModelConnections.count(name)) {
			cerr << "deleting stuff" << endl;
			for (auto &c : cellModelConnections.at(name)) {
				for (auto &conn : c.second) {
					c.first->removeModelConnection(conn.second.get());
				}
			}
			cerr << "ok" << endl;
			cellModelConnections.erase(name);
		}
		modelGrid.clear();
		for (auto &m : models) {
			insertInGrid(m.second);
		}
	}

	void insertInGrid(Model &m) {
		//#pragma omp parallel for
		for (size_t i = 0; i < m.faces.size(); ++i) {
			auto &f = m.faces[i];
			modelGrid.insert({&m, i}, m.vertices[f.indices[0]], m.vertices[f.indices[1]],
			                 m.vertices[f.indices[2]]);
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

	void checkForCellModellCollisions() {
		// first, we set all connections to dirty
		for (auto &m : cellModelConnections) {
			for (auto &c : m.second) {
				for (auto &conn : c.second) {
					conn->dirty = true;
				}
			}
		}
		for (auto &c : cells) {
			// for each cell, we find if a cell - model collision is possible.
			auto toTest = modelGrid.retrieve(c->getPosition(), c->getRadius());
			for (const auto &mf : toTest) {
				cerr << GREY << "+----------------------------------------------------+" << NORMAL
				     << endl;
				cerr << " potential collision between cell " << c << " and model "
				     << mf.first->name << endl;
				// for each pair <model*, faceId> mf potentially colliding with c
				const Vec &p0 = mf.first->vertices[mf.first->faces[mf.second].indices[0]];
				const Vec &p1 = mf.first->vertices[mf.first->faces[mf.second].indices[1]];
				const Vec &p2 = mf.first->vertices[mf.first->faces[mf.second].indices[2]];
				// checking if cell c is in contact with triangle p0, p1, p2
				pair<bool, Vec> projec = projectionIntriangle(p0, p1, p2, c->getPosition());
				// projec = {projection inside triangle, projection coordinates}
				// TODO: we also need to check if the connection should be on a vertice

				Vec currentDirection = projec.second - c->getPosition();
				cerr << " projec = {" << projec.first << ", " << projec.second << "}" << endl;
				if (projec.first && currentDirection.sqlength() < pow(c->getRadius(), 2)) {
					// we have a potential connection. Now we consider 2 cases:
					// 1 - brand new connection (easy)
					// 2 - older connection	(we need to update it)
					//  => same cell/model pair + similar bounce angle (same face or similar normal)
					currentDirection.normalize();
					bool alreadyExist = false;
					cerr << CYAN << " collision !" << NORMAL << endl;
					if (cellModelConnections.count(mf.first) &&
					    cellModelConnections[mf.first].count(c)) {
						cerr << " there already is a connection between this cell and this model. "
						     << endl;
						for (auto &otherconn : cellModelConnections[mf.first][c]) {
							Vec prevDirection =
							    (otherconn->bounce.getNode0().getPosition() - c->getPrevposition())
							        .normalized();
							cerr << " prevDir.dot(currentDir) = " << prevDirection.dot(currentDirection)
							     << endl;
							if (prevDirection.dot(currentDirection) > MIN_CONNECTION_SIMILARITY) {
								alreadyExist = true;
								cerr << GREEN << " Yep, it's an old connection (" << otherconn.get()
								     << ")" << endl;
								otherconn->dirty = false;
								// case n° 2, we want to update otherconn
								// first, the bounce spring
								otherconn->bounce.getNode0().position = projec.second;
								otherconn->bounce.getNode0().face = mf.second;
								cerr << " bounce spring updated" << endl;
								// then the anchor. It's just another simple spring that is always at the
								// same height as the cell (orthogonal to the bounce spring)
								// it has a restlength of 0 and follows the cell when its length is more
								// than the cell's radius;
								if (otherconn->anchor.getSc().length > 0) {
									cerr << " putting anchor at cell level" << endl;
									// first we keep the anchor at cell height
									const Vec &anchorDirection = otherconn->anchor.getSc().direction;
									Vec crossp =
									    currentDirection.cross(currentDirection.cross(anchorDirection));
									if (crossp.sqlength() > c->getRadius() * 0.02) {
										crossp.normalize();
										cerr << " projection axis = " << crossp << endl;
										float_t projLength = min(
										    (otherconn->anchor.getNode0().getPosition() - c->getPosition())
										        .dot(crossp),
										    c->getRadius());
										otherconn->anchor.getNode0().position =
										    c->getPosition() + projLength * crossp;
									}
								}
								break;
							}
						}
					}
					if (!alreadyExist) {
						// new connection
						cerr << BLUE << " it's a new connection" << endl;
						float_t adh = c->getAdhesionWithModel(mf.first->name);
						float_t l = mix(MAX_CELL_ADH_LENGTH * c->getRadius(),
						                MIN_CELL_ADH_LENGTH * c->getRadius(), adh);
						unique_ptr<CellModelConnection<Cell>> cmc(new CellModelConnection<Cell>(
						    Connection<SpaceConnectionPoint, Cell *>(
						        {SpaceConnectionPoint(c->getPosition()), c},  // N0, N1
						        Spring(100, dampingFromRatio(0.9, c->getMass(), 100),
						               0)),  // anchor
						    Connection<ModelConnectionPoint, Cell *>(
						        {ModelConnectionPoint(mf.first, projec.second, mf.second),
						         c},  // N0, N1
						        Spring(c->getStiffness(),
						               dampingFromRatio(c->getDampRatio(), c->getMass(),
						                                c->getStiffness() * 1.0),
						               l)  // bounce
						        )));
						cmc->anchor.tjEnabled = false;
						// cmc->anchor.getFlex().first.targetUpdateEnabled = false;
						// cmc->anchor.getFlex().first.target = -currentDirection;
						c->addModelConnection(cmc.get());
						cellModelConnections[mf.first][c].push_back(move(cmc));
					}
				}
				cerr << PURPLE << "|___________________________________________|" << NORMAL
				     << endl
				     << endl;
			}
		}
		// clean up: dirty connections
		for (auto &m : cellModelConnections) {
			for (auto &c : m.second) {
				for (auto it = c.second.begin(); it != c.second.end();) {
					if ((*it)->dirty) {
						cerr << " deleting dirty connection " << endl;
						c.first->removeModelConnection(it->get());
						it = c.second.erase(it);
					} else {
						++it;
					}
				}
			}
		}
		// clean up: empty entries
		for (auto itM = cellModelConnections.begin(); itM != cellModelConnections.end();) {
			if (itM->second.empty()) {
				itM = cellModelConnections.erase(itM);
			} else {
				for (auto itC = itM->second.begin(); itC != itM->second.end();) {
					if (itC->second.empty()) {
						itC = itM->second.erase(itC);
					} else {
						++itC;
					}
				}
				++itM;
			}
		}
	}

	// void cellCollisions() {
	// for (auto &c : cells) {
	// vector<Cell *> toTest = grid.retrieve(c);
	// connect_type *s = nullptr;
	// for (const auto &c2 : toTest) {
	// if (!c2->alreadyTested()) {
	// auto con = c->connection(c2);
	// if (con) connections.push_back(con);
	//}
	//}
	// c->markAsTested();
	//}
	//}
	void cellCollisions() {
		auto gridCells = grid.getThreadSafeGrid();
		for (auto &batch : gridCells) {
			// each batch should be thread safe
			array<vector<connect_type *>, 64> newConnections;
// set<Cell *> test;
#pragma omp parallel for schedule(dynamic)
			for (int i = 0; i < batch.size(); ++i) {
				for (int j = 0; j < batch[i].size(); ++j) {
					auto &c0 = batch[i][j];
					// assert(test.count(c0) == 0);
					// test.insert(c0);
					for (int k = j + 1; k < batch[i].size(); ++k) {
						auto &c1 = batch[i][k];
						auto con = c0->connection(c1);
						if (con) {
#ifdef PARALLEL
							newConnections[omp_get_thread_num()].push_back(con);
#else
							connections.push_back(con);
#endif
						}
					}
				}
			}
#ifdef PARALLEL
			for (auto &cv : newConnections) {
				for (auto &nc : cv) {
					connections.push_back(nc);
				}
			}
#endif
		}
	}

	void deleteImpossibleConnections() {
		// erase and delete connections longer than their max length
		connections.erase(
		    remove_if(connections.begin(), connections.end(), [&](connect_type *c) {
			    float_t maxL = c->getNode0()->getRadius() + c->getNode1()->getRadius();
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
		float_t overlapCoef = 0.9;
		vector<connect_type *> &vec = cell->getRWConnections();
		for (auto c0It = vec.begin(); c0It < vec.end();) {
			bool deleted = false;  // tells if c0 was deleted inside the inner loop (so we know
			                       // if we have to increment c0It)
			connect_type *c0 = *c0It;
			if (c0->getNode1() != nullptr) {  // if this is not a wall connection
				Vec c0dir;
				Cell *other0 = nullptr;
				float_t r0;
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
				float_t c0SqLength = pow(c0->getLength(), 2);
				for (auto c1It = c0It + 1; c1It < vec.end();) {
					connect_type *c1 = *c1It;
					if (c1->getNode1() != nullptr) {
						Vec c1dir;
						Cell *other1 = nullptr;
						float_t r1;
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
						float_t c1SqLength = pow(c1->getLength(), 2);
						float_t scal01 = c0v.dot(c1dir);
						float_t scal10 = c1v.dot(c0dir);
						if (scal01 > 0 && c0SqLength < c1SqLength &&
						    (c0SqLength - scal01 * scal01) < r0 * r0 * overlapCoef) {
							c1It = vec.erase(c1It);
							other1->eraseConnection(c1);
							cell->eraseCell(other1);
							other1->eraseCell(cell);
							connections.erase(remove(connections.begin(), connections.end(), c1),
							                  connections.end());
							delete c1;
						} else if (scal10 > 0 && c1SqLength < c0SqLength &&
						           (c1SqLength - scal10 * scal10) < r1 * r1 * overlapCoef) {
							c0It = vec.erase(c0It);
							other0->eraseConnection(c0);
							cell->eraseCell(other0);
							other0->eraseCell(cell);
							connections.erase(remove(connections.begin(), connections.end(), c0),
							                  connections.end());
							deleted = true;
							delete c0;
							break;  // we need to exit the inner loop, c0 doesn't exist
							        // anymore.
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
		//#pragma omp parallel for
		for (size_t i = 0; i < cells.size(); ++i) {
			addCell(cells[i]->updateBehavior(dt));
		}
	}

	~BasicWorld() {
		destroyCells();
		while (!cells.empty()) delete cells.back(), cells.pop_back();
		while (!connections.empty()) delete connections.back(), connections.pop_back();
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
					if (cellModelConnections.count(&m.second) &&
					    cellModelConnections.at(&m.second).count(c)) {
						cellModelConnections.at(&m.second).erase(c);
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
