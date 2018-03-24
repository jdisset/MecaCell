#ifndef MECACELL_WORLD_H
#define MECACELL_WORLD_H
#include <array>
#include <functional>
#include <unordered_set>
#include <utility>
#include <vector>
#include "integrators.hpp"
#include "utilities/hooktools.hpp"
#include "utilities/threadpool.hpp"
#include "utilities/utils.hpp"

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
 private:
	size_t nbThreads = 0;

 public:
	using cell_t = Cell;
	using integrator_t = Integrator;
	using hook_s = void(World *);  /// hook signature, as they should appear in
	                               /// plugins classes. cf. hooks & plugins section

	ThreadPool threadpool;  // threadpool that can be used by plugins (and is also used to
	                        // parallelize updateBehavior)

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
	struct dumb {};
	template <class, class = MecaCell::void_t<>>
	struct embedded_plugin_type {  // declaration
		using type = dumb;  // defaults to empty type if no embedded plugin is defined
	};
	template <class T>  // specialization
	struct embedded_plugin_type<T, void_t<typename T::embedded_plugin_t>> {
		using type = typename T::embedded_plugin_t;  // embedded plugin detected
	};
	using cellPlugin_t =
	    typename embedded_plugin_type<cell_t>::type;  // either dumb empty struct or the
	                                                  // plugin type
	                                                  // defined by Cell (if existing)
 protected:
	size_t frame = 0;         /// +1 at each update. cf getNbUpdates()
	size_t nbAddedCells = 0;  /// +1 on cell add. Used for cell's unique ids
	size_t updtBhvPeriod =
	    1;  /// period at which the world should call the cells updateBehavior method.
	bool parallelUpdateBehavior = false;

	/**
	 * @brief removes all cells marked dead
	 */
	void deleteDeadCells() {
		for (auto i = cells.begin(); i != cells.end();) {
			if ((*i)->isDead()) {
				auto tmp = *i;
				i = cells.erase(i);
				delete tmp;
			} else {
				++i;
			}
		}
	}

 public:
	double dt = 1.0 / 100.0;  /// The amount by which time is increased every update
	cellPlugin_t cellPlugin;  // instance of the embedded cell plugin type
	                          // (cellPlugin_t default to a dumb char if not specified)

	vector<Cell *> newCells;  /// cells that are registered to be added

	/* HOOKS
	 */
	DECLARE_HOOK(onAddCell, beginUpdate, preBehaviorUpdate, preDeleteDeadCellsUpdate,
	             postBehaviorUpdate, endUpdate, allForcesAppliedToCells, destructor)

	/* CELLS */
	std::vector<Cell *> cells;  /// all the cells are in this container

	/**
	 * @brief World constructor, registers the (optional) cell type's embedded plugin
	 * instance.
	 *
	 * @param nbThreads the number of threads to be used by plugins (physics, for example)
	 * or cells (through their updateBehavior method)
	 * that can take advantage of parallelism
	 */
	World(size_t nThreads = 0) : nbThreads(nThreads), threadpool(nThreads) {
		registerPlugins(cellPlugin);
	}

	/**
	 * @brief enables or disables the parallelisation of the cells' updateBehavior methods
	 * Only has effect if nbThreads > 0;
	 *
	 * @param p
	 */
	void setParallelUpdateBehavior(bool p) { parallelUpdateBehavior = p; };

	/**
	 * @brief the size of the threadpool that can be used by plugins to launch asynchronous
	 * jobs and also directly in the world class to launch the updateBehavior methods in
	 * parallel. See setParallelUpdateBehavior.
	 *
	 * @return the number of threads in the threadpool
	 */
	size_t getNbThreads() { return nbThreads; }

	void setNbThreads(size_t n) {
		nbThreads = n;
		threadpool.setNbThreads(n);
	}

	/**********************************************
	 *               HOOKS & PLUGINS              *
	 *********************************************/

	/**
	 * @brief register a single hook method. cf registerPlugins()
	 *
	 * @param h hook type (same name as hook method)
	 * @param f the actual hook
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
	 * cells methods
	 *  - postBehaviorUpdate: called after updateBehavior(World&) and just
	 * before dead cells are removed (this is where death-related cleanup should be made,
	 * before the cell is automatically effectively removed from the world)
	 *  - endUpdate: called at the end of each world update routine
	 *  - addCell: called after each cell addition to the world (the new cell is thus
	 * guaranteed to be the last one in the cells container)
	 *  - destructor: called when world's destructor is called, just before all of the cells
	 * are deleted.
	 *
	 */
	template <typename P, typename... Rest>
	void registerPlugins(P &&p, Rest &&... otherPlugins) {
		// registerPlugin returns the onRegister hook from the plugin
		registerPlugin(std::forward<P>(p))(this);
		registerPlugins(std::forward<Rest>(otherPlugins)...);
	}
	void registerPlugins() {}  /// end of recursion

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
	 * @return number of updates
	 */
	size_t getNbUpdates() const { return frame; }

	/**
	 * @brief Creates a new cell and adds it through addCell()
	 *
	 * @tparam Args
	 * @param args Cell's constructor parameters
	 *
	 * @return a pointer to the new Cell
	 */
	template <typename... Args> Cell *createCell(Args &&... args) {
		Cell *c = new Cell(std::forward<Args>(args)...);
		addCell(c);
		return c;
	}

	/**
	 * @brief adds a cell to the new cells batch (which will be added to the main cells
	 * container at the end of the update cycle - or can be forced manually)
	 *
	 * Construction can be done by the user but deletion is automatically handled by
	 * the world (ex. ' addCell(new MyCell()); ' ). createCell(...) should probably be used
	 * instead.
	 *
	 * @param c a pointer to the cell
	 */
	void addCell(Cell *c) {
		if (c) {
			newCells.push_back(c);
			c->id = nbAddedCells++;
		}
	}

	/**
	 * @brief effectively adds the new cells that were registered by addCell
	 * triggers addCell hooks if there is something to add
	 */
	void addNewCells() {
		if (newCells.size()) {
			for (auto &f : hooks[eToUI(Hooks::onAddCell)]) f(this);
			cells.insert(cells.end(), newCells.begin(), newCells.end());
			newCells.clear();
		}
	}

	/**
	 * @brief this method triggers the allForcesAppliedToCells.
	 * It should be called by the embedded physics plugin just before updating positions
	 */
	void allForcesHaveBeenAppliedToCells() {
		for (auto &f : hooks[eToUI(Hooks::allForcesAppliedToCells)]) f(this);
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
	double getDt() { return dt; }

	/**
	 * @brief sets the period at which the world must call the updateBehavior method of each
	 * cell. This can be useful, for example, when the physics code must run at a different
	 * timescale than the behavior code.
	 *
	 * @param p the new period. updateBehavior will be called every p updates.
	 */
	void setUpdateBehaviorPeriod(size_t p) { updtBhvPeriod = p; }

	/**
	 * @brief returns a list of pair of connected cells
	 *
	 * @return
	 */
	std::vector<std::pair<cell_t *, cell_t *>> getConnectedCellsList() {
		std::unordered_set<ordered_pair<cell_t *>> res;
		for (auto &c : cells) {
			for (auto &connected : c->getConnectedCells())
				res.insert(make_ordered_cell_pair(c, connected));
		}
		std::vector<std::pair<cell_t *, cell_t *>> vecRes;
		for (auto &r : res) vecRes.push_back(std::make_pair(r.first, r.second));
		return vecRes;
	}

	/**
	 * @brief calls the updateBehavior of each cell, potentially in parallel
	 * (see parallelUpdateBehavior and nbThreads)
	 */
	void callUpdateBehavior() {
		const size_t MIN_CHUNK_SIZE = 2;
		const double AVG_TASKS_PER_THREAD = 3.0;
		if (parallelUpdateBehavior && nbThreads > 0) {
			threadpool.autoChunks(cells, MIN_CHUNK_SIZE, AVG_TASKS_PER_THREAD,
			                      [this](auto *c) { c->updateBehavior(*this); });
			threadpool.waitUntilLast();
		} else
			for (auto &c : cells) c->updateBehavior(*this);
	}

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
		for (auto &f : hooks[eToUI(Hooks::preBehaviorUpdate)]) f(this);
		if (frame % updtBhvPeriod == 0) callUpdateBehavior();
		addNewCells();
		for (auto &f : hooks[eToUI(Hooks::preDeleteDeadCellsUpdate)]) f(this);
		deleteDeadCells();
		for (auto &f : hooks[eToUI(Hooks::postBehaviorUpdate)]) f(this);
		for (auto &f : hooks[eToUI(Hooks::endUpdate)]) f(this);
		++frame;
	}

	/**
	 * @brief World's destructor. Triggers the destructor hooks and delete all cells.
	 */
	~World() {
		for (auto &f : hooks[eToUI(Hooks::destructor)]) f(this);
		while (!cells.empty()) delete cells.back(), cells.pop_back();
		while (!newCells.empty()) delete newCells.back(), newCells.pop_back();
	}

	EXPORTABLE(World, KV(frame), KV(cells));
};
}  // namespace MecaCell
#endif
