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
#include "integrators.hpp"
#include "modelconnection.hpp"
#include <cstdint>

using namespace std;

#undef DBG
#define DBG DEBUG(basicworld)

namespace MecaCell {
template <typename Cell, typename Integrator = Euler,
          template <class> class SpacePartition = Grid>
class BasicWorld {
 public:
	using cell_type = Cell;
	using model_type = Model;
	using integrator_type = Integrator;
	using CellCellConnectionContainer = typename Cell::CellCellConnectionContainer;
	using CellModelConnectionContainer = typename Cell::CellModelConnectionContainer;

 protected:
	float_t dt = 1.0 / 50.0;  // interval btwn updates
	int frame = 0;            // current update number

	// space partition hashmap for cells
	SpacePartition<Cell *> cellSpacePartition =
	    SpacePartition<Cell *>(4.5 * DEFAULT_CELL_RADIUS);

	// space partition hashmap for 3Dobj faces
	SpacePartition<std::pair<model_type *, size_t>> modelSpacePartition =
	    SpacePartition<std::pair<model_type *, size_t>>(100);

	// enabled collisions & connections checks
	bool cellCellCollisions = true;
	bool cellModelCollisions = true;

	// physics parameters of the world
	Vec g = Vec::zero();
	float_t viscosityCoef = 0.001;

 public:
	CellCellConnectionContainer cellCellConnections;
	CellModelConnectionContainer cellModelConnections;

	// all the cells are in this container
	vector<Cell *> cells;
	// all models are stored in this map, using their name as the key
	unordered_map<string, model_type> models;

	/**********************************************
	 *                 GET & SET                  *
	 *********************************************/
	Vec getG() const { return g; }
	void setG(const Vec &v) { g = v; }
	void setDt(float_t d) { dt = d; }
	const SpacePartition<Cell *> &getCellGrid() { return cellSpacePartition; }
	const SpacePartition<pair<model_type *, size_t>> &getModelGrid() {
		return modelSpacePartition;
	}
	float_t getViscosityCoef() const { return viscosityCoef; }
	void setViscosityCoef(const float_t d) { viscosityCoef = d; }
	void disableCellCellCollisions() { cellCellCollisions = false; }
	int getNbUpdates() const { return frame; }

	vector<pair<Cell *, Cell *>> getConnectedCellsList() {
		unordered_set<ordered_pair<Cell *>> uniquePairs;
		for (auto &c : cells) {
			for (auto &other : c->getConnectedCells()) {
				uniquePairs.insert(make_ordered_pair(c, other));
			}
		}
		vector<pair<Cell *, Cell *>> result;
		for (auto &p : uniquePairs) {
			result.push_back(make_pair(p.first, p.second));
		}
		return result;
	}
	size_t getNbOfCellCellConnections() { return getConnectedCellsList().size(); }

	/**********************************************
	 *             MAIN UPDATE ROUTINE            *
	 *********************************************/
	void update() {
		// adding world specific forces
		for (auto &c : cells) {
			c->receiveForce(-6.0 * M_PI * viscosityCoef * c->getBoundingBoxRadius() *
			                c->getVelocity());  // friction
			c->receiveForce(g * c->getMass());  // gravity
		}
		// then connections/collisions induced forces
		Cell::updateCellCellConnections(cellCellConnections, dt);
		Cell::updateCellModelConnections(cellModelConnections, dt);

		for (auto &c : cells) {
			c->setForce(roundN(c->getForce()));
		}
		// updating cells positions
		for (auto &c : cells) {
			c->template updatePositionsAndOrientations<Integrator>(dt);
			c->setPosition(roundN(c->getPosition()));
		}

		// looking for connections/collisions
		if (cellCellCollisions)
			Cell::checkForCellCellConnections(cells, cellCellConnections, cellSpacePartition);
		if (cellModelCollisions && models.size() > 0)
			Cell::checkForCellModelConnections(cells, models, cellModelConnections,
			                                   modelSpacePartition);

		// cell behavior returns a new cell or nullptr
		vector<Cell *> newCells;
		for (auto &c : cells) {
			Cell *nc = c->updateBehavior(dt);
			if (nc) {
				newCells.push_back(nc);
			}
		}
		for (auto &nc : newCells) {
			addCell(nc);
		}

		destroyDeadCells();  // cleanup

		// getting ready for next update
		for (auto &c : cells) {
			c->updateStats();
			c->resetForces();
		}
		++frame;
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
			for (auto &c : cellModelConnections.at(name)) {
				for (auto &conn : c.second) {
					c.first->removeModelConnection(conn.second.get());
				}
			}
			cellModelConnections.erase(name);
		}
		modelSpacePartition.clear();
		for (auto &m : models) {
			insertInGrid(m.second);
		}
	}
	void insertInGrid(model_type &m) {
		for (size_t i = 0; i < m.faces.size(); ++i) {
			auto &f = m.faces[i];
			modelSpacePartition.insert({&m, i}, m.vertices[f.indices[0]],
			                           m.vertices[f.indices[1]], m.vertices[f.indices[2]]);
		}
	}
	void updateModelGrid() {
		bool modelChange = false;
		for (auto &m : models) {
			if (m.second.changedSinceLastCheck()) {
				modelChange = true;
			}
		}
		if (modelChange) {
			modelSpacePartition.clear();
			for (auto &m : models) {
				insertInGrid(m.second);
			}
		}
	}

	/******************************
	 *           MODELS           *
	 ******************************/
	void addCell(Cell *c) {
		if (c) {
			cells.push_back(c);
			c->id = cells.size() - 1;
		}
	}

	void destroyDeadCells() {
		for (auto i = cells.begin(); i != cells.end();) {
			if ((*i)->isDead()) {
				auto c = *i;
				Cell::disconnectAndDeleteAllConnections(c, cellCellConnections);
				i = cells.erase(i);
				delete c;
			} else {
				++i;
			}
		}
	}
	~BasicWorld() {
		while (!cells.empty()) delete cells.back(), cells.pop_back();
	}
};
}

#endif
