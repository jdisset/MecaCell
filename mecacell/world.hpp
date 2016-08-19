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

/**
 * @brief Where "everything" happens.
 *
 * Handles the addition and deletion of cells and the main update routine. Hooks can be
 * added either through the registerPlugin method or through the addHook and addHooks
 * methods
 *
 * @tparam Cell the user's cell type
 * @tparam Integrator the type of integrator to use. Defaults to semi-explicit Euler
 */
template <typename Cell, typename Integrator = Euler> class World {
 public:
	using cell_t = Cell;
	using integrator_t = Integrator;
	using hook_t =
	    std::function<void(World *)>;  /// hook signature, as they should appear in
	                                   /// plugins classes. cf. hooks & plugins section

	/// A cell type can define one embedded plugin class type which will be
	/// instanciated in the world.
	/// This can be useful, for example, to declare a collision system that stores
	/// connections at world level. As this kind of things are common & tightly related to
	/// the cell's implementation (ie. its membrane type),
	/// it seems fitting to let it be handled by a plugin that would be directly defined by
	/// the cell type itself.
	// -------------------------------------
	// The following code detects if any embedded plugin type is specified and defaults
	// to the instantiation of a useles byte if not.
	template <class, class = void_t<>> struct embedded_plugin_type {  // declaration
		using type = char;  // defaults to dummy char type if no embedded plugin is defined
	};
	template <class T>  // specialization
	struct embedded_plugin_type<T, void_t<typename T::embedded_plugin_t>> {
		using type = typename T::embedded_plugin_t;  // embedded plugin detected
	};
	using cellPlugin_t = typename embedded_plugin_type<cell_t>::type;  // either char or the
	                                                                   // plugin type
	                                                                   // defined by Cell

 protected:
	double dt = 1.0 / 100.0;  /// The amount by which time is increased every update
	size_t frame = 0;         /// +1 at each update. cf getNbUpdates()
	size_t nbAddedCells = 0;  // +1 on cell add. Used for cell's unique ids

	std::array<std::vector<hook_t>, eToUI(Hooks::LAST)> hooks;  // where hooks are stored

	void deleteDeadCells() {
		for (auto i = cells.begin(); i != cells.end();)
			if ((*i)->isDead())
				i = cells.erase(i), delete *i;
			else
				++i;
	}

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

 public:
	cellPlugin_t cellPlugin;  // instance of the embedded cell plugin type
	                          // (cellPlugin_t default to a dumb char if not specified)

	std::vector<Cell *> cells;  /// all the cells are in this container

	/**
	 * @brief World constructor, registers the (optional) cell type's embedded plugin
	 * instance.
	 */
	World() { registerPlugins(cellPlugin); }

	/**********************************************
	 *               HOOKS & PLUGINS              *
	 *********************************************/

	/**
	 * @brief register a single hook method. cf registerPlugins()
	 *
	 * @param h hook type (same name as hook method)
	 * @param f the actual hook (a std::function<void(World*)>)
	 */
	void registerHook(const Hooks &h, hook_t f) { hooks[eToUI(h)].push_back(f); }

	/**
	 * @brief Registers a plugin. A plugin is a class or struct that contains hooks
	 *
	 * This method takes an arbitrary number of plugin classes and registers them in the
	 * order of appearance, which is also the order in which similar hooks will be called.
	 *
	 * Signatures of hooks methods must be void (World*)
	 * Available hooks are:
	 *  - beginUpdate: called at each world update before everything else
	 *  - preBehaviorUpdate: called the world calls every individual updateBehavior(World&)
	 * cells
	 * methods
	 *  - postBehaviorUpdate: called after updateBehavior(World&) and just before dead cells
	 * are
	 * removed (this is where death-related cleanup should be made, before the cell is
	 * automatically effectively removed from the world)
	 *  - endUpdate: called at the end of each world update routine
	 *  - addCell: called after each cell addition to the world (the new cell is thus
	 * guaranteed to be the last one in the cells container)
	 *  - destructor: called when world's destructor is called, just before all of the cells
	 * are deleted.
	 *
	 */
	template <typename P, typename... Rest>
	void registerPlugins(P &&p, Rest &&... otherPlugins) {
		registerPlugin(std::forward<P>(p));
		registerPlugins(std::forward<Rest>(otherPlugins)...);
	}
	void registerPlugins() {}  /// end of recursion
	template <typename P> void registerPlugin(P &p) {
		loadPluginHooks(this, p);
	}  /// for a single plugin

	/**
	 * @brief clear all registered hooks. CAUTION: it will also delete any default embedded
	 * plugins the cell type could provide (specific collision manager for example)
	 */
	void clearHooks() {
		for (auto &h : hooks) h.clear();
	}

	/**********************************************
	 *                 GET & SET                  *
	 *********************************************/

	/**
	 * @brief get the number of update since the creation of the world
	 *
	 * @return
	 */
	size_t getNbUpdates() const { return frame; }

	/**
	 * @brief adds a cell to the cells container.
	 *
	 * Construction should be done by the user but deletion is automatically handled by
	 * the world (ex. ' addCell(new MyCell()); ' )
	 * Added cell is guaranteed to be at the end of the contianer.
	 * Triggers addCell hooks
	 *
	 * @param c a pointer to the cell
	 */
	void addCell(Cell *c) {
		if (c) {
			cells.push_back(c);
			c->id = nbAddedCells++;
		}
		for (auto &f : hooks[eToUI(Hooks::addCell)]) f(this);
	}

	/**
	 * @brief sets the amount by which time is increased at each update() call.
	 *
	 * smaller dt values generally results in more stable physics (at the expense of slower
	 * simulations)
	 *
	 * @param d the new dt value
	 */
	void setDt(double d) { dt = d; }

	/**
	 * @brief main update method
	 *
	 * this should be manually called at each time step by the user. It calls the core
	 * logics of the adhesions & collisions systemi, updateis every forces, speeds and
	 * positions and calls the individual updateBehavior of each cells.
	 * 4 type of hooks are triggered:
	 * - beginUpdate
	 * - preBehaviorUpdate
	 * - postBehaviorUpdate
	 * - endUpdate
	 */
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

	/**
	 * @brief destructor. Triggers the destructor hooks and delete all cells.
	 */
	~World() {
		for (auto &f : hooks[eToUI(Hooks::destructor)]) f(this);
		while (!cells.empty()) delete cells.back(), cells.pop_back();
	}
};
}
#endif