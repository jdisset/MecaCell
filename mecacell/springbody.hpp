#ifndef SPRINGBODY_HPP
#define SPRINGBODY_HPP
#include "genericconnectionplugin.hpp"
#include "integrators.hpp"
#include "orientable.h"
#include "spring.hpp"
#include "springconnection.hpp"
#include "utilities/grid.hpp"
#include "utilities/ordered_hash_map.hpp"
#include "utilities/ordered_pair.hpp"
#include "utilities/utils.h"

namespace MecaCell {
template <typename Cell> class SpringBody : public Orientable {
	friend class GenericConnectionBodyPlugin<Cell, SpringConnection>;

	Cell *cell = nullptr;
	std::vector<SpringConnection<Cell> *> cellConnections;
	double restRadius = Config::DEFAULT_CELL_RADIUS;    // radiius of the cell when at rest
	double stiffness = Config::DEFAULT_CELL_STIFFNESS;  // cell's body stiffness

 public:
	using embedded_plugin_t = GenericConnectionBodyPlugin<Cell, SpringConnection>;
	SpringBody(Cell *c) : cell(c) {}
	void setRestRadius(double r) { restRadius = r; }
	double getBoundingBoxRadius() const { return restRadius; };
	double getStiffness() const { return stiffness; }
	double getMomentOfInertia() const {
		return 0.4 * cell->getMass() * restRadius * restRadius;
	}
	template <typename Integrator = Euler> void updatePositionsAndOrientations(double dt) {
		Integrator::updatePosition(*cell, dt);
		Integrator::updateOrientation(*this, dt);
	}

	std::tuple<Cell *, double> getConnectedCellAndMembraneDistance(const Vec &d) const {
		// /!\ assumes that d is normalized
		Cell *closestCell = nullptr;
		double closestDist = restRadius;
		for (auto &con : cellConnections) {
			auto normal = cell == con->cells.first ? -con->direction : con->direction;
			double dot = normal.dot(d);
			if (dot < 0) {
				const auto &midpoint =
				    cell == con->cells.first ? con->midpoint.first : con->midpoint.second;
				double l = -midpoint / dot;
				if (l < closestDist) {
					closestDist = l;
					closestCell = con->cells.first == cell ? con->cells.second : con->cells.first;
				}
			}
		}
		return std::make_tuple(closestCell, closestDist);
	}
	inline Cell *getConnectedCell(const Vec &d) const {
		return get<0>(getConnectedCellAndMembraneDistance(d));
	}
	// required by GenericConnection Plugin
	inline double getPreciseMembraneDistance(const Vec &d) const {
		return get<1>(getConnectedCellAndMembraneDistance(d));
	}
	void updateInternals(double) {}
};
}
#endif
