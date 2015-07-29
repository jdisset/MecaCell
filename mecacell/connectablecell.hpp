#ifndef CONNECTABLECELL_HPP
#define CONNECTABLECELL_HPP
#include <vector>
#include <deque>
#include <array>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include "rotation.h"
#include "movable.h"
#include "orientable.h"
#include "connection.h"
#include "model.h"

#define CUBICROOT2 1.25992104989
#define VOLUMEPI 0.23873241463 // 1/(4/3*pi)

using namespace std;

namespace MecaCell {
template <typename Derived> class ConnectableCell : public Movable, public Orientable {
protected:
	using ConnectionType = Connection<Derived *>;
	using ModelConnectionType =
	    pair<Connection<ModelConnectionPoint, Derived *>, Connection<ModelConnectionPoint, Derived *>>;
	bool dead = false; // is the cell dead or alive ?
	array<double, 3> color = {{0.75, 0.12, 0.07}};
	double radius = DEFAULT_CELL_RADIUS;
	double baseRadius = DEFAULT_CELL_RADIUS;
	double stiffness = DEFAULT_CELL_STIFFNESS;
	double dampRatio = DEFAULT_CELL_DAMP_RATIO;
	double angularStiffness = DEFAULT_CELL_ANG_STIFFNESS;
	bool tested = false; // has already been tested for collision
	vector<ConnectionType *> connections;
	vector<ModelConnectionType *> modelConnections;
	vector<Derived *> connectedCells;

public:
	ConnectableCell(Vec pos) : Movable(pos) { randomColor(); }

	ConnectableCell(const Derived &c, const Vec &translation)
	    : Movable(c.getPosition() + translation, c.mass),
	      dead(false),
	      color(c.color),
	      radius(c.radius),
	      baseRadius(c.baseRadius),
	      stiffness(c.stiffness),
	      dampRatio(c.dampRatio),
	      angularStiffness(c.angularStiffness),
	      tested(false) {}

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

	void setBaseRadius(double r) { baseRadius = r; }
	void setStiffness(double s) { stiffness = s; }
	void setAngularStiffness(double s) { angularStiffness = s; }
	void setRadius(double r) { radius = r; }
	void markAsTested() { tested = true; }
	void markAsNotTested() { tested = false; }
	double getBaseVolume() const { return (4.0 / 3.0) * M_PI * baseRadius * baseRadius * baseRadius; }
	double getVolume() const { return (4.0 / 3.0) * M_PI * radius * radius * radius; }
	double getRelativeVolume() const { return getVolume() / getBaseVolume(); }
	double getDampRatio() const { return dampRatio; }

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
	double getAdhesionWith(const Derived *d) { return self().getAdhesionWith(d); }
	double getAdhesionWithModel(const string &) { return 0.7; }

	vector<ConnectionType *> &getRWConnections() { return connections; }
	vector<ModelConnectionType *> &getRWModelConnections() { return modelConnections; }

	// TODO : try using set instead of vectors for connections (faster random deletion)
	void addModelConnection(ModelConnectionType *con) { modelConnections.push_back(con); }
	void removeModelConnection(ModelConnectionType *con) {
		modelConnections.erase(remove(modelConnections.begin(), modelConnections.end(), con),
		                       modelConnections.end());
	}

	/******************************
	 * connections
	 *****************************/
	void connection(Derived *c, vector<ConnectionType *> &worldConnexions) {
		if (c != this) {
			Vec AB = c->position - position;
			double sqdist = AB.sqlength();
			double sql = radius + c->radius;
			sql *= sql;
			// interpenetration
			if (sqdist <= sql) {
				if (find(connectedCells.begin(), connectedCells.end(), c) == connectedCells.end()) {
					// if those cells aren't already connected
					// we check if this connection would not go through an already connected cell
					bool ok = true;
					for (auto &con : connections) {
						Derived *otherCell = con->getNode0() == selfptr() ? con->getNode1() : con->getNode0();
						Vec AO = otherCell->getPosition() - position;
						double AOdotAB = AO.dot(AB);
						if (AOdotAB > 0) {
							// Other cell's projection onto AB
							Vec AP = AB * AOdotAB / sqdist;
							if (AP.dot(AB) < sqdist) {
								// the other cell's projection is closer than our candidate
								if ((AP - AO).sqlength() < 0.92 * pow(otherCell->getRadius(), 2)) {
									ok = false;
									break;
								}
							}
						}
					}
					if (ok) {
						// TODO: store adhesion with each connection. Each call to getAdhesionWith should only be for
						// a new connection. For the old connection, user should be able to tweak the coefficien through a
						// updateConnectionParams(ConnectionType*) method.
						double minAdh = (getAdhesionWith(c) + c->getAdhesionWith(selfptr())) * 0.5;
						double l = getConnectionLength(c, minAdh);
						double k = (stiffness * radius + c->stiffness * c->radius) / (radius + c->radius);
						double dr = (dampRatio * radius + c->dampRatio * c->radius) / (radius + c->radius);
						// double maxTeta = mix(0.0, M_PI / 2.0, minAdh);
						double maxTeta = M_PI / 12.0;
						ConnectionType *s = new ConnectionType(
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
						double contactSurface = M_PI * (sqdist + pow((radius + c->radius) / 2, 2));
						s->getFlex().first.setCurrentKCoef(contactSurface);
						s->getFlex().second.setCurrentKCoef(contactSurface);
						s->getTorsion().first.setCurrentKCoef(contactSurface);
						s->getTorsion().second.setCurrentKCoef(contactSurface);
						addConnection(c, s);

						worldConnexions.push_back(s);
					}
				}
			}
		}
	}

	double getMomentOfInertia() const { return 4.0 * mass * radius * radius; }
	double getAngularStiffness() const { return angularStiffness; }

	// recomputes all connections sizes according to the the current size of the cell
	// TODO : also change the stiffness / strength of a connection according to the adhesion area
	//  (maybe directly from World?)
	void updateAllConnections() {
		for (auto &con : connections) {
			Derived *otherCell = con->getNode0() == selfptr() ? con->getNode1() : con->getNode0();
			double adhCoef = (getAdhesionWith(otherCell) + otherCell->getAdhesionWith(selfptr())) * 0.5;
			con->setBaseLength(getConnectionLength(otherCell, adhCoef));
		}
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
		updateAllConnections();
	}

	void addConnection(Derived *c, ConnectionType *s) {
		connections.push_back(s);
		c->connections.push_back(s);
		connectedCells.push_back(c);
		c->connectedCells.push_back(selfptr());
	}

	// erase cell from the connectedCells container
	void eraseCell(Derived *cell) {
		unsigned int prevC = connectedCells.size();
		connectedCells.erase(remove(connectedCells.begin(), connectedCells.end(), cell), connectedCells.end());
		assert(connectedCells.size() == prevC - 1 || prevC == 0);
	}

	// erase connection with a cell (calls deleteConnection(c,s))
	void removeConnection(Derived *c) {
		ConnectionType *s = nullptr;
		for (unsigned int i = 0; i < connections.size(); i++) {
			ConnectionType *co = connections[i];
			if ((co->getNode1() == this && co->getNode0() == c) ||
			    (co->getNode1() == c && co->getNode0() == this)) {
				s = co;
				break;
			}
		}
		if (s) removeConnection(c, s);
	}

	// erase connection S with cell C from the connections and connectedCells containers
	// destructors are not called
	void removeConnection(Derived *c, ConnectionType *s) {
		assert(c);
		eraseCell(c);
		c->eraseCell(selfptr());
		eraseConnection(s);
		c->eraseConnection(s);
	}

	// erase connection s f the connections container
	void eraseConnection(ConnectionType *s) {
		connections.erase(remove(connections.begin(), connections.end(), s), connections.end());
	}

	void eraseAndDeleteAllConnections(std::vector<ConnectionType *> &aux) {
		for (auto cIt = connections.begin(); cIt != connections.end();) {
			ConnectionType *sp = *cIt;
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
		double r0 = 0.0001 * (rand() % 10000);
		double r1 = 0.0001 * (rand() % 10000);
		if (false) {
			if (r0 < 1.0 / 3.0) {
				/// green
				color[0] = 0.1 + (0.4 * r1);
				color[1] = 0.78 + (0.2 * r0); // + (0.1 * r0);
				color[2] = 0.06 + 0.6 * r0;
			} else if (r0 < 2.0 / 3.0) {
				// yellow
				color[0] = 0.9 + (0.1 * r1);
				color[1] = 0.73; // + (0.1 * r0);
				color[2] = 0.36 + 0.2 * r0;
			} else {
				// blue
				color[0] = 0.05 * r1;
				color[1] = 0.6 + (0.1 * r1);
				color[2] = 0.7 + (0.2 * r0);
			}
		}
		color[0] = 0.6 + (0.3 * r1);
		color[1] = 0.3 * r1;
		color[2] = 0.05 + (0.2 * r0);
	}
};
}

#endif
