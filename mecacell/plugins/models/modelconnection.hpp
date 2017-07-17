#ifndef MECACELL_MODELCONNECTION_H
#define MECACELL_MODELCONNECTION_H
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <vector>
#include "connection.h"
#include "matrix4x4.h"
#include "model.h"
#include "objmodel.h"
#include "utils.hpp"

using std::string;
using std::vector;
using std::unordered_map;
using std::unordered_set;
using std::pair;

namespace MecaCell {

struct SpaceConnectionPoint {  // just a connection point with anything anywhere
	SpaceConnectionPoint(Vec p) : position(p) {}
	Vec position;
	size_t face;
	void setPosition(const Vec &){};
	void setVelocity(const Vec &){};
	Vec getPosition() { return position; }
	Vec getVelocity() { return Vec::zero(); }
	Vec getAngularVelocity() { return Vec::zero(); }
	Basis<Vec> getOrientation() { return Basis<Vec>(); }
	Rotation<Vec> getOrientationRotation() { return Rotation<Vec>(); }
	double getInertia() { return 1; }
	void receiveForce(double, const Vec &, bool) {}
	void receiveForce(const Vec &) {}
	void receiveTorque(const Vec &) {}
};

struct ModelConnectionPoint {
	ModelConnectionPoint(Model *m, Vec p, size_t f) : model(m), position(p), face(f) {}
	Model *model;
	Vec position;
	size_t face;
	// TODO : toggle movable / orientable in connection
	void setPosition(const Vec &){};
	void setVelocity(const Vec &){};
	Vec getPosition() { return position; }
	Vec getVelocity() { return Vec::zero(); }
	Vec getAngularVelocity() { return Vec::zero(); }
	Basis<Vec> getOrientation() { return Basis<Vec>(); }
	Rotation<Vec> getOrientationRotation() { return Rotation<Vec>(); }
	double getInertia() { return 1; }
	void receiveForce(double, const Vec &, bool) {}
	void receiveForce(const Vec &) {}
	void receiveTorque(const Vec &) {}
};

template <typename Cell> struct CellModelConnection {
	using CMConnection = Connection<ModelConnectionPoint, Cell *>;
	using CSConnection = Connection<SpaceConnectionPoint, Cell *>;
	Model *model;
	CSConnection anchor;    // slide and anchor, only angular
	CMConnection bounce;    // always perpendicular, only classic spring
	double maxTeta = 0.1;  // this is for the anchor, and should always be smaller than the
	                        // actual connection's maxTeta
	void computeForces(double dt) {
		anchor.computeForces(dt);
		bounce.computeForces(dt);
	}
	CellModelConnection() {}
	CellModelConnection(CSConnection a, CMConnection b) : anchor(a), bounce(b) {}
	bool dirty = false;  // does this connection need to be deleted?
};
}
#endif
