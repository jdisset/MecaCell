#ifndef MECACELL_WORLD_H
#define MECACELL_WORLD_H
#include <array>
#include <functional>
#include <vector>
#include "cellcellconnectionshandler.hpp"
#include "integrators.hpp"
#include "utilities/hooks_utilities.hpp"
#include "utilities/utils.h"

namespace MecaCell {
template <typename Cell, typename Integrator = Euler> class World {
	/*
	 * Handles the addition and deletion of cells, as well as their collisions and adhesions
	 * through the CellCellConnectionsHandler. Hooks can be added either through the
	 * registerPlugin method or through the addHook and addHooks methods
	 */
 public:
	using cell_t = Cell;
	using integrator_t = Integrator;
	using hook_t = std::function<void(World *)>;

 protected:
	double dt = 1.0 / 100.0;  // The amount by which time is increased every update
	size_t frame = 0;         // +1 at each update
	size_t nbAddedCells = 0;  // +1 on cell add. Used for cell's unique ids
	CellCellConnectionsHandler ccch{};

	/**********************************************
	 *                   HOOKS                    *
	 *********************************************/
	std::array<std::vector<hook_t>, eToUI(Hooks::LAST)> hooks;  // where lambdas are stored

	void registerHook(const Hooks &h, hook_t f) { hooks[eToUI(h)].push_back(f); }

	// a plugin is just an object with hooks methods in it
	template <typename P> void registerPlugin(P &p) { loadPluginHooks(this, p); }
	template <typename P> void registerPlugin void registerPlugins() {}  // end of recursion
	template <typename P, typename... Rest>
	void registerPlugins(P &&p, Rest &&... otherPlugins) {
		registerPlugin(std::forward<P>(p));
		registerPlugins(std::forward<Rest>(otherPlugns));
	}
	void clearHooks() {
		for (auto &h : hooks) h.clear();
	}

 public:
	std::vector<Cell *> cells;  // all the cells are in this container
	World() { registerPlugin(ccch); }

	/**********************************************
	 *                 GET & SET                  *
	 *********************************************/
	size_t getNbUpdates() const { return frame; }
	auto &getCellCellConnectionsHandler() { return ccch; }

	void prepareCellForNextUpdate() {
		for (auto &c : cells) {
			c->updateStats();
			c->resetForces();
			c->applyExternalForces();
			c->applyExternalTorque();
			c->resetExternalForces();
			c->resetExternalTorque();
		}
	}

	void addCell(Cell *c) {
		if (c) {
			cells.push_back(c);
			c->id = nbAddedCells++;
		}
		for (auto &f : hooks[eToUI(Hooks::addCell)]) f(this);
	}

	void deleteDeadCells() {
		for (auto i = cells.begin(); i != cells.end();)
			if ((*i)->isDead())
				i = cells.erase(i), delete *i;
			else
				++i;
	}

	void update() {
		for (auto &f : hooks[eToUI(Hooks::beginUpdate)]) f(this);
		prepareCellForNextUpdate();
		for (auto &c : cells) c->template updatePositionsAndOrientations<Integrator>(dt);
		for (auto &f : hooks[eToUI(Hooks::preBehaviorUpdate)]) f(this);
		for (auto &c : cells) c->updateBehavior(*this);
		for (auto &f : hooks[eToUI(Hooks::postBehaviorUpdate)]) f(this);
		deleteDeadCells();
		for (auto &f : hooks[eToUI(Hooks::endUpdate)]) f(this);
		++frame;
	}

	~World() {
		for (auto &f : hooks[eToUI(Hooks::destructor)]) f(this);
		while (!cells.empty()) delete cells.back(), cells.pop_back();
	}
};
}
#endif
