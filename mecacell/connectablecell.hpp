#ifndef MECACELL_CONNECTABLECELL_HPP
#define MECACELL_CONNECTABLECELL_HPP
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
#include "contactsurfacebody.hpp"
#include "geometry/rotation.h"
#include "movable.h"
#include "orientable.h"
#include "utilities/exportable.hpp"
#include "utilities/unique_vector.hpp"
#include "utilities/utils.hpp"

namespace MecaCell {

/**
 * @brief Basis for every cell a user might want to use.
 *
 * It uses the curriously reccuring template pattern : Derived is the user's type
 * inheriting ConnectableCell.
 * Body is the type of body used by the cell. A body is a low level class
 * that handles everything related to physics (deformations, connections, volume).
 * A ConnectableCell is a more abstract interface meant to expose higher instinctive,
 * meaningful parameters and methods. It also represents the cell's center, which can
 * be seen as its kernel (it is NOT always the center of mass, but often close enough)
 *
 * @tparam Body the body implementation (which contains most of the core logic)
 */
template <class Derived, template <class> class Body = ContactSurfaceBody>
class ConnectableCell {
 public:
	using body_t = Body<Derived>;
	using embedded_plugin_t = typename body_t::embedded_plugin_t;
	// using vec_t = decltype((body_t) nullptr->getPosition());
	friend body_t;
	friend embedded_plugin_t;

 protected:
	body_t body;  // core implementation
	bool dead = false;

	// viewer related attributes. They probably could be moved elswhere but are clearly
	// harmless and greatly simplify basic usage of a simple viewer. They might also be used
	// toconvey information in headless mode
	bool isVisible = true;  // should we display this cell? (cosmetic only - cell is still
	                        // present and active -)
	array<double, 3> color = {
	    {0.75, 0.12, 0.07}};  // cell's color (interpreted as RGB by viewer)

	unique_vector<Derived *> connectedCells;  // list of currently connected cells

	// helpers & shortcuts
	inline Derived *selfptr() { return static_cast<Derived *>(this); }
	inline Derived &self() { return static_cast<Derived &>(*this); }
	const Derived &selfconst() const { return static_cast<const Derived &>(*this); }

 public:
	/**
	 * @brief disconnect a neighboring cell
	 *
	 * removes it from the connectedCells contianer
	 *
	 * @param cell pointer to the cell to be disconnected
	 */
	void eraseConnectedCell(Derived *cell) { connectedCells.erase(cell); }
	void addConnectedCell(Derived *c) { connectedCells.insert(c); }
	void clearConnectedCells() { connectedCells.clear(); }
	bool isConnectedTo(Derived *c) { return connectedCells.count(c); }

	size_t id = 0;  // mostly for debugging, num of cell by order of addition in world
	size_t getId() const { return id; }
	ConnectableCell(const Derived &c)
	    : body(static_cast<Derived *>(this)), dead(false), color(c.color) {}

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
	ConnectableCell(Vec pos) : body(static_cast<Derived *>(this), pos) {}

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
	      body(static_cast<Derived *>(this)),
	      dead(false),
	      color(c.color) {}

	/*************** UPDATES **************/

	/**
	 * @brief flags the cell as dead so it can be cleanly removed from the world
	 */
	void die() { dead = true; }
	bool isDead() { return dead; }
	Vector3D getPosition() const { return body.getPosition(); }

	void setColorRGB(size_t r, size_t g, size_t b) {
		color = {{static_cast<double>(r) / 255.0, static_cast<double>(g) / 255.0,
		          static_cast<double>(b) / 255.0}};
	}
	void setColorRGB(std::array<int, 3> rgb) { setColorRGB(rgb[0], rgb[1], rgb[2]); }
	void setColorRGB(std::array<double, 3> rgb) { color = rgb; }
	void setColorHSV(double H, double S, double V) { color = hsvToRgb(H, S, V); }
	void setColorHSV(std::array<double, 3> hsv) { setColorHSV(hsv[0], hsv[1], hsv[2]); }

	/**
	 * @brief should a viewer displpay this cell ?
	 *
	 * @param v
	 */
	void setVisible(bool v) { isVisible = v; }

	/**
	 * @brief dumps internal infos
	 *
	 * @return a formatted string
	 */
	string toString() {
		stringstream s;
		s << "Cell " << id << " :" << std::endl;
		s << " position = " << getPosition() << std::endl;
		s << " nbConnections = " << connectedCells.size();
		return s.str();
	}

	/************** GET ******************/
	body_t &getBody() { return body; }
	const body_t &getConstBody() const { return body; }
	double getBoundingBoxRadius() const { return body.getBoundingBoxRadius(); }
	double getColor(unsigned int i) const {
		if (i < 3) return color[i];
		return 0;
	}
	const std::vector<Derived *> &getConnectedCells() const {
		return connectedCells.getUnderlyingVector();
	}
	bool getVisible() { return isVisible; }
	int getNbConnections() const { return connectedCells.size(); }

	EXPORTABLE(ConnectableCell, KV(body), KV(id));
};
}  // namespace MecaCell
#endif
