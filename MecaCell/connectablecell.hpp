#ifndef CONNECTABLECELL_HPP
#define CONNECTABLECELL_HPP
#include <vector>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include "rotation.h"
#include "movable.h"
#include "orientable.h"
#include "connection.h"

#define CUBICROOT2 1.25992104989
#define VOLUMEPI 0.23873241463 // 1/(4/3*pi)

using namespace std;

namespace MecaCell {
template <typename Derived> class ConnectableCell : public Movable, public Orientable {
protected:
	int typeId = 0;    // current cell type id. Used for determining adhesion
	bool dead = false; // is the cell dead or alive ?
	vector<double> color = {0.75, 0.12, 0.07};
	double radius = DEFAULT_CELL_RADIUS;
	double baseRadius = DEFAULT_CELL_RADIUS;
	double stiffness = DEFAULT_CELL_STIFFNESS;
	double dampRatio = DEFAULT_CELL_DAMP_RATIO;
	double angularStiffness = DEFAULT_CELL_ANG_STIFFNESS;
	bool tested = false; // has already been tested for collision
	vector<Connection<Derived> *> connections;
	vector<Derived *> connectedCells;

public:
	ConnectableCell(Vec pos) : Movable(pos) { randomColor(); }

	ConnectableCell(const Derived &c, const Vec &translation)
	    : Movable(c.getPosition() + translation, c.mass), radius(c.radius) {
		randomColor();
	}

	double getRadius() const { return radius; }
	double getBaseRadius() const { return baseRadius; }
	double getStiffness() const { return stiffness; }
	double getColor(unsigned int i) const {
		if (i < 3) return color[i];
		return 0;
	}
	const std::vector<Derived *> &getConnectedCells() const { return connectedCells; }
	double getPressure() const {
		double surface = 4.0f * M_PI * radius * radius;
		return totalForce / surface;
	}
	double getNormalizedPressure() const {
		double p = getPressure();
		double sign = p >= 0 ? 1 : -1;
		return 0.5 + sign * 0.5 * (1.0 - exp(-abs(14.0 * p)));
	}
	double getSqradius() const { return radius * radius; }
	bool alreadyTested() const { return tested; }
	int getNbConnections() const { return connections.size(); }

	/******************************
	 * main setters & getters
	 *****************************/

	void setRadius(double r) { radius = r; }
	void markAsTested() { tested = true; }
	void markAsNotTested() { tested = false; }
	double getBaseVolume() const { return (4.0 / 3.0) * M_PI * baseRadius * baseRadius * baseRadius; }
	double getVolume() const { return (4.0 / 3.0) * M_PI * radius * radius * radius; }
	double getRelativeVolume() const { return getVolume() / getBaseVolume(); }
	int getTypeId() const { return typeId; }

	// return the connection length with another cell
	// according to an adhesion coef (0 <= adh <= 1)
	double getConnectionLength(const Derived *c, const double adh) const {
		double l = radius + c->radius;
		return getConnectionLength(l, adh);
	}

	static double getConnectionLength(const double l, const double adh) {
		if (adh > ADH_THRESHOLD) return mix(MAX_CELL_ADH_LENGTH * l, MIN_CELL_ADH_LENGTH * l, adh);
		return l;
	}

	void setVolume(double v) { setRadius(cbrt(v / (4.0 * M_PI / 3.0))); }
	Derived *selfptr() { return static_cast<Derived *>(this); }
	Derived &self() { return static_cast<Derived &>(*this); }
	const Derived &selfconst() const { return static_cast<const Derived &>(*this); }

	// Don't forget to implement this method in the derived class
	double getAdhesionWith(const int tId) const { return selfconst().getAdhesionWith(tId); }

	vector<Connection<Derived> *> &getRWConnections() { return connections; }

	/******************************
	 * connections
	 *****************************/
	Connection<Derived> *connection(Derived *c) {
		if (c != this) {
			Vec AB = c->position - position;
			double sqdist = AB.sqlength();
			double sql = radius + c->radius;
			sql *= sql;
			if (sqdist <= sql) {
				// if those cells aren't already connected
				if (find(connectedCells.begin(), connectedCells.end(), c) == connectedCells.end()) {
					// first we check if this connection would not go through an already connected cell
					bool ok = true;
					Vec ABn = AB.normalized();
					for (auto c1It = connections.begin(); c1It != connections.end(); ++c1It) {
						auto c1 = *c1It;
						Vec c1Vec;
						double r1;
						double c1L = c1->getLength();
						if (c1->getNode0() == this) {
							c1Vec = c1->getDirection() * c1L;
							r1 = c1->getNode1()->getRadius();
						} else {
							c1Vec = -c1->getDirection() * c1L;
							r1 = c1->getNode0()->getRadius();
						}
						double scal = c1Vec.dot(ABn);
						c1L *= c1L;
						if (scal > 0 && c1L < sqdist && c1L - scal * scal < r1 * r1) {
							ok = false;
							break;
						}
					}
					if (ok) {
						double adh0 = getAdhesionWith(c->typeId);
						double adh1 = c->getAdhesionWith(typeId);
						double minAdh = (adh0 + adh1) * 0.5;
						double l = getConnectionLength(c, minAdh);
						double k = (stiffness * radius + c->stiffness * c->radius) / (radius + c->radius);
						double dr = (dampRatio * radius + c->dampRatio * c->radius) / (radius + c->radius);
						double maxTeta = mix(0.0, M_PI, minAdh);
						Connection<Derived> *s = new Connection<Derived>(
						    pair<Derived *, Derived *>(selfptr(), c),
						    Spring(k, dampingFromRatio(dr, mass + c->mass, k), l),
						    make_pair(Joint(getAngularStiffness(),
						                    dampingFromRatio(dr, getMomentOfInertia() * 2.0, angularStiffness), maxTeta),
						              Joint(getAngularStiffness(),
						                    dampingFromRatio(dr, c->getMomentOfInertia() * 2.0, c->angularStiffness),
						                    maxTeta)),
						    make_pair(Joint(getAngularStiffness(),
						                    dampingFromRatio(dr, getMomentOfInertia() * 2.0, angularStiffness), maxTeta),
						              Joint(getAngularStiffness(),
						                    dampingFromRatio(dr, c->getMomentOfInertia() * 2.0, c->angularStiffness),
						                    maxTeta)));
						addConnection(c, s);
						return s;
					}
				}
			}
		}
		return NULL;
	}

	double getMomentOfInertia() const { return 4.0 * mass * radius * radius; }
	double getAngularStiffness() const { return angularStiffness; }

	void updateAllConnections() {
		// for (unsigned int i = 0; i < connections.size(); i++) {
		// Connection<Derived>* co = connections[i];
		// int otherId = co->getNode(0)->getParent() == this ? 1 : 0;
		// Cell* otherCell = co->getNode(otherId)->getParent();
		// double adh1 = 0;
		// double adh0 = 0;
		// double otherRadius = 0;
		// if (otherCell != nullptr) {
		// adh1 = otherCell->getCurrentState().getAdhesionWith(getCurrentState().getId());
		// otherRadius = otherCell->getRadius();
		// adh0 = getCurrentState().getAdhesionWith(otherCell->getCurrentState().getId());
		//} else {
		// adh1 = getCurrentState().getWallAdhesion();
		// adh0 = adh1;
		//}
		// double minAdh = adh0 < adh1 ? adh0 : adh1;
		// co->ls.l = getConnectionLength(getRadius() + otherRadius, minAdh);
		//}
	}

	Derived *divide() { return divide(Vec::randomUnit()); }

	Derived *divide(const Vec &direction) {
		setRadius(getBaseRadius());
		setMass(getBaseMass());
		updateAllConnections();
		Derived *newC = new Derived(selfconst(), direction.normalized() * radius * 0.8);
		return newC;
	}

	void grow(double qtty) {
		double rv = getRelativeVolume() + qtty;
		setVolume(getBaseVolume() * rv);
		setMass(getBaseMass() * rv);
	}

	void addConnection(Derived *c, Connection<Derived> *s) {
		connections.push_back(s);
		c->connections.push_back(s);
		connectedCells.push_back(c);
		c->connectedCells.push_back(selfptr());
	}

	void eraseCell(Derived *cell) {
		unsigned int prevC = connectedCells.size();
		connectedCells.erase(remove(connectedCells.begin(), connectedCells.end(), cell), connectedCells.end());
		assert(connectedCells.size() == prevC - 1 || prevC == 0);
	}

	void deleteConnection(Derived *c) {
		Connection<Derived> *s = nullptr;
		for (unsigned int i = 0; i < connections.size(); i++) {
			Connection<Derived> *co = connections[i];
			if ((co->getNode1() == this && co->getNode0() == c) ||
			    (co->getNode1() == c && co->getNode0() == this)) {
				s = co;
				break;
			}
		}
		deleteConnection(c, s);
	}

	void deleteConnection(Derived *c, Connection<Derived> *s) {
		assert(c != nullptr);
		eraseCell(c);
		c->eraseCell(selfptr());
		eraseConnection(s);
		c->eraseConnection(s);
	}

	void eraseConnection(Connection<Derived> *s) {
		connections.erase(remove(connections.begin(), connections.end(), s), connections.end());
	}

	void eraseAndDeleteAllConnections(vector<Connection<Derived> *> &aux) {
		for (auto cIt = connections.begin(); cIt != connections.end();) {
			Connection<Derived> *sp = *cIt;
			auto otherCell = sp->getNode0() == this ? sp->getNode1() : sp->getNode0();
			if (otherCell != nullptr) {
				aux.erase(remove(aux.begin(), aux.end(), sp), aux.end());
				connectedCells.erase(remove(connectedCells.begin(), connectedCells.end(), otherCell),
				                     connectedCells.end());
				otherCell->connectedCells.erase(
				    remove(otherCell->connectedCells.begin(), otherCell->connectedCells.end(), this),
				    otherCell->connectedCells.end());
				otherCell->eraseConnection(sp);
				cIt = connections.erase(cIt);
				delete sp;
			} else {
				++cIt;
			}
		}
	}

	/******************************
	 * division and control
	 *****************************/

	Derived *updateBehavior(double dt) { return self().updateBehavior(dt); }

	void die() { dead = true; }
	bool isDead() { return dead; }

	void randomColor() {
		double r0 = 0.001 * (rand() % 1000);
		double r1 = 0.001 * (rand() % 1000);
		if (r0 < 0.5) {
			color[1] = 0.1 * r1;
			color[2] = 0.65 + (0.2 * r0);
			color[0] = 0.8 + (0.2 * r1);
		} else {
			color[0] = 0.1 * r1;
			color[1] = 0.65 + (0.2 * r0);
			color[2] = 0.8 + (0.2 * r1);
		}
	}
};
}

#endif
