#ifndef CONNECTABLECELL_HPP
#define CONNECTABLECELL_HPP
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
#include "geometry/rotation.h"
#include "movable.h"
#include "orientable.h"
#include "utilities/unique_vector.hpp"
#include "utilities/utils.h"
#include "volumemembrane.hpp"

namespace MecaCell {

/**
 * @brief Basis for every cell a user might want to use.
 *
 * It uses the curriously reccuring template pattern : Derived is the user's type
 * inheriting ConnectableCell.
 * Membrane is the type of membrane used by the cell. A membrane is a low level class
 * that handles everything related to physics (deformations, connections, volume).
 * A ConnectableCell is a more abstract interface meant to expose higher instinctive,
 * meaningful parameters and methods. It also represents the cell's center, which can
 * be seen as its kernel (it is NOT always the center of mass, but often close enough)
 *
 * @tparam Derived the user's cell class inheriting from ConnectableCell
 * @tparam Membrane the membrane implemebtation (which contains most of the core logic)
 */
template <typename Derived, template <class> class Membrane = VolumeMembrane>
class ConnectableCell : public Movable, public Orientable {
	friend class Membrane<Derived>;
	friend typename Membrane<Derived>::CCCM;

 public:
	using membrane_t = Membrane<Derived>;
	static constexpr bool hasModelCollisions = membrane_t::hasModelCollisions;
	using CellCellConnectionContainer = typename membrane_t::CellCellConnectionContainer;

 protected:
	membrane_t membrane;  // core implementation
	bool dead = false;
	array<double, 3> color = {{0.75, 0.12, 0.07}};
	bool tested = false;                      // has already been tested for collision
	unique_vector<Derived *> connectedCells;  // list of currently connected cells

	/**
	 * @brief disconnect a neighboring cell
	 *
	 * removes it from the connectedCells contianer
	 *
	 * @param cell pointer to the cell to be disconnected
	 */
	void eraseCell(Derived *cell) {
		connectedCells.erase(remove(connectedCells.begin(), connectedCells.end(), cell),
		                     connectedCells.end());
	}

	// raises/lower flag for collision detection
	void markAsTested() { tested = true; }
	void markAsNotTested() { tested = false; }
	bool alreadyTested() const { return tested; }

	// helpers & shortcuts
	inline Derived *selfptr() { return static_cast<Derived *>(this); }
	inline Derived &self() { return static_cast<Derived &>(*this); }
	const Derived &selfconst() const { return static_cast<const Derived &>(*this); }
	static inline void checkAndCreateConnection(Derived *c0, Derived *c1,
	                                            CellCellConnectionContainer &con) {
		membrane_t::checkAndCreateConnection(c0, c1, con);
	}

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

	/**
	 * @brief Copy constructor
	 *
	 * @param c the other cell
	 */
	ConnectableCell(const Derived *c) : ConnectableCell(*c) {}

	/**
	 * @brief Constructor
	 *
	 * @param pos initial position of the cell's kernel
	 */
	ConnectableCell(Vec pos) : Movable(pos), membrane(static_cast<Derived *>(this)) {}

	ConnectableCell() : ConnectableCell(Vec(0, 0, 0)) {}

	/**
	 * @brief Copy constructor with translation
	 *
	 * Useful for division, for example. Shifts the created cell relatively to the copied
	 * one.
	 *
	 * @param c the copied cell
	 * @param translation the vector by which the created cell is translated
	 */
	ConnectableCell(const Derived &c, const Vec &translation)
	    : Movable(c.getPosition() + translation, c.mass),
	      membrane(static_cast<Derived *>(this), c.membrane),
	      dead(false),
	      color(c.color),
	      tested(false) {}

	/*************** UPDATES **************/

	/**
	 * @brief Calls the membrane's core update routine
	 *
	 * @tparam Integrator
	 * @param dt
	 */
	template <typename Integrator> void inline updatePositionsAndOrientations(double dt) {
		membrane.template updatePositionsAndOrientations<Integrator>(dt);
	}

