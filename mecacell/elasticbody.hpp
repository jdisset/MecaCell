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
	double fK = 3.0;
	double tK = 1.0;
	double prevDist = 0;
	GhostCenter(H &h) : OrientedParticle(h.getPosition()), host(h) {
		this->mass = host.getMass();
		this->position = h.getPosition();
		this->orientation = h.getOrientation();
		this->orientationRotation = h.getOrientationRotation();
	}
	template <typename Integrator = Euler> void update(double dt, double momentOfInertia) {
		this->mass = host.getMass();
		logger<DBG>("GhostCenter force = ", this->force);
		Integrator::updatePosition(*this, dt);
		Integrator::updateOrientation(*this, momentOfInertia, dt);
		auto dir = this->position - host.getPosition();
		double l = dir.length();
		if (l > 0) {
			dir = dir / l;
			double f = std::min(1.0 / dt, l * fK);
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
	double restRadius = Config::DEFAULT_CELL_RADIUS;  // radiius of the cell when at rest
	double youngModulus = Config::DEFAULT_CELL_YOUNGMOD;
	double poissonCoef = Config::DEFAULT_CELL_POISSONCOEF;

 public:
	using embedded_plugin_t = GenericConnectionBodyPlugin<Cell, ElasticConnection>;
	OrientedParticle realCenter;
	GhostCenter<OrientedParticle> ghostCenter;

	ElasticBody(Cell *c, Vector3D pos = Vector3D::zero())
	    : cell(c), realCenter(pos), ghostCenter(realCenter){};
	void setRestRadius(double r) { restRadius = r; }
	double getBoundingBoxRadius() const { return restRadius; };
	double getYoungModulus() const { return youngModulus; }
	double getPoissonCoef() const { return poissonCoef; }
	void setYoungModulus(double y) { youngModulus = y; }
	void setPoissonCoef(double p) { poissonCoef = p; }
	void setMass(double m) { realCenter.setMass(m); }
	Vector3D getVelocity() { return realCenter.getVelocity(); }
	Vector3D getForce() { return realCenter.getForce(); }
	Vector3D getTorque() { return realCenter.getTorque(); }
	double getMass() { return realCenter.getMass(); }
	double getMomentOfInertia() const {
		return 0.4 * realCenter.getMass() * restRadius * restRadius;
	}
	Vector3D getPosition() { return realCenter.getPosition(); }
	void receiveTorque(const Vec &t) { ghostCenter.receiveTorque(t); }
	void receiveForce(const Vec &f) { ghostCenter.receiveForce(f); }

	template <typename Integrator = Euler> void updatePositionsAndOrientations(double dt) {
		Integrator::updatePosition(realCenter, dt);
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
	void resetForce() { realCenter.resetForce(); }
	void resetTorque() { realCenter.resetTorque(); }

	inline Cell *getConnectedCell(const Vec &d) const {
		return get<0>(getConnectedCellAndMembraneDistance(d));
	}

	// required by GenericConnection Plugin
	inline double getPreciseMembraneDistance(const Vec &d) const {
		return get<1>(getConnectedCellAndMembraneDistance(d));
	}

	void moveTo(Vector3D newpos) {
		ghostCenter.setPosition(newpos);
		ghostCenter.setVelocity(Vec::zero());
	}

	void updateInternals(double dt) {
		auto moi = getMomentOfInertia();
		ghostCenter.update(dt, moi);
	}
};
}
#endif
