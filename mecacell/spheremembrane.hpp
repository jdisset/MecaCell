#ifndef SPHEREMEMBRANE_HPP
#define SPHEREMEMBRANE_HPP
#include "tools.h"
#include "connection.h"
#include "model.h"
#include "modelconnection.hpp"
#include <unordered_map>
#include <utility>
#include <vector>

// default breaking connection angle
#define DEFAULT_MAX_TETA M_PI / 12.0
// threshold (dot product) above which we want two model connections to be merged
#define MIN_MODEL_CONNECTION_SIMILARITY 0.8

namespace MecaCell {
template <typename Cell> class SphereMembrane {
	/********************** SphereMembrane class template **********************/
	// Abstract :
	// A sphere membrane is a very crude membrane approximation where a cell is
	// only defined by its radius. It is meant to be fast while allowing cells to
	// dynamically connect and bounce. It provides rudimentary (but fast)
	// volume conservation approximations and tries to maintain an adhesive
	// strength proportional to the contact surface.

 protected:
	using ConnectionType = Connection<Cell *>;
	using ModelConnectionType = CellModelConnection<Cell>;

 public:
	using CellCellConnectionContainer = unordered_map<ordered_pair<Cell *>, ConnectionType>;
	using CellModelConnectionContainer =
	    unordered_map<Model *,
	                  unordered_map<Cell *, vector<unique_ptr<CellModelConnection<Cell>>>>>;

 protected:
	Cell *cell;
	unordered_map<Cell *, ConnectionType *> cellConnections;
	vector<ModelConnectionType *> modelConnections;
	float_t baseRadius = DEFAULT_CELL_RADIUS;
	float_t radius = DEFAULT_CELL_RADIUS;
	float_t correctedRadius =
	    DEFAULT_CELL_RADIUS;  // taking volume conservation into account
	float_t stiffness = DEFAULT_CELL_STIFFNESS;
	float_t dampRatio = DEFAULT_CELL_DAMP_RATIO;
	float_t angularStiffness = DEFAULT_CELL_ANG_STIFFNESS;
	float_t maxTeta = M_PI / 12.0;
	float_t pressure = 0;
	bool volumeConservation = true;

 public:
	SphereMembrane(Cell *c) : cell(c){};
	SphereMembrane(Cell *c, const SphereMembrane &sm)
	    : cell(c),
	      baseRadius(sm.baseRadius),
	      radius(sm.radius),
	      correctedRadius(sm.radius),
	      stiffness(sm.stiffness),
	      dampRatio(sm.dampRatio),
	      angularStiffness(sm.angularStiffness),
	      maxTeta(sm.maxTeta){};

	/**********************************************************
	                             GET
	***********************************************************/
	/************************ basics **************************/
	inline float_t getBoundingBoxRadius() const { return correctedRadius; };
	inline float_t getStiffness() const { return stiffness; }
	inline float_t getRadius() const { return radius; }
	inline float_t getBaseRadius() const { return baseRadius; }
	inline float_t getCorrectedRadius() const { return correctedRadius; }
	inline float_t getPressure() const { return pressure; }
	inline float_t getSqradius() const { return radius * radius; }
	inline float_t getDampRatio() const { return dampRatio; }
	inline float_t getAngularStiffness() const { return angularStiffness; }
	decltype(cellConnections) &getRWCellConnections() { return cellConnections; }
	decltype(modelConnections) &getRWModelConnections() { return modelConnections; }
	/************************ computed **************************/
	float_t getPreciseMembraneDistance(const Vec &d) const {
		// /!\ assumes that d is normalized
		float_t closestDist = radius;
		for (auto &con : cellConnections) {
			Vec normal =
			    cell < con.first ? con.second->getDirection() : -con.second->getDirection();
			float_t dot = normal.dot(d);
			if (dot < 0) {
				float_t midpoint =
				    con.second->getLength() * radius / (radius + con.first->membrane.radius);
				float_t l = -normal.dot(normal * midpoint) / dot;
				if (l < closestDist) closestDist = l;
			}
		}
		return closestDist;
	}

	inline float_t getVolume() const {
		return (4.0 / 3.0) * M_PI * radius * radius * radius;
	}
	inline float_t getBaseVolume() const {
		return (4.0 / 3.0) * M_PI * baseRadius * baseRadius * baseRadius;
	}
	inline float_t getMomentOfInertia() const { return 4.0 * cell->mass * radius * radius; }
	double getCurrentActualVolume() {
		double targetVol =
		    (4.0 / 3.0) * M_PI * correctedRadius * correctedRadius * correctedRadius;
		double volumeLoss = 0;
		for (auto &c : cellConnections) {
			Cell *other = c.first;
			double midpoint =
			    c.second->getLength() * radius / (radius + other->membrane.radius);
			double h = getCorrectedRadius() - midpoint;
			volumeLoss +=
			    (M_PI * h / 6.0) *
			    (3.0 * (getCorrectedRadius() * getCorrectedRadius() - midpoint * midpoint) +
			     h * h);
		}
		return targetVol - volumeLoss;
	}
	static inline float_t getConnectionLength(const Cell *c0, const Cell *c1) {
		return getConnectionLength(
		    c0->membrane.correctedRadius + c1->membrane.correctedRadius,
		    min(c0->getAdhesionWith(c1), c1->getAdhesionWith(c0)));
	}
	static float_t getConnectionLength(const float_t l, const float_t adh) {
		if (adh > ADH_THRESHOLD)
			return mix(MAX_CELL_ADH_LENGTH * l, MIN_CELL_ADH_LENGTH * l, adh);
		return l;
	}

	/**********************************************************
	                             SET
	***********************************************************/
	void setRadius(float_t r) { radius = r; }
	void setVolume(float_t v) { setRadius(cbrt(v / (4.0 * M_PI / 3.0))); }
	void setStiffness(float_t s) { stiffness = s; }
	void setAngularStiffness(float_t s) { angularStiffness = s; }

	/**********************************************************
	                           UPDATE
	***********************************************************/
	template <typename Integrator> void updatePositionsAndOrientations(double dt) {
		// here we just have to move the cell center
		Integrator::updatePosition(*cell, dt);
		Integrator::updateOrientation(*cell, dt);
		if (volumeConservation) compensateVolumeLoss();
		cell->markAsNotTested();
	}

	void computePressure() {
		float_t surface = 4.0 * M_PI * radius * radius;
		pressure = cell->totalForce / surface;
	}

	double compensateVolumeLoss() {
		// just updates the correctedRadius
		double targetVol = getVolume();
		double volumeLoss = 0;
		for (auto &co : cellConnections) {
			auto &c = co.second;
			double midpoint = c->getLength() * radius / (radius + co.first->membrane.radius);
			double h = radius - midpoint;
			volumeLoss +=
			    (M_PI * h / 6.0) * (3.0 * (radius * radius - midpoint * midpoint) + h * h);
		}
		correctedRadius = cbrt((targetVol + volumeLoss) / ((4.0 / 3.0) * M_PI));
	}
	void resetForces() {
		cell->force = Vec::zero();
		cell->torque = Vec::zero();
	}
	void updateStats() { computePressure(); }

	/**********************************************************
	                       CONNECTIONS
	***********************************************************/
	/******************** with 3D models **********************/
	void addModelConnection(ModelConnectionType *con) { modelConnections.push_back(con); }
	void removeModelConnection(ModelConnectionType *con) {
		modelConnections.erase(remove(modelConnections.begin(), modelConnections.end(), con),
		                       modelConnections.end());
	}

	template <typename SpacePartition>
	static void checkForCellModelCollisions(
	    vector<Cell *> &cells, unordered_map<string, Model>,
	    CellModelConnectionContainer &cellModelConnections, SpacePartition &modelGrid) {
		for (auto &m : cellModelConnections) {
			for (auto &c : m.second) {
				for (auto &conn : c.second) {
					conn->dirty = true;
				}
			}
		}
		for (auto &c : cells) {
			// for each cell, we find if a cell - model collision is possible.
			auto toTest = modelGrid.retrieve(c->getPosition(), c->getBoundingBoxRadius());
			for (const auto &mf : toTest) {
				// for each pair <model*, faceId> mf potentially colliding with c
				const Vec &p0 = mf.first->vertices[mf.first->faces[mf.second].indices[0]];
				const Vec &p1 = mf.first->vertices[mf.first->faces[mf.second].indices[1]];
				const Vec &p2 = mf.first->vertices[mf.first->faces[mf.second].indices[2]];
				// checking if cell c is in contact with triangle p0, p1, p2
				pair<bool, Vec> projec = projectionIntriangle(p0, p1, p2, c->getPosition());
				// projec = {projection inside triangle, projection coordinates}
				// TODO: we also need to check if the connection should be on a vertice

				Vec currentDirection = projec.second - c->getPosition();
				if (projec.first &&
				    currentDirection.sqlength() < pow(c->getBoundingBoxRadius(), 2)) {
					// we have a potential connection. Now we consider 2 cases:
					// 1 - brand new connection (easy)
					// 2 - older connection	(we need to update it)
					//  => same cell/model pair + similar bounce angle (same face or similar normal)
					currentDirection.normalize();
					bool alreadyExist = false;
					if (cellModelConnections.count(mf.first) &&
					    cellModelConnections[mf.first].count(c)) {
						for (auto &otherconn : cellModelConnections[mf.first][c]) {
							Vec prevDirection =
							    (otherconn->bounce.getNode0().getPosition() - c->getPrevposition())
							        .normalized();
							if (prevDirection.dot(currentDirection) > MIN_MODEL_CONNECTION_SIMILARITY) {
								alreadyExist = true;
								otherconn->dirty = false;
								// case n° 2, we want to update otherconn
								// first, the bounce spring
								otherconn->bounce.getNode0().position = projec.second;
								otherconn->bounce.getNode0().face = mf.second;
								// then the anchor. It's just another simple spring that is always at the
								// same height as the cell (orthogonal to the bounce spring)
								// it has a restlength of 0 and follows the cell when its length is more
								// than the cell's radius;
								if (otherconn->anchor.getSc().length > 0) {
									// first we keep the anchor at cell height
									const Vec &anchorDirection = otherconn->anchor.getSc().direction;
									Vec crossp =
									    currentDirection.cross(currentDirection.cross(anchorDirection));
									if (crossp.sqlength() > c->membrane.radius * 0.02) {
										crossp.normalize();
										float_t projLength = min(
										    (otherconn->anchor.getNode0().getPosition() - c->getPosition())
										        .dot(crossp),
										    c->membrane.radius);
										otherconn->anchor.getNode0().position =
										    c->getPosition() + projLength * crossp;
									}
								}
								break;
							}
						}
					}
					if (!alreadyExist) {
						// new connection
						float_t adh = c->getAdhesionWithModel(mf.first->name);
						float_t l = mix(MAX_CELL_ADH_LENGTH * c->membrane.correctedRadius,
						                MIN_CELL_ADH_LENGTH * c->membrane.correctedRadius, adh);
						unique_ptr<CellModelConnection<Cell>> cmc(new CellModelConnection<Cell>(
						    Connection<SpaceConnectionPoint, Cell *>(
						        {SpaceConnectionPoint(c->getPosition()), c},  // N0, N1
						        Spring(100, dampingFromRatio(0.9, c->getMass(), 100),
						               0)),  // anchor
						    Connection<ModelConnectionPoint, Cell *>(
						        {ModelConnectionPoint(mf.first, projec.second, mf.second),
						         c},  // N0, N1
						        Spring(c->membrane.getStiffness(),
						               dampingFromRatio(c->membrane.getDampRatio(), c->getMass(),
						                                c->membrane.getStiffness() * 1.0),
						               l)  // bounce
						        )));
						cmc->anchor.tjEnabled = false;
						c->membrane.addModelConnection(cmc.get());
						cellModelConnections[mf.first][c].push_back(move(cmc));
					}
				}
			}
		}
		// clean up: dirty connections
		for (auto &m : cellModelConnections) {
			for (auto &c : m.second) {
				for (auto it = c.second.begin(); it != c.second.end();) {
					if ((*it)->dirty) {
						c.first->membrane.removeModelConnection(it->get());
						it = c.second.erase(it);
					} else {
						++it;
					}
				}
			}
		}
		// clean up: empty entries
		for (auto itM = cellModelConnections.begin(); itM != cellModelConnections.end();) {
			if (itM->second.empty()) {
				itM = cellModelConnections.erase(itM);
			} else {
				for (auto itC = itM->second.begin(); itC != itM->second.end();) {
					if (itC->second.empty()) {
						itC = itM->second.erase(itC);
					} else {
						++itC;
					}
				}
				++itM;
			}
		}
	}

	/******************** between cells ***********************/
	template <typename SpacePartition>
	static void checkForCellCellConnections(
	    vector<Cell *> &cells, CellCellConnectionContainer &cellCellConnections,
	    SpacePartition &grid) {
		vector<ordered_pair<Cell *>> newConnections;
		grid.clear();
		for (const auto &c : cells) grid.insert(c);
		auto gridCells = grid.getThreadSafeGrid();
		for (auto &batch : gridCells) {
			for (int i = 0; i < batch.size(); ++i) {
				for (int j = 0; j < batch[i].size(); ++j) {
					auto &c0 = batch[i][j];
					for (int k = j + 1; k < batch[i].size(); ++k) {
						auto &c1 = batch[i][k];
						Vec AB = c1->position - c0->position;
						float_t sqDistance = AB.sqlength();
						float_t sqMaxLength = pow(c0->membrane.radius + c1->membrane.radius, 2);
						if (sqDistance <= sqMaxLength) {
							if (!c0->connectedCells.count(c1) && c1 != c0) {
								float_t dist = sqrt(sqDistance);
								Vec dir = AB / dist;
								if (dist < c0->membrane.getPreciseMembraneDistance(dir) +
								               c1->membrane.getPreciseMembraneDistance(-dir)) {
									newConnections.push_back(make_ordered_pair(c0, c1));
								}
							}
						}
					}
				}
			}
		}
		for (auto &nc : newConnections) {
			createConnection(nc, cellCellConnections);
		}
	}

	static void updateCellModelConnections(CellModelConnectionContainer &con, float_t dt) {
		for (auto &mod : con) {
			for (auto &vec : mod.second) {
				for (auto &c : vec.second) {
					c->computeForces(dt);
				}
			}
		}
	}

	inline float_t getConnectionMidpoint(const Cell *other,
	                                     const ConnectionType *connection) {
		return connection->getLength() * radius / (radius + other->membrane.radius);
	}

	static void updateCellCellConnections(CellCellConnectionContainer &con, float_t dt) {
		// we update forces & delete impossible connections (cells not in contact anymore...)
		// here connections are just beams between cells centers
		vector<ordered_pair<Cell *>> toErase;
		for (auto it = con.begin(); it != con.end(); ++it) {
			auto &connection = (*it).second;  // the connection
			auto &c0 = (*it).first.first;     // cell
			auto &c1 = (*it).first.second;    // cell
			// we check if the connection still makes sense
			if (connection.getSc().length >
			    0.00000000001 + c0->getMembraneDistance(connection.getDirection()) +
			        c1->getMembraneDistance(-connection.getDirection())) {
				toErase.push_back((*it).first);
			} else {
				// connection still ok, we update its parameters
				// according to contact surface and cells volumes
				float_t contactSurface =
				    M_PI * (pow(connection.getSc().length, 2) +
				            pow((c0->membrane.radius + c1->membrane.radius) / 2.0, 2));
				connection.getFlex().first.setCurrentKCoef(contactSurface);
				connection.getFlex().second.setCurrentKCoef(contactSurface);
				connection.getTorsion().first.setCurrentKCoef(contactSurface);
				connection.getTorsion().second.setCurrentKCoef(contactSurface);
				float_t newl = getConnectionLength(
				    c0->membrane.correctedRadius + c1->membrane.correctedRadius,
				    (c0->getAdhesionWith(c1) + c1->getAdhesionWith(c0)) * 0.5);
				connection.getSc().setRestLength(newl);
				// then we compute the forces
				connection.computeForces(dt);
			}
		}
		for (auto &p : toErase) {
			disconnect(p.first, p.second);
			con.erase(p);
		}
	}

	static inline void disconnect(Cell *c0, Cell *c1) {
		c0->membrane.cellConnections.erase(c1);
		c1->membrane.cellConnections.erase(c0);
		c0->connectedCells.erase(c1);
		c1->connectedCells.erase(c0);
	}

	static inline void disconnectAndDeleteAllConnections(Cell *c0,
	                                                     CellCellConnectionContainer &con) {
		// TODO: handle model connections
		auto cop = c0->connectedCells;
		for (auto &c1 : cop) {
			disconnect(c0, c1);
			auto p = make_ordered_pair(c0, c1);
			con.erase(p);
		}
	}

	static vector<ordered_pair<Cell *>> checkAndReturnNewConnection(
	    Cell *c0, Cell *c1, CellCellConnectionContainer &con) {}

	static void createConnection(ordered_pair<Cell *> cells,
	                             CellCellConnectionContainer &con) {
		float_t l = getConnectionLength(cells.first, cells.second);
		auto &membrane0 = cells.first->membrane;
		auto &membrane1 = cells.second->membrane;
		float_t vol0 = membrane0.getVolume();
		float_t vol1 = membrane1.getVolume();
		float_t volProportion = vol0 + vol1;
		float_t k = (membrane0.stiffness * vol0 + membrane1.stiffness * vol1) / volProportion;
		float_t dr =
		    (membrane0.dampRatio * vol0 + membrane1.dampRatio * vol1) / volProportion;

		pair<Joint, Joint> joints = {
		    Joint(membrane0.getAngularStiffness(),
		          dampingFromRatio(dr, membrane0.getMomentOfInertia() * 2.0,
		                           membrane0.angularStiffness),
		          membrane0.maxTeta),
		    Joint(membrane1.getAngularStiffness(),
		          dampingFromRatio(dr, membrane1.getMomentOfInertia() * 2.0,
		                           membrane1.angularStiffness),
		          membrane1.maxTeta)};

		con.emplace(make_pair(
		    cells,
		    ConnectionType(
		        make_pair(cells.first, cells.second),
		        Spring(k, dampingFromRatio(dr, cells.first->mass + cells.second->mass, k), l),
		        joints, joints)));
		cells.first->connectedCells.insert(cells.second);
		cells.second->connectedCells.insert(cells.first);
		membrane0.cellConnections[cells.second] = &con.at(cells);
		membrane1.cellConnections[cells.first] = &con.at(cells);
	}

	void division() { setRadius(getBaseRadius()); }
};
}

#endif