	/**
	 * @brief calls the membrane's update method
	 */
	void updateStats() { membrane.updateStats(); }

	/**
	 * @brief orders the cell to grow
	 *
	 * @param qtty percentage relative to the base volume by which the cell must grow
	 */
	void grow(double qtty) {
		double rv = getRelativeVolume() + qtty;
		setVolume(getBaseVolume() * rv);
		setMass(getBaseMass() * rv);
	}

	/**
	 * @brief flags the cell as dead so it can be cleanly removed from the world
	 */
	void die() { dead = true; }
	bool isDead() { return dead; }

	/**
	 * @brief setsThe current volume of the cell
	 *
	 * @param v
	 */
	void setVolume(double v) { membrane.setVolume(v); }

	void resetForces() { membrane.resetForces(); }

	void setColorRGB(int r, int g, int b) {
		color = {{static_cast<double>(r) / 255.0, static_cast<double>(g) / 255.0,
		          static_cast<double>(b) / 255.0}};
	}

	void setColorHSV(double H, double S, double V) {
		// h =	[0, 360]; s, v = [0, 1]
		double C = V * S;  // Chroma
		double HPrime = fmod(H / 60.0, 6.0);
		double X = C * (1.0 - fabs(fmod(HPrime, 2.0) - 1.0));
		double M = V - C;
		double R, G, B;
		if (0 <= HPrime && HPrime < 1) {
			R = C;
			G = X;
			B = 0;
		} else if (1 <= HPrime && HPrime < 2) {
			R = X;
			G = C;
			B = 0;
		} else if (2 <= HPrime && HPrime < 3) {
			R = 0;
			G = C;
			B = X;
		} else if (3 <= HPrime && HPrime < 4) {
			R = 0;
			G = X;
			B = C;
		} else if (4 <= HPrime && HPrime < 5) {
			R = X;
			G = 0;
			B = C;
		} else if (5 <= HPrime && HPrime < 6) {
			R = C;
			G = 0;
			B = X;
		} else {
			R = 0;
			G = 0;
			B = 0;
		}
		R += M;
		G += M;
		B += M;
		color = {{R, G, B}};
	}

	/**
	 * @brief dumps internal infos
	 *
	 * @return a formatted string
	 */
	string toString() {
		stringstream s;
		s << "Cell " << id << " :" << std::endl;
		s << " position = " << position << std::endl;
		s << " velocity = " << velocity << ", angular velocity = " << angularVelocity
		  << std::endl;
		s << " force = " << force << ", torque " << torque << std::endl;
		s << " nbConnections = " << connectedCells.size();
		return s.str();
	}

	/************** GET ******************/
	membrane_t &getMembrane() { return membrane; }
	const membrane_t &getConstMembrane() const { return membrane; }
	double getBaseVolume() const { return membrane.getBaseVolume(); }
	double getVolume() const { return membrane.getVolume(); }
	double getMembraneDistance(const Vec &d) const {
		return membrane.getPreciseMembraneDistance(d);
	};
	double getBoundingBoxRadius() const { return membrane.getBoundingBoxRadius(); }
	double getColor(unsigned int i) const {
		if (i < 3) return color[i];
		return 0;
	}
	const std::vector<Derived *> &getConnectedCells() const {
		return connectedCells.getUnderlyingVector();
	}
	double getPressure() const { return membrane.getPressure(); }

	// computed

	double getMomentOfInertia() const { return membrane.getMomentOfInertia(); }
	double getRelativeVolume() const { return getVolume() / getBaseVolume(); }
	double getNormalizedPressure() const {
		double sign = getPressure() >= 0 ? 1 : -1;
		return 0.5 + sign * 0.5 * (1.0 - exp(-abs(60.0 * getPressure())));
	}
	int getNbConnections() const { return connectedCells.size(); }
};
}
#endif
