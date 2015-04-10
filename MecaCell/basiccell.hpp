#ifndef BASICCELL_HPP
#define BASICCELL_HPP
#include "connectablecell.hpp"

namespace MecaCell {
class BasicCell : public ConnectableCell<BasicCell> {
 public:
	using ConnectableCell<BasicCell>::ConnectableCell;

	double getAdhesionWith(const int) const {
		return 0.8;  // whatever
	}

	BasicCell* updateBehavior(double) { return nullptr; }
};
}

#endif
