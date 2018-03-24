#ifndef SPRINGBODY_HPP
#define SPRINGBODY_HPP
#include "genericconnectionplugin.hpp"
#include "geometry/particle.hpp"
#include "integrators.hpp"
#include "orientable.h"
#include "spring.hpp"
#include "springconnection.hpp"
#include "utilities/grid.hpp"
#include "utilities/ordered_hash_map.hpp"
#include "utilities/ordered_pair.hpp"
#include "utilities/utils.hpp"

namespace MecaCell {
template <typename Cell> class SpringBody : public OrientedParticle {
	friend struct GenericConnectionBodyPlugin<Cell, SpringConnection>;

	Cell *cell = nullptr;

 public:
	double radius = Config::DEFAULT_CELL_RADIUS;        // radiius of the cell when at rest
	double stiffness = Config::DEFAULT_CELL_STIFFNESS;  // cell's body stiffness

	std::vector<SpringConnection<Cell> *> cellConnections;
	using embedded_plugin_t = GenericConnectionBodyPlugin<Cell, SpringConnection>;
	SpringBody(Cell *c, Vector3D pos = Vector3D::zero()) : OrientedParticle(pos), cell(c) {}
	void setRadius(double r) { radius = r; }
	double getRadius() { return radius; }
	double getBoundingBoxRadius() const { return radius; };
	double getStiffness() const { return stiffness; }
	void setStiffness(double k) { stiffness = k; }
	double getMomentOfInertia() const { return 0.4 * this->mass * radius * radius; }
	template <typename Integrator = Euler> void updatePositionsAndOrientations(double dt) {
		Integrator::updatePosition(*this, dt);
		Integrator::updateOrientation(*this, getMomentOfInertia(), dt);
	}

	std::tuple<Cell *, double> getConnectedCellAndMembraneDistance(const Vec &d) const {
		// /!\ assumes that d is normalized
		Cell *closestCell = nullptr;
		double closestDist = radius;
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

	void moveTo(Vector3D newpos) { this->setPosition(newpos); }
	void updateInternals(double) {}

	EXPORTABLE(SpringBody, KV(radius), KV(position));
};
}  // namespace MecaCell
#endif
