#ifndef ELASTICBODY_HPP
#define ELASTICBODY_HPP
#include "elasticconnection.hpp"
#include "genericconnectionplugin.hpp"
#include "geometry/particle.hpp"
#include "integrators.hpp"
#include "orientable.h"
#include "spring.hpp"
#include "utilities/grid.hpp"
#include "utilities/ordered_hash_map.hpp"
#include "utilities/ordered_pair.hpp"
#include "utilities/utils.hpp"

namespace MecaCell {
template <typename H> struct GhostCenter : public OrientedParticle {
	H &host;
	num_t fK = 3.0;
	num_t tK = 1.0;
	num_t prevDist = 0;
	GhostCenter(H &h) : OrientedParticle(h.getPosition()), host(h) {
		this->mass = host.getMass();
		this->position = h.getPosition();
		this->orientation = h.getOrientation();
		this->orientationRotation = h.getOrientationRotation();
	}
	template <typename Integrator = Euler> void update(num_t dt, num_t momentOfInertia) {
		this->mass = host.getMass();
		logger<DBG>("GhostCenter force = ", this->force);
		Integrator::updatePosition(*this, dt);
		Integrator::updateOrientation(*this, momentOfInertia, dt);
		auto dir = this->position - host.getPosition();
		num_t l = dir.length();
		if (l > 0) {
			dir = dir / l;
			num_t f = std::min(1.0 / dt, l * fK);
			host.receiveForce(dir * f);
			host.receiveForce(-host.getVelocity() * 0.05);
			logger<DBG>("Host total force = ", host.getForce(), " (sent = ", dir * f, ")");
		}
		prevDist = l;
		// auto rot = Vector3D::getRotation(this->getOrientation(), host.getOrientation());
		// host.receiveTorque(rot.n * rot.teta * tK);
		this->resetForce();
		this->resetTorque();
	}
};

template <typename Cell> class ElasticBody {
	friend class GenericConnectionBodyPlugin<Cell, ElasticConnection>;

	Cell *cell = nullptr;
	std::vector<ElasticConnection<Cell> *> cellConnections;
	num_t restRadius = Config::DEFAULT_CELL_RADIUS;  // radiius of the cell when at rest
	num_t youngModulus = Config::DEFAULT_CELL_YOUNGMOD;
	num_t poissonCoef = Config::DEFAULT_CELL_POISSONCOEF;

 public:
	using embedded_plugin_t = GenericConnectionBodyPlugin<Cell, ElasticConnection>;
	OrientedParticle realCenter;
	GhostCenter<OrientedParticle> ghostCenter;

	ElasticBody(Cell *c, Vector3D pos = Vector3D::zero())
	    : cell(c), realCenter(pos), ghostCenter(realCenter){};
	void setRestRadius(num_t r) { restRadius = r; }
	num_t getBoundingBoxRadius() const { return restRadius; };
	num_t getYoungModulus() const { return youngModulus; }
	num_t getPoissonCoef() const { return poissonCoef; }
	void setYoungModulus(num_t y) { youngModulus = y; }
	void setPoissonCoef(num_t p) { poissonCoef = p; }
	void setMass(num_t m) { realCenter.setMass(m); }
	Vector3D getVelocity() { return realCenter.getVelocity(); }
	Vector3D getForce() { return realCenter.getForce(); }
	Vector3D getTorque() { return realCenter.getTorque(); }
	num_t getMass() { return realCenter.getMass(); }
	num_t getMomentOfInertia() const {
		return 0.4 * realCenter.getMass() * restRadius * restRadius;
	}
	Vector3D getPosition() { return realCenter.getPosition(); }
	void receiveTorque(const Vec &t) { ghostCenter.receiveTorque(t); }
	void receiveForce(const Vec &f) { ghostCenter.receiveForce(f); }

	template <typename Integrator = Euler> void updatePositionsAndOrientations(num_t dt) {
		Integrator::updatePosition(realCenter, dt);
	}

	std::tuple<Cell *, num_t> getConnectedCellAndMembraneDistance(const Vec &d) const {
		// /!\ assumes that d is normalized
		Cell *closestCell = nullptr;
		num_t closestDist = restRadius;
		for (auto &con : cellConnections) {
			auto normal = cell == con->cells.first ? -con->direction : con->direction;
			num_t dot = normal.dot(d);
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
	void resetForce() { realCenter.resetForce(); }
	void resetTorque() { realCenter.resetTorque(); }

	inline Cell *getConnectedCell(const Vec &d) const {
		return get<0>(getConnectedCellAndMembraneDistance(d));
	}

	// required by GenericConnection Plugin
	inline num_t getPreciseMembraneDistance(const Vec &d) const {
		return get<1>(getConnectedCellAndMembraneDistance(d));
	}

	void moveTo(Vector3D newpos) {
		ghostCenter.setPosition(newpos);
		ghostCenter.setVelocity(Vec::zero());
	}

	void updateInternals(num_t dt) {
		auto moi = getMomentOfInertia();
		ghostCenter.update(dt, moi);
	}
};
}
#endif
