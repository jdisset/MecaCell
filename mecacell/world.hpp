#ifndef MECACELL_WORLD_H
#define MECACELL_WORLD_H
#include <array>
#include <functional>
#include <mutex>
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
template <typename Cell, typename Body, typename Integrator = Euler> class World {
 private:
	size_t nbThreads = 0;

 public:
	using cell_t = Cell;
	using body_t = Body;
	using integrator_t = Integrator;
	using hook_s = void(World *);  /// hook signature, as they should appear in
	                               /// plugins classes. cf. hooks & plugins section

	ThreadPool threadpool;  // threadpool that can be used by plugins (and is also used to
	                        // parallelize updateBehavior)

	/// A body type can define one embedded plugin class type which will be
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
	using bodyPlugin_t =
	    typename embedded_plugin_type<body_t>::type;  // either dumb empty struct or the
	                                                  // plugin type
	                                                  // defined by Body (if existing)
 protected:
	unsigned long long int frame = 0;         /// +1 at each update. cf getNbUpdates()
	unsigned long long int nbAddedCells = 0;  /// +1 on cell add. Used for cell's unique ids
	size_t updtBhvPeriod =
	    1;  /// period at which the world should call the cells updateBehavior method.
	bool parallelUpdateBehavior = false;

	std::mutex cellLock;  // for concurrent addCells;

	/**
	 * @brief removes all cells marked dead
	 */
	void deleteDeadCells() {
		// TODO
		assert(bodies.size() == cells.size());
		for (size_t i = 0; i < cells.size(); ++i) {
		}
	}

	/* HOOKS
	 */
	DECLARE_HOOK(onAddCell, beginUpdate, preBehaviorUpdate, preDeleteDeadCellsUpdate,
	             postBehaviorUpdate, endUpdate, allForcesAppliedToCells, destructor)

 public:
	double dt = 1.0 / 100.0;  // The amount by which time is increased every update
	bodyPlugin_t bodyPlugin;  // instance of the embedded cell plugin type
	                          // (bodyPlugin_t default to a dumb char if not specified)

	std::vector<cell_t> newCells;   // cells that are registered to be added
	std::vector<body_t> newBodies;  // all the bodies are in this container

	/* CELLS */
	std::vector<cell_t> cells;   // all the cells are in this container
	std::vector<body_t> bodies;  // all the bodies are in this container

	/**
	 * @brief World constructor, registers the (optional) cell type's embedded plugin
	 * instance.
	 *
	 * @param nbThreads the number of threads to be used by plugins (physics, for example)
	 * or cells (through their updateBehavior method)
	 * that can take advantage of parallelism
	 */
	World(size_t nThreads = 0) : nbThreads(nThreads), threadpool(nThreads) {
		registerPlugins(bodyPlugin);
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
	 * @brief adds a cell to the new cells batch (which will be added to the main cells
	 * container at the end of the update cycle - or can be forced manually)
	 * Its body is assumed to be bodies.back();
	 *
	 * @param c a cell
	 * @param b its body
	 *
	 * returns the cell id
	 */
	size_t addCell(cell_t &&c, Body &&b) {
		newCells.emplace_back(c);
		newBodies.emplace_back(b);
		newCells.back().id = nbAddedCells;
		newBodies.back().id = nbAddedCells;
		nbAddedCells++;
	}

	/**
	 * @brief effectively adds the new cells that were registered by addCell
	 * triggers addCell hooks if there is something to add
	 */
	void addNewCells() {
		if (newCells.size()) {
			auto prevSize = newCells.size();
			for (auto &f : hooks[eToUI(Hooks::onAddCell)]) f(this);

			cells.reserve(newCells.size());
			for (auto &c : newCells) cells.emplace_back(std::move(c));
			bodies.reserve(newBodies.size());
			for (auto &b : newBodies) bodies.emplace_back(std::move(b));

			newCells.clear();
			newBodies.clear();
			// reserve for next round step (+10%)
			newCells.reserve(prevSize + (prevSize / 10));
			newBodies.reserve(prevSize + (prevSize / 10));
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
	 * @brief calls the updateBehavior of each cell, potentially in parallel
	 * (see parallelUpdateBehavior and nbThreads)
	 */
	void callUpdateBehavior() {
		const size_t S = cells.size();
		/*     if (parallelUpdateBehavior && nbThreads > 0) {*/
		// const size_t MIN_CHUNK_SIZE = 500;
		// const double AVG_TASKS_PER_THREAD = 2000.0;
		// threadpool.autoChunks(cells, MIN_CHUNK_SIZE, AVG_TASKS_PER_THREAD,
		//[this](auto *c) { c->updateBehavior(*this); });
		// threadpool.waitUntilLast();
		/*} else*/

		for (size_t i = 0; i < S; ++i) cells[i].updateBehavior(*this, bodies[i]);
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
		// std::cerr<<"WORLD UPDATE 0"<<std::endl;
		for (auto &f : hooks[eToUI(Hooks::beginUpdate)]) f(this);
		// std::cerr<<"WORLD UPDATE 1"<<std::endl;
		for (auto &f : hooks[eToUI(Hooks::preBehaviorUpdate)]) f(this);
		// std::cerr<<"WORLD UPDATE 2"<<std::endl;
		if (frame % updtBhvPeriod == 0) callUpdateBehavior();
		// std::cerr<<"WORLD UPDATE 3"<<std::endl;
		addNewCells();
		// std::cerr<<"WORLD UPDATE 4"<<std::endl;
		for (auto &f : hooks[eToUI(Hooks::preDeleteDeadCellsUpdate)]) f(this);
		// std::cerr<<"WORLD UPDATE 5"<<std::endl;
		deleteDeadCells();
		// std::cerr<<"WORLD UPDATE 6"<<std::endl;
		for (auto &f : hooks[eToUI(Hooks::postBehaviorUpdate)]) f(this);
		// std::cerr<<"WORLD UPDATE 7"<<std::endl;
		for (auto &f : hooks[eToUI(Hooks::endUpdate)]) f(this);
		// std::cerr<<"WORLD UPDATE 8"<<std::endl;
		++frame;
	}

	/**
	 * @brief World's destructor. Triggers the destructor hooks and delete all cells.
	 */
	~World() {
		for (auto &f : hooks[eToUI(Hooks::destructor)]) f(this);
	}

	// EXPORTABLE(World, KV(frame), KV(dt), KV(cells));
	EXPORTABLE(World, KV(frame), KV(dt));
};
}  // namespace MecaCell
#endif
