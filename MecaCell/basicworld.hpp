#ifndef WORLD_H
#define WORLD_H
#include <vector>
#include <algorithm>
#include "connection.h"
#include "grid.hpp"

using namespace std;
namespace MecaCell {
template <typename Cell, typename Integrator> class BasicWorld {

 protected:
	Integrator updateCellPos;
	double dt = 1.0 / 45.0;
	int frame = 0;
	vector<Cell*> cellsToDestroy;
	Grid<Cell> grid = Grid<Cell>(8.0 * DEFAULT_CELL_RADIUS);
	bool cellCellCollisions = true;
	Vec g = Vec::zero();

 public:
	typedef Cell cell_type;
	typedef Integrator integrator_type;


	// Raw pointers! Why?
	// because it is impossible to use unique_ptr here
	// because shared_ptr would be too slow
	// because we would need to use a slower type of container if not using pointers
	// because it is not a difficult case of memory management
	vector<Connection<Cell>*> connections;
	vector<Cell*> cells;
	// list<InfPlane<V>> infplanes;  // "walls"

	/**********************************************
	 *             MAIN UPDATE ROUTINE            *
	 *********************************************/
	void update() {
		if (cells.size() > 0) {
			resetForces();
			//applyGravity();
			computeForces();
			// for (auto& p : infplanes) {
			// p.updateCollisions();
			// p.collisions(cells);
			//}
			updatePositionsAndOrientations();
			if (cellCellCollisions) {
				grid.clear();
				for (const auto& c : cells) grid.insert(c);
				updateConnectionsLengthAndDirection();
				cellCollisions();
				//deleteImpossibleConnections();
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
		for (auto cIt = connections.begin(); cIt < connections.end(); ++cIt) {
			(*cIt)->computeForces(dt);
		}
		// update walls nodes
		// for (auto& p : infplanes) {
		// p.computeForces();
		//}
	}
	void resetForces() {
		for (auto cIt = cells.begin(); cIt < cells.end(); ++cIt) {
			(*cIt)->resetForce();
			(*cIt)->resetTorque();
		}
	}
	void applyGravity() {
		for (auto cIt = cells.begin(); cIt < cells.end(); ++cIt) (*cIt)->receiveForce(g * (*cIt)->getMass());
	}

	void updateConnectionsLengthAndDirection() {
		for (auto& c : connections) {
			c->updateLengthDirection();
		}
	}

	void updatePositionsAndOrientations() {
		for (auto cIt = cells.begin(); cIt < cells.end(); ++cIt) {
			Cell* c = *cIt;
			updateCellPos(*c,dt);
			c->markAsNotTested();
		}
	}

	/******************************
	 *         COLLISIONS         *
	 ******************************/
	void cellCollisions() {
		for (auto& c : cells) {
			vector<Cell*> toTest = grid.retrieve(c);
			Connection<Cell>* s = nullptr;
			for (const auto& c2 : toTest) {
				if (!c2->alreadyTested()) {
					s = c->connection(c2);
					if (s != nullptr) {
						connections.push_back(s);
					}
				}
			}
			c->markAsTested();
		}
	}

	void deleteImpossibleConnections() {
		connections.erase(remove_if(connections.begin(), connections.end(), [&](Connection<Cell>* c) {
			                  double maxL = c->getNode0()->getRadius() + c->getNode1()->getRadius();
			                  if (c->getLength() > maxL) {
				                  c->getNode0()->deleteConnection(c->getNode1(), c);
				                  delete c;
				                  return true;
			                  }
			                  return false;
			                }),
		                  connections.end());
		for (auto& c : cells) {
			deleteOverlapingConnections(c);
		}
	}

	// deleteOverlapingConnections
	// deletes all connections going through cells
	void deleteOverlapingConnections(Cell* cell) {
		vector<Connection<Cell>*>& vec = cell->getRWConnections();
		for (auto c0It = vec.begin(); c0It < vec.end();) {
			bool deleted = false;  // tells if c0 was deleted inside the inner loop (so we know
			                       // if we have to increment c0It)
			Connection<Cell>* c0 = *c0It;
			if (c0->getNode1() != nullptr) {  // if this is not a wall connection
				Vec c0dir;
				Cell* other0 = nullptr;
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
					Connection<Cell>* c1 = *c1It;
					if (c1->getNode1() != nullptr) {
						Vec c1dir;
						Cell* other1 = nullptr;
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
						if (scal01 > 0 && c0SqLength < c1SqLength && (c0SqLength - scal01 * scal01) < r0 * r0 * 0.8) {
							c1It = vec.erase(c1It);
							other1->eraseConnection(c1);
							cell->eraseCell(other1);
							other1->eraseCell(cell);
							connections.erase(remove(connections.begin(), connections.end(), c1), connections.end());
							delete c1;
						} else if (scal10 > 0 && c1SqLength < c0SqLength &&
						           (c1SqLength - scal10 * scal10) < r1 * r1 * 0.8) {
							c0It = vec.erase(c0It);
							other0->eraseConnection(c0);
							cell->eraseCell(other0);
							other0->eraseCell(cell);
							connections.erase(remove(connections.begin(), connections.end(), c0), connections.end());
							deleted = true;
							delete c0;
							break;  // we need to exit the inner loop, c0 doesn't exist anymore.
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
		while (!cells.empty()) delete cells.back(), cells.pop_back();
		while (!connections.empty()) delete connections.back(), connections.pop_back();
	}

	void disableCellCellCollisions() { cellCellCollisions = false; }

	int getNbUpdates() const { return frame; }

	void addCell(Cell* c) {
		if (c != NULL) cells.push_back(c);
	}

	void destroyCells() {
		for (auto i = cells.begin(); i != cells.end();) {
			if ((*i)->isDead()) {
				auto c = *i;
				c->eraseAndDeleteAllConnections(connections);
				i = cells.erase(i);
				// deleting wall connections as well
				// for (auto& p : infplanes) {
				// p.deleteConnectionsToCell(c);
				//}
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
