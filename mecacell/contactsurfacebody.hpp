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
	num_t incompressibility = 0.01;
	num_t membraneStiffness = 0.5;

	num_t restRadius = 40;  /// radiius of the cell when at rest
	// dynamic target radius:
	num_t dynamicRadius = restRadius;
	num_t prevDynamicRadius = dynamicRadius;
	static constexpr num_t MAX_DYN_RADIUS_RATIO = 2.0;
	num_t currentArea = 4.0 * M_PI * restRadius * restRadius;
	num_t restVolume = (4.0 * M_PI / 3.0) * restRadius * restRadius;
	num_t currentVolume = restVolume;
	num_t pressure = 0;
	bool volumeConservationEnabled = true;

 public:
	using embedded_plugin_t = GenericConnectionBodyPlugin<Cell, ContactSurface>;

	ContactSurfaceBody(Cell *c, Vector3D pos = Vector3D::zero())
	    : OrientedParticle(pos), cell(c){};

	void updateInternals(num_t dt) {
		computeCurrentAreaAndVolume();
		updateDynamicRadius(dt);
	}
	void setVolumeConservationEnabled(bool v) { volumeConservationEnabled = v; }
	void setRestVolume(num_t v) { restVolume = v; }
	void setRadius(num_t r) { restRadius = r; }
	num_t getDynamicRadius() const { return dynamicRadius; }
	num_t getBoundingBoxRadius() const { return dynamicRadius; };
	std::tuple<Cell *, num_t> getConnectedCellAndMembraneDistance(const Vec &d) const {
		// /!\ assumes that d is normalized
		Cell *closestCell = nullptr;
		num_t closestDist = dynamicRadius;
		for (auto &con : cellConnections) {
			auto direction = cell == con->cells.first ? -con->direction : con->direction;
			num_t dot = direction.dot(d);
			if (dot < 0) {
				const auto &midpoint =
				    cell == con->cells.first ? con->midpoint.first : con->midpoint.second;
				num_t l = -midpoint / dot;
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
		num_t volumeLoss = 0;
		// num_t surfaceLoss = 0;
		// cell connections
		for (auto &con : cellConnections) {
			auto &midpoint =
			    cell == con->cells.first ? con->midpoint.first : con->midpoint.second;
			auto h = dynamicRadius - midpoint;
			volumeLoss += (M_PI * h / 6.0) * (3.0 * con->sqradius + h * h);
			// surfaceLoss += 2.0 * M_PI * dynamicRadius * h - con->area;
		}
		// TODO : soustraire les overlapps
		num_t baseVol = (4.0 * M_PI / 3.0) * dynamicRadius * dynamicRadius * dynamicRadius;
		// num_t baseArea = (4.0 * M_PI) * dynamicRadius * dynamicRadius;
		currentVolume = baseVol - volumeLoss;
		// currentArea = baseArea - surfaceLoss;
		const num_t minVol = 0.1 * restVolume;
		// const num_t minArea =
		// 0.1 * getRestArea();  // garde fou en attendant de soustraire les overlaps
		if (currentVolume < minVol) currentVolume = minVol;
		// if (currentArea < minArea) currentArea = minArea;
	}

	void updateDynamicRadius(num_t dt) {
		if (volumeConservationEnabled) {
			num_t dA = max(0.0, currentArea - getRestArea());
			num_t dV = restVolume - currentVolume;
			auto Fv = incompressibility * dV;
			auto Fa = membraneStiffness * dA;
			pressure = Fv / currentArea;
			num_t dynSpeed = (dynamicRadius - prevDynamicRadius) / dt;
			num_t c = .1;
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
	template <typename Integrator = Euler> void updatePositionsAndOrientations(num_t dt) {
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
	inline num_t getPreciseMembraneDistance(const Vec &d) const {
		return get<1>(getConnectedCellAndMembraneDistance(d));
	}
	inline num_t getRestArea() const { return 4.0 * M_PI * restRadius * restRadius; }
	inline void computeRestVolume() {
		restVolume = (4.0 * M_PI / 3.0) * restRadius * restRadius * restRadius;
	}
	inline num_t getRestMomentOfInertia() const {
		return 0.4 * this->getMass() * restRadius * restRadius;
	}

	inline num_t getMomentOfInertia() const { return getRestMomentOfInertia(); }
	inline num_t getVolumeVariation() const { return restVolume - currentVolume; }

	num_t getVolume() const { return restVolume; }
	// SET
	void setIncompressibility(num_t i) { incompressibility = i; }
	void setStiffness(num_t k) { membraneStiffness = k; }
	void setDynamicRadius(num_t r) { dynamicRadius = r; }
	num_t getPressure() const { return pressure; }
	num_t getRadius(void) const { return restRadius; }
	void moveTo(Vector3D newpos) { this->setPosition(newpos); }
};
}  // namespace MecaCell
#endif
