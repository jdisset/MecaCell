#ifndef SPHEREMEMBRANE_HPP
#define SPHEREMEMBRANE_HPP
#include "tools.h"
#include "connection.h"
#include "model.h"
#include "modelconnection.hpp"
#include "cellcellconnectionmanager.hpp"
#include <unordered_map>
#include <utility>
#include <vector>
#include <map>

// default breaking connection angle
#define DEFAULT_MAX_TETA M_PI / 12.0
// threshold (dot product) above which we want two model connections to be merged
#define MIN_MODEL_CONNECTION_SIMILARITY 0.8

#undef DBG
#define DBG DEBUG(spheremembrane)

namespace MecaCell {
template <typename Cell> class SphereMembrane {
	CREATE_METHOD_CHECKS(getAdhesionWith);
	static constexpr bool hasPreciseAdhesion = has_getAdhesionWith_signatures<
	    Cell, double(Cell *, double, double), float(Cell *, float, float),
	    double(const Cell *, double, double), float(const Cell *, float, float)>::value;
	static constexpr bool hasAdhesion =
	    has_getAdhesionWith_signatures<Cell, double(Cell *), float(Cell *),
	                                   double(const Cell *), float(const Cell *)>::value;

	/********************** SphereMembrane class template **********************/
	// Abstract :
	// A sphere membrane is a very crude membrane approximation where a cell is
	// only defined by its radius. It is meant to be fast while allowing cells to
	// dynamically connect and bounce. It provides rudimentary (but fast)
	// volume conservation approximations and tries to maintain an adhesive
	// strength proportional to the contact surface.

 public:
	using CCCM = CellCellConnectionManager_map<Cell>;
	friend CCCM;
	using ModelConnectionType = CellModelConnection<Cell>;
	using CellCellConnectionType = typename CCCM::ConnectionType;
	using CellCellConnectionContainer = typename CCCM::CellCellConnectionContainer;
	using CellModelConnectionContainer =
	    unordered_map<Model *,
	                  unordered_map<Cell *, vector<unique_ptr<CellModelConnection<Cell>>>>>;
	static const bool forcesOnMembrane =
	    false;  // are forces applied directly to membrane or to the cell's center?

 protected:
	Cell *cell;
	CCCM cccm;
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
	bool volumeConservation = false;

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
	inline Cell *getCell() { return cell; }
	inline CCCM &getCellCellConnectionManager() { return cccm; }
	inline float_t getBoundingBoxRadius() const { return correctedRadius; };
	inline float_t getStiffness() const { return stiffness; }
	inline float_t getRadius() const { return radius; }
	inline float_t getBaseRadius() const { return baseRadius; }
	inline float_t getCorrectedRadius() const { return correctedRadius; }
	inline float_t getPressure() const { return pressure; }
	inline float_t getSqradius() const { return radius * radius; }
	inline float_t getDampRatio() const { return dampRatio; }
	inline float_t getAngularStiffness() const { return angularStiffness; }

	/************************ computed **************************/
	bool shouldDisconnect(const Cell *other,
	                      const CellCellConnectionType &connection) const {
		const Vec &d = other == connection.getConstNode1() ? connection.getDirection() :
		                                                     -connection.getDirection();
		const float_t tolerance = 0.99;
		float_t mp = getConnectionMidpoint(cell, other, connection);
		for (const auto &cc : cccm.cellConnections) {
			auto &con = CCCM::getConnection(cc);
			auto &otherCell =
			    cell == con.getConstNode0() ? con.getConstNode1() : con.getConstNode0();
			if (otherCell != other) {
				Vec normal =
				    cell == con.getConstNode0() ? -con.getDirection() : con.getDirection();
				float_t dot = normal.dot(d);
				if (dot < 0) {
					float_t midpoint =
					    con.getLength() * correctedRadius /
					    (correctedRadius + otherCell->getConstMembrane().correctedRadius);
					float_t l = -midpoint / dot;
					if (l < mp * tolerance) {
						return true;
					}
				}
			}
		}
		return false;
	}

	pair<Cell *, float_t> getConnectedCellAndMembraneDistance(const Vec &d) const {
		// /!\ assumes that d is normalized
		Cell *closestCell = nullptr;
		float_t closestDist = correctedRadius;
		for (auto &cc : cccm.cellConnections) {
			auto &con = CCCM::getConnection(cc);
			auto &otherCell = cell == con.getNode0() ? con.getNode1() : con.getNode0();
			Vec normal = cell == con.getNode0() ? -con.getDirection() : con.getDirection();
			float_t dot = normal.dot(d);
			if (dot < 0) {
				float_t midpoint =
				    con.getLength() * correctedRadius /
				    (correctedRadius + otherCell->getConstMembrane().correctedRadius);
				float_t l = -midpoint / dot;
				if (l < closestDist || (fuzzyEqual(l, closestDist) && closestCell &&
				                        closestCell->id > otherCell->id)) {
					closestDist = l;
					closestCell = otherCell;
				}
			}
		}
		return {closestCell, closestDist};
	}

	inline Cell *getConnectedCell(const Vec &d) const {
		// /!\ multiple cells can be at the same shortest distance
		return get<0>(getConnectedCellAndMembraneDistance(d));
	}
	inline float_t getPreciseMembraneDistance(const Vec &d) const {
		return get<1>(getConnectedCellAndMembraneDistance(d));
	}

	inline float_t getVolume() const {
		return roundN((4.0 / 3.0) * M_PI * radius * radius * radius);
	}
	inline float_t getBaseVolume() const {
		return roundN((4.0 / 3.0) * M_PI * baseRadius * baseRadius * baseRadius);
	}
	inline float_t getMomentOfInertia() const { return 4.0 * cell->mass * radius * radius; }
	double getCurrentActualVolume() {
		double targetVol =
		    (4.0 / 3.0) * M_PI * correctedRadius * correctedRadius * correctedRadius;
		double volumeLoss = 0;
		for (auto &cc : cccm.cellConnections) {
			auto &con = CCCM::getConnection(cc);
			auto &other = cell == con.getNode0() ? con.getNode1() : con.getNode0();
			double midpoint =
			    con.getLength() * radius / (radius + other->getConstMembrane().radius);
			double h = getCorrectedRadius() - midpoint;
			volumeLoss +=
			    (M_PI * h / 6.0) *
			    (3.0 * (getCorrectedRadius() * getCorrectedRadius() - midpoint * midpoint) +
			     h * h);
		}
		return roundN(targetVol - volumeLoss);
	}

	static inline float_t getConnectionLength(const ordered_pair<Cell *> cells) {
		Vec dir = (cells.second->getPosition() - cells.first->getPosition()).normalized();
		return getConnectionLength(cells.first->getConstMembrane().correctedRadius +
		                               cells.second->getConstMembrane().correctedRadius,
		                           min(getAdhesion(cells.first, cells.second, dir),
		                               getAdhesion(cells.second, cells.first, -dir)));
	}

	static float_t getConnectionLength(float_t l, float_t adh) {
		if (adh > ADH_THRESHOLD)
			return mix(MAX_CELL_ADH_LENGTH * l, MIN_CELL_ADH_LENGTH * l, adh);
		return l;
	}

	/**********************************************************
	                             SET
	***********************************************************/
	void setRadius(float_t r) {
		radius = r;
		correctedRadius = r;
	}
	void setBaseRadius(float_t r) { baseRadius = r; }
	void setRadiusRatio(float_t r) {
		radius = r * baseRadius;
		correctedRadius = r;
	}
	void setVolume(float_t v) { setRadius(cbrt(v / (4.0 * M_PI / 3.0))); }
	void setStiffness(float_t s) { stiffness = s; }
	void setAngularStiffness(float_t s) { angularStiffness = s; }

	/**********************************************************
	                           UPDATE
	***********************************************************/
	template <typename Integrator> void updatePositionsAndOrientations(double dt) {
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
		for (auto &cc : cccm.cellConnections) {
			auto &con = CCCM::getConnection(cc);
			auto &other = cell == con.getNode0() ? con.getNode1() : con.getNode0();
			double midpoint = con.getLength() * correctedRadius /
			                  (correctedRadius + other->getConstMembrane().correctedRadius);
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
					//  => same cell/model pair + similar bounce angle (same face or similar
					//  normal)
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
								// then the anchor. It's just another simple spring that is always at
								// the
								// same height as the cell (orthogonal to the bounce spring)
								// it has a restlength of 0 and follows the cell when its length is more
								// than the cell's radius;
								if (otherconn->anchor.getSc().length > 0) {
									// first we keep the anchor at cell height
									const Vec &anchorDirection = otherconn->anchor.getSc().direction;
									Vec crossp =
									    currentDirection.cross(currentDirection.cross(anchorDirection));
									if (crossp.sqlength() > c->getConstMembrane().radius * 0.02) {
										crossp.normalize();
										float_t projLength = min(
										    (otherconn->anchor.getNode0().getPosition() - c->getPosition())
										        .dot(crossp),
										    c->getConstMembrane().radius);
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
						float_t l =
						    mix(MAX_CELL_ADH_LENGTH * c->getConstMembrane().correctedRadius,
						        MIN_CELL_ADH_LENGTH * c->getConstMembrane().correctedRadius, adh);
						unique_ptr<CellModelConnection<Cell>> cmc(new CellModelConnection<Cell>(
						    Connection<SpaceConnectionPoint, Cell *>(
						        {SpaceConnectionPoint(c->getPosition()), c},  // N0, N1
						        Spring(100, dampingFromRatio(0.9, c->getMass(), 100),
						               0)),  // anchor
						    Connection<ModelConnectionPoint, Cell *>(
						        {ModelConnectionPoint(mf.first, projec.second, mf.second),
						         c},  // N0, N1
						        Spring(c->getConstMembrane().getStiffness(),
						               dampingFromRatio(c->getConstMembrane().getDampRatio(),
						                                c->getMass(),
						                                c->getConstMembrane().getStiffness() * 1.0),
						               l)  // bounce
						        )));
						cmc->anchor.tjEnabled = false;
						c->getMembrane().addModelConnection(cmc.get());
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
						c.first->getMembrane().removeModelConnection(it->get());
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
		for (auto &cc : cellCellConnections) {
			CCCM::getConnection(cc).updateLengthDirection();
		}
		unordered_set<ordered_pair<Cell *>> newConnections;
		grid.clear();
		for (const auto &c : cells) grid.insert(c);
		auto gridCells = grid.getThreadSafeGrid();
		for (auto &batch : gridCells) {
			for (size_t i = 0; i < batch.size(); ++i) {
				for (size_t j = 0; j < batch[i].size(); ++j) {
					for (size_t k = j + 1; k < batch[i].size(); ++k) {
						auto op = make_ordered_cell_pair(batch[i][j], batch[i][k]);
						Vec AB = op.second->position - op.first->position;
						float_t sqDistance = AB.sqlength();
						if (sqDistance < pow(op.first->getConstMembrane().correctedRadius +
						                         op.second->getConstMembrane().correctedRadius,
						                     2)) {
							if (!CCCM::areConnected(op.first, op.second) && !newConnections.count(op) &&
							    op.first != op.second) {
								float_t dist = sqrt(sqDistance);
								Vec dir = AB / dist;
								auto d0 = op.first->getConstMembrane().getPreciseMembraneDistance(dir);
								auto d1 = op.second->getConstMembrane().getPreciseMembraneDistance(-dir);
								if (dist < 0.999999 * (d0 + d1)) {
									newConnections.insert(op);
								}
							}
						}
					}
				}
			}
		}
		for (const auto &nc : newConnections) {
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

	static inline float_t getConnectionMidpoint(const Cell *c0, const Cell *c1,
	                                            const CellCellConnectionType &connection) {
		return connection.getLength() * c0->getConstMembrane().correctedRadius /
		       (c0->getConstMembrane().correctedRadius +
		        c1->getConstMembrane().correctedRadius);
	}

	static void updateCellCellConnections(CellCellConnectionContainer &con, float_t dt) {
		// we update forces & delete impossible connections (cells not in contact
		// anymore...)
		// here connections are just beams between cells centers

		vector<tuple<Cell *, Cell *, CellCellConnectionType *>> toErase;
		for (auto &cc : con) {
			auto &connection = CCCM::getConnection(cc);
			auto &c0 = connection.getNode0();
			auto &c1 = connection.getNode1();
			// we check if the connection still makes sense
			auto c1ClosestToC0 =
			    c0->getConstMembrane().getConnectedCell(connection.getDirection()) == c1;
			auto c0ClosestToC1 =
			    c1->getConstMembrane().getConnectedCell(-connection.getDirection()) == c0;
			auto shouldDisc = c0->getMembrane().shouldDisconnect(c1, connection) ||
			                  c1->getMembrane().shouldDisconnect(c0, connection);
			if (connection.getSc().length > c0->getConstMembrane().correctedRadius +
			                                    c1->getConstMembrane().correctedRadius ||
			    shouldDisc) {
				toErase.push_back(
				    tuple<Cell *, Cell *, CellCellConnectionType *>(c0, c1, &connection));
			} else {
				// connection still ok, we update its parameters
				// according to contact surface and cells volumes
				float_t contactSurface = roundN(
				    M_PI *
				    (pow(connection.getSc().length, 2) +
				     pow((c0->getConstMembrane().radius + c1->getConstMembrane().radius) / 2.0,
				         2)));
				connection.getFlex().first.setCurrentKCoef(contactSurface);
				connection.getFlex().second.setCurrentKCoef(contactSurface);
				connection.getTorsion().first.setCurrentKCoef(contactSurface);
				connection.getTorsion().second.setCurrentKCoef(contactSurface);

				float_t newl =
				    getConnectionLength(c0->getConstMembrane().correctedRadius +
				                            c1->getConstMembrane().correctedRadius,
				                        (getAdhesion(c1, c0, -connection.getDirection()) +
				                         getAdhesion(c0, c1, connection.getDirection())) *
				                            0.5);

				connection.getSc().setRestLength(newl);
				// then we compute the forces
				connection.computeForces(dt);
			}
		}
		for (auto &p : toErase) {
			CCCM::disconnect(con, get<0>(p), get<1>(p), get<2>(p));
		}
	}

	// different precision level for getAdhesionWith
	// precise : takes a pointer to the other cell + the orientation of the connection
	template <typename T = float_t>
	static inline typename enable_if<hasPreciseAdhesion, T>::type getAdhesion(
	    const T *a, const T *b, const Vec &ABnorm) {
		const auto B = a->getOrientation();
		bool zNeg = ABnorm.dot(B.X.cross(B.Y)) < 0;
		double phi = acos(ABnorm.dot(B.Y));
		double teta = zNeg ? acos(ABnorm.dot(B.X)) : M_PI * 2.0 - acos(ABnorm.dot(B.X));
		return a->getAdhesionWith(b, teta, phi);
	}
	// not precise: takes a pointer to the other cell
	template <typename T = float_t, typename... Whatever>
	static inline typename enable_if<!hasPreciseAdhesion && hasAdhesion, T>::type
	    getAdhesion(const T *a, const T *b, Whatever...) {
		return a->getAdhesionWith(b);
	}
	// not even declared: always 0
	template <typename T = float_t, typename... Whatever>
	static inline
	    typename enable_if<!hasPreciseAdhesion && !hasAdhesion, T>::type getAdhesion(
	        Whatever...) {
		return 0.0;
	}

	static inline void disconnectAndDeleteAllConnections(Cell *c0,
	                                                     CellCellConnectionContainer &con) {
		auto cop = c0->connectedCells;
		for (auto &c1 : cop) {
			CCCM::disconnect(con, c0, c1);
		}
	}

	static void createConnection(ordered_pair<Cell *> cells,
	                             CellCellConnectionContainer &con) {
		float_t l = getConnectionLength(cells);

		auto &membrane0 = cells.first->getMembrane();
		auto &membrane1 = cells.second->getMembrane();
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

		CCCM::createConnection(
		    con, cells,
		    Spring(k, dampingFromRatio(dr, cells.first->mass + cells.second->mass, k), l),
		    joints, joints);
	}

	void division() { setRadius(getBaseRadius()); }

	// void deleteOverlapingConnections() {
	// float_t overlapCoef = 0.9;
	// for (auto c0It = vec.begin(); c0It < vec.end();) {
	// bool deleted = false;  // tells if c0 was deleted inside the inner loop (so we
	// know
	//// if we have to increment c0It)
	// connect_type *c0 = *c0It;
	// if (c0->getNode1() != nullptr) {  // if this is not a wall connection
	// Vec c0dir;
	// Cell *other0 = nullptr;
	// float_t r0;
	// if (c0->getNode0() == cell) {
	// c0dir = c0->getDirection();
	// r0 = c0->getNode1()->getRadius();
	// other0 = c0->getNode1();
	//} else {
	// c0dir = -c0->getDirection();
	// r0 = c0->getNode0()->getRadius();
	// other0 = c0->getNode0();
	//}
	// Vec c0v = c0dir * c0->getLength();
	// float_t c0SqLength = pow(c0->getLength(), 2);
	// for (auto c1It = c0It + 1; c1It < vec.end();) {
	// connect_type *c1 = *c1It;
	// if (c1->getNode1() != nullptr) {
	// Vec c1dir;
	// Cell *other1 = nullptr;
	// float_t r1;
	// if (c1->getNode0() == cell) {
	// c1dir = c1->getDirection();
	// r1 = c1->getNode1()->getRadius();
	// other1 = c1->getNode1();
	//} else {
	// c1dir = -c1->getDirection();
	// r1 = c1->getNode0()->getRadius();
	// other1 = c1->getNode0();
	//}
	// Vec c1v = c1dir * c1->getLength();
	// float_t c1SqLength = pow(c1->getLength(), 2);
	// float_t scal01 = c0v.dot(c1dir);
	// float_t scal10 = c1v.dot(c0dir);
	// if (scal01 > 0 && c0SqLength < c1SqLength &&
	//(c0SqLength - scal01 * scal01) < r0 * r0 * overlapCoef) {
	// c1It = vec.erase(c1It);
	// other1->eraseConnection(c1);
	// cell->eraseCell(other1);
	// other1->eraseCell(cell);
	// connections.erase(remove(connections.begin(), connections.end(), c1),
	// connections.end());
	// delete c1;
	//} else if (scal10 > 0 && c1SqLength < c0SqLength &&
	//(c1SqLength - scal10 * scal10) < r1 * r1 * overlapCoef) {
	// c0It = vec.erase(c0It);
	// other0->eraseConnection(c0);
	// cell->eraseCell(other0);
	// other0->eraseCell(cell);
	// connections.erase(remove(connections.begin(), connections.end(), c0),
	// connections.end());
	// deleted = true;
	// delete c0;
	// break;  // we need to exit the inner loop, c0 doesn't exist
	//// anymore.
	//} else {
	//++c1It;
	//}
	//} else {
	//++c1It;
	//}
	//}
	//}
	// if (!deleted) ++c0It;
	//}
	//}
};
}

#endif
