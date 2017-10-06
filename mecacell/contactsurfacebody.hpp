#ifndef CONTACTSURFACEBODY_HPP
#define CONTACTSURFACEBODY_HPP
#include <future>
#include <memory>
#include "contactsurface.hpp"
#include "genericconnectionplugin.hpp"
#include "geometry/particle.hpp"
#include "integrators.hpp"
#include "orientable.h"
#include "utilities/grid.hpp"
#include "utilities/ordered_hash_map.hpp"
#include "utilities/ordered_pair.hpp"

namespace MecaCell {

template <typename Cell> class ContactSurfaceBody : public OrientedParticle {
	friend struct GenericConnectionBodyPlugin<Cell, ContactSurface>;

	Cell *cell = nullptr;
	std::vector<ContactSurface<Cell> *> cellConnections;

	// params
	double incompressibility = 0.01;
	double membraneStiffness = 0.5;

	double restRadius = 40;  /// radiius of the cell when at rest
	// dynamic target radius:
	double dynamicRadius = restRadius;
	double prevDynamicRadius = dynamicRadius;
	static constexpr double MAX_DYN_RADIUS_RATIO = 2.0;
	double currentArea = 4.0 * M_PI * restRadius * restRadius;
	double restVolume = (4.0 * M_PI / 3.0) * restRadius * restRadius;
	double currentVolume = restVolume;
	double pressure = 0;
	bool volumeConservationEnabled = true;

 public:
	using embedded_plugin_t = GenericConnectionBodyPlugin<Cell, ContactSurface>;

	ContactSurfaceBody(Cell *c, Vector3D pos = Vector3D::zero())
	    : OrientedParticle(pos), cell(c){};

	void updateInternals(double dt) {
		computeCurrentAreaAndVolume();
		updateDynamicRadius(dt);
	}
	void setVolumeConservationEnabled(bool v) { volumeConservationEnabled = v; }
	void setRestVolume(double v) { restVolume = v; }
	void setRadius(double r) { restRadius = r; }
	double getDynamicRadius() const { return dynamicRadius; }
	double getBoundingBoxRadius() const { return dynamicRadius; };
	std::tuple<Cell *, double> getConnectedCellAndMembraneDistance(const Vec &d) const {
		// /!\ assumes that d is normalized
		Cell *closestCell = nullptr;
		double closestDist = dynamicRadius;
		for (auto &con : cellConnections) {
			auto direction = cell == con->cells.first ? -con->direction : con->direction;
			double dot = direction.dot(d);
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

	void computeCurrentAreaAndVolume() {
		restVolume = (4.0 * M_PI / 3.0) * restRadius * restRadius * restRadius;
		double volumeLoss = 0;
		// double surfaceLoss = 0;
		// cell connections
		for (auto &con : cellConnections) {
			auto &midpoint =
			    cell == con->cells.first ? con->midpoint.first : con->midpoint.second;
			auto h = dynamicRadius - midpoint;
			volumeLoss += (M_PI * h / 6.0) * (3.0 * con->sqradius + h * h);
			// surfaceLoss += 2.0 * M_PI * dynamicRadius * h - con->area;
		}
		// TODO : soustraire les overlapps
		double baseVol = (4.0 * M_PI / 3.0) * dynamicRadius * dynamicRadius * dynamicRadius;
		// double baseArea = (4.0 * M_PI) * dynamicRadius * dynamicRadius;
		currentVolume = baseVol - volumeLoss;
		// currentArea = baseArea - surfaceLoss;
		const double minVol = 0.1 * restVolume;
		// const double minArea =
		// 0.1 * getRestArea();  // garde fou en attendant de soustraire les overlaps
		if (currentVolume < minVol) currentVolume = minVol;
		// if (currentArea < minArea) currentArea = minArea;
	}

	void updateDynamicRadius(double dt) {
		if (volumeConservationEnabled) {
			double dA = max(0.0, currentArea - getRestArea());
			double dV = restVolume - currentVolume;
			auto Fv = incompressibility * dV;
			auto Fa = membraneStiffness * dA;
			pressure = Fv / currentArea;
			double dynSpeed = (dynamicRadius - prevDynamicRadius) / dt;
			double c = .1;
			dynamicRadius += dt * dt * (Fv - Fa - dynSpeed * c);
			if (dynamicRadius > restRadius * MAX_DYN_RADIUS_RATIO)
				dynamicRadius = restRadius * MAX_DYN_RADIUS_RATIO;
			else if (dynamicRadius < restRadius)
				dynamicRadius = restRadius;
		} else
			dynamicRadius = restRadius;
	}

	/**
	 * @brief uses
	 *
	 * @tparam Integrator
	 * @param dt
	 */
	template <typename Integrator = Euler> void updatePositionsAndOrientations(double dt) {
		Integrator::updatePosition(*this, dt);
		Integrator::updateOrientation(*this, getMomentOfInertia(), dt);
	}

	void solidifyAdhesions() {
		for (auto &c : cellConnections) c->fixedAdhesion = true;
	}
	void releaseAdhesions() {
		for (auto &c : cellConnections) c->fixedAdhesion = false;
	}

	inline Cell *getConnectedCell(const Vec &d) const {
		return get<0>(getConnectedCellAndMembraneDistance(d));
	}
	inline double getPreciseMembraneDistance(const Vec &d) const {
		return get<1>(getConnectedCellAndMembraneDistance(d));
	}
	inline double getRestArea() const { return 4.0 * M_PI * restRadius * restRadius; }
	inline void computeRestVolume() {
		restVolume = (4.0 * M_PI / 3.0) * restRadius * restRadius * restRadius;
	}
	inline double getRestMomentOfInertia() const {
		return 0.4 * this->getMass() * restRadius * restRadius;
	}

	inline double getMomentOfInertia() const { return getRestMomentOfInertia(); }
	inline double getVolumeVariation() const { return restVolume - currentVolume; }

	double getVolume() const { return restVolume; }
	// SET
	void setIncompressibility(double i) { incompressibility = i; }
	void setStiffness(double k) { membraneStiffness = k; }
	void setDynamicRadius(double r) { dynamicRadius = r; }
	double getPressure() const { return pressure; }
	double getRadius(void) const { return restRadius; }
	void moveTo(Vector3D newpos) { this->setPosition(newpos); }
};
}  // namespace MecaCell
#endif
