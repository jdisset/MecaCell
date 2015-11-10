#ifndef CONNECTABLECELL_HPP
#define CONNECTABLECELL_HPP
#include <vector>
#include <deque>
#include <array>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <unordered_set>
#include <functional>
#include "rotation.h"
#include "movable.h"
#include "orientable.h"
#include "model.h"
#include "spheremembrane.hpp"

using namespace std;

namespace MecaCell {

template <typename Derived, template <class> class Membrane = SphereMembrane>
class ConnectableCell : public Movable, public Orientable {
	friend class Membrane<Derived>;
	friend typename Membrane<Derived>::CCCM;
	/************************ ConnectableCell class template ******************************/
	// Abstract:
	// A ConnectableCell is the basis for every cell a user might want to use in mecacell.
	// It uses the curriously reccuring template pattern : Derived is the user's type
	// inheriting ConnectableCell.
	// Membrane is the type of membrane used by the cell. A membrane is a low level class
	// that handles everything related to physics (deformations, connections, volume).
	// A ConnectableCell is a more abstract interface meant to expose higher instinctive,
	// meaningful parameters and methods. It also represents the cell's center, which can
	// be seen as its kernel (it is NOT always the center of mass, but often close enough)
 public:
	using membrane_t = Membrane<Derived>;
	using CellCellConnectionContainer = typename membrane_t::CellCellConnectionContainer;
	using CellModelConnectionContainer = typename membrane_t::CellModelConnectionContainer;

 protected:
	membrane_t membrane;
	bool dead = false;
	array<float_t, 3> color = {{0.75, 0.12, 0.07}};
	bool tested = false;  // has already been tested for collision
	unordered_set<Derived *> connectedCells;
	bool visible = true;

 public:
	size_t id = 0;  // mostly for debugging, num of cell by order of addition in world
	ConnectableCell(const Derived &c)
	    : Movable(c.getPosition()),
	      membrane(static_cast<Derived *>(this), c.membrane),
	      dead(false),
	      color(c.color),
	      tested(false) {}

	ConnectableCell(const ConnectableCell &c)
	    : ConnectableCell(static_cast<const Derived &>(c)) {}

	ConnectableCell(const Derived *c) : ConnectableCell(*c) {}

	ConnectableCell(Vec pos) : Movable(pos), membrane(static_cast<Derived *>(this)) {
		randomColor();
	}

	ConnectableCell(const Derived &c, const Vec &translation)
	    : Movable(c.getPosition() + translation, c.mass),
	      membrane(static_cast<Derived *>(this), c.membrane),
	      dead(false),
	      color(c.color),
	      tested(false) {}

	/*************** STATIC **************/
	template <typename SpacePartition>
	static inline void checkForCellCellConnections(
	    vector<Derived *> &cells, CellCellConnectionContainer &cellCellConnections,
	    SpacePartition &grid) {
		membrane_t::checkForCellCellConnections(cells, cellCellConnections, grid);
	}
	template <typename SpacePartition>
	static inline void checkForCellModelConnections(
	    vector<Derived *> &cells, unordered_map<string, Model> models,
	    CellModelConnectionContainer &cellModelConnections, SpacePartition &modelGrid) {
		membrane_t::checkForCellModelCollisions(cells, models, cellModelConnections,
		                                        modelGrid);
	}
	static inline void updateCellCellConnections(CellCellConnectionContainer &c,
	                                             float_t dt) {
		membrane_t::updateCellCellConnections(c, dt);
	}
	static inline void updateCellModelConnections(CellModelConnectionContainer &c,
	                                              float_t dt) {
		membrane_t::updateCellModelConnections(c, dt);
	}

	static inline void disconnectAndDeleteAllConnections(Derived *c0,
	                                                     CellCellConnectionContainer &con) {
		membrane_t::disconnectAndDeleteAllConnections(c0, con);
	}

	template <typename Integrator> void inline updatePositionsAndOrientations(double dt) {
		membrane.template updatePositionsAndOrientations<Integrator>(dt);
	}

	/************** GET ******************/
	// basics
	inline membrane_t &getMembrane() { return membrane; }
	inline float_t getBaseVolume() const { return membrane.getBaseVolume(); }
	inline float_t getVolume() const { return membrane.getVolume(); }
	inline float_t getMembraneDistance(const Vec &d) const {
		return membrane.getPreciseMembraneDistance(d);
	};
	inline float_t getBoundingBoxRadius() const { return membrane.getBoundingBoxRadius(); }
	inline float_t getColor(unsigned int i) const {
		if (i < 3) return color[i];
		return 0;
	}
	inline const std::unordered_set<Derived *> &getConnectedCells() const {
		return connectedCells;
	}
	inline float_t getPressure() const { return membrane.getPressure(); }

