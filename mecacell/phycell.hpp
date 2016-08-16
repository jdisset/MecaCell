#ifndef PHYCELL_H
#define PHYCELL_H
#include <array>
#include "pressuremembrane.hpp"
#include "utilities/unique_vector.hpp"

namespace MecaCell {
template <typename Derived, template <class> class Membrane = PressureMembrane>
class PhyCell {
	/************************ PhyCell class template ******************************/
	// Abstract:
	// A PhyCell is the basis for every particle-based cell a user might want to use in
	// mecacell. It uses the curriously reccuring template pattern : Derived is the user's
	// type inheriting PhyCell.
	// Membrane is the type of membrane used by the cell. A membrane is a low level class
	// that handles everything related to physics (deformations, connections, volume).
	// A PhyCell is a more abstract interface meant to expose higher instinctive,
	// meaningful parameters and methods. It also represents the cell's center, which can
	// be seen as its kernel (it is NOT always the center of mass, but often close enough)

	friend class Membrane<Derived>;

 public:
	using membrane_t = Membrane<Derived>;

 protected:
	membrane_t membrane;
	bool dead = false;
	std::array<double, 3> color = {{0.75, 0.12, 0.07}};
	bool tested = false;  // has already been tested for collision
	unique_vector<Derived *> connectedCells;
	bool visible = true;

 public:
	size_t id = 0;  // num of cell by order of addition in world
	PhyCell();
	PhyCell(const Derived &c)
	    : Movable(c.getPosition()),
	      membrane(static_cast<Derived *>(this), c.membrane),
	      dead(false),
	      color(c.color),
	      tested(false) {}

	PhyCell(const PhyCell &c) : PhyCell(static_cast<const Derived &>(c)) {}

	PhyCell(const Derived *c) : PhyCell(*c) {}

	PhyCell(Vec pos) : Movable(pos), membrane(static_cast<Derived *>(this)) {
		randomColor();
	}

	PhyCell(const Derived &c, const Vec &translation)
	    : Movable(c.getPosition() + translation, c.mass),
	      membrane(static_cast<Derived *>(this), c.membrane),
	      dead(false),
	      color(c.color),
	      tested(false) {}

	/************** GET ******************/
	inline membrane_t &getMembrane() { return membrane; }
	inline const membrane_t &getConstMembrane() const { return membrane; }
	inline double getColor(unsigned int i) const {
		if (i < 3) return color[i];
		return 0;
	}
	inline const std::vector<Derived *> &getConnectedCells() const {
		return connectedCells.getUnderlyingVector();
	}
	bool alreadyTested() const { return tested; }
	int getNbConnections() const { return connectedCells.size(); }
	bool getVisible() const { return visible; }
};
}
#endif /* PHYCELL_H */
