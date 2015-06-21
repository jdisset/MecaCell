#ifndef BASICCELL_HPP
#define BASICCELL_HPP
#include "connectablecell.hpp"

namespace MecaCell {
class BasicCell : public ConnectableCell<BasicCell> {
	bool isDividing = false;

public:
	using ConnectableCell<BasicCell>::ConnectableCell;

	double getAdhesionWith(const int) const {
		return 0.8; // whatever
	}

	BasicCell *updateBehavior(double) {
		if (isDividing) {
			// grow..
			isDividing = false;
			return divide();
		}
		return nullptr;
	}

	void startDivision() { isDividing = true; }
};
}

#endif