	// Don't forget to implement this method in the derived class
	inline float_t getAdhesionWith(const Derived *d) const {
		return selfconst().getAdhesionWith(d);
	}
	inline float_t getAdhesionWithModel(const string &) const { return 0.7; }

	// computed

	inline float_t getMomentOfInertia() const { return membrane.getMomentOfInertia(); }
	inline float_t getRelativeVolume() const { return getVolume() / getBaseVolume(); }
	float_t getNormalizedPressure() const {
		float_t sign = getPressure() >= 0 ? 1 : -1;
		return 0.5 + sign * 0.5 * (1.0 - exp(-abs(10.0 * getPressure())));
	}
	bool alreadyTested() const { return tested; }
	int getNbConnections() const { return connectedCells.size(); }
	bool getVisible() const { return visible; }

	string toString() const {
		stringstream s;
		s << "Cell " << id << " :" << endl;
		s << " position = " << hexstr(position) << ", orientation = " << orientation << endl;
		s << " velocity = " << hexstr(velocity) << ", angular velocity = " << angularVelocity
		  << endl;
		s << " force = " << force << " ; " << hexstr(force) << endl;
		s << " nbConnections = " << connectedCells.size() << endl;
		return s.str();
	}

	/************** SET ******************/
	void setVisible(bool v) { visible = v; }
	void markAsTested() { tested = true; }
	void markAsNotTested() { tested = false; }
	void setVolume(float_t v) { membrane.setVolume(v); }
	inline void resetForces() { membrane.resetForces(); }
	// return the connection length with another cell
	// according to an adhesion coef (0 <= adh <= 1)
	inline Derived *selfptr() { return static_cast<Derived *>(this); }
	inline Derived &self() { return static_cast<Derived &>(*this); }
	const Derived &selfconst() const { return static_cast<const Derived &>(*this); }

	template <typename C = Derived> C *divide() { return divide<C>(Vec::randomUnit()); }

	template <typename C = Derived> C *divide(const Vec &direction) {
		setMass(getBaseMass());
		membrane.division();
		C *newC =
		    new C(selfconst(), direction.normalized() * getMembraneDistance(direction) * 0.8);
		return newC;
	}

	void grow(float_t qtty) {
		float_t rv = getRelativeVolume() + qtty;
		setVolume(getBaseVolume() * rv);
		setMass(getBaseMass() * rv);
	}

	// erase cell from the connectedCells container
	void eraseCell(Derived *cell) {
		unsigned int prevC = connectedCells.size();
		connectedCells.erase(remove(connectedCells.begin(), connectedCells.end(), cell),
		                     connectedCells.end());
		assert(connectedCells.size() == prevC - 1 || prevC == 0);
	}

	static inline void checkAndCreateConnection(Derived *c0, Derived *c1,
	                                            CellCellConnectionContainer &con) {
		membrane_t::checkAndCreateConnection(c0, c1, con);
	}

	/******************************
	 * division and control
	 *****************************/

	void updateStats() { membrane.updateStats(); }

	Derived *updateBehavior(float_t dt) { return self().updateBehavior(dt); }

	void die() { dead = true; }
	bool isDead() { return dead; }

	void randomColor() {
		float_t r0 = 0.0001 * (rand() % 10000);
		float_t r1 = 0.0001 * (rand() % 10000);
		if (false) {
			if (r0 < 1.0 / 3.0) {
				/// green
				color[0] = 0.1 + (0.4 * r1);
				color[1] = 0.78 + (0.2 * r0);  // + (0.1 * r0);
				color[2] = 0.06 + 0.6 * r0;
			} else if (r0 < 2.0 / 3.0) {
				// yellow
				color[0] = 0.9 + (0.1 * r1);
				color[1] = 0.73;  // + (0.1 * r0);
				color[2] = 0.36 + 0.2 * r0;
			} else {
				// blue
				color[0] = 0.05 * r1;
				color[1] = 0.6 + (0.1 * r1);
				color[2] = 0.7 + (0.2 * r0);
			}
		}
		color[0] = 0.6 + (0.3 * r1);
		color[1] = 0.3 * r1;
		color[2] = 0.05 + (0.2 * r0);
	}
};
}
#endif
