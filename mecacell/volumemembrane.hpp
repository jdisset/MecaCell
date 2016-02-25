#ifndef VOLUMEMEMBRANE_HPP
#define VOLUMEMEMBRANE_HPP
#include "tools.h"
#include "connection.h"
#include "model.h"
#include "contactsurface.hpp"
#include "modelconnection.hpp"
#include "cellcellconnectionmanager.hpp"
#include <utility>
#include <vector>
#include <map>

#undef DBG
#define DBG DEBUG(volumemembrane)

#define FOUR_THIRD_PI 4.1887902047863909846168578443726705

namespace MecaCell {
template <typename Cell> class VolumeMembrane {
	/********************** VolumeMembrane class template **********************/

	CREATE_METHOD_CHECKS(getAdhesionWith);
	template <typename C> struct SFINAE {
		static constexpr bool hasPreciseAdhesion = has_getAdhesionWith_signatures<
		    C, double(C *, double, double), float(C *, float, float),
		    double(const C *, double, double), float(const C *, float, float)>::value;
		static constexpr bool hasAdhesion =
		    has_getAdhesionWith_signatures<C, double(C *), float(C *), double(const C *),
		                                   float(const C *)>::value;

		// different precision level for getAdhesionWith
		// precise : takes a pointer to the other cell + the orientation of the connection
		template <typename T = float_t>
		static inline typename enable_if<hasPreciseAdhesion, T>::type getAdhesion(
		    const Cell *a, const Cell *b, const Vec &ABnorm) {
			const auto B = a->getOrientation();
			bool zNeg = ABnorm.dot(B.X.cross(B.Y)) < 0;
			double phi = acos(ABnorm.dot(B.Y));
			double teta = zNeg ? acos(ABnorm.dot(B.X)) : M_PI * 2.0 - acos(ABnorm.dot(B.X));
			return a->getAdhesionWith(b, teta, phi);
		}
		// not precise: takes a pointer to the other cell
		template <typename T = float_t, typename... Whatever>
		static inline typename enable_if<!hasPreciseAdhesion && hasAdhesion, T>::type
		    getAdhesion(const Cell *a, const Cell *b, Whatever...) {
			return a->getAdhesionWith(b);
		}
		// not even declared: always 0
		template <typename T = float_t, typename... Whatever>
		static inline
		    typename enable_if<!hasPreciseAdhesion && !hasAdhesion, T>::type getAdhesion(
		        Whatever...) {
			return 0.0;
		}
	};

 public:
	using CCCM = CellCellConnectionManager_map<Cell, ContactSurface<Cell>>;
	friend CCCM;
	friend class ContactSurface<Cell>;
	using CellCellConnectionType = typename CCCM::ConnectionType;
	using CellCellConnectionContainer = typename CCCM::CellCellConnectionContainer;
	using CellModelConnectionContainer = int;
	static const bool hasModelCollisions = false;
	static const bool forcesOnMembrane =
	    false;  // are forces applied directly to membrane or to the cell's center?

 protected:
	Cell *cell;
	CCCM cccm;

	// params
	float_t incompressibility = 0.003;
	float_t membraneStiffness = 3;

	// internal stuff
	float_t baseRadius = DEFAULT_CELL_RADIUS;
	float_t restRadius = DEFAULT_CELL_RADIUS;
	// dynamic target radius:
	float_t dynamicRadius = DEFAULT_CELL_RADIUS;
	float_t prevDynamicRadius = DEFAULT_CELL_RADIUS;
	static constexpr float_t MAX_DYN_RADIUS_RATIO = 2.0;
	// this is the effective Radius, the one we deduce from the membrane area:
	float_t deducedRadius = restRadius;
	float_t currentArea = 4.0 * M_PI * restRadius * restRadius;
	float_t restVolume = FOUR_THIRD_PI * restRadius * restRadius;
	float_t currentVolume = restVolume;
	float_t pressure = 0;

 public:
	VolumeMembrane(Cell *c) : cell(c) { updateStats(); };
	VolumeMembrane(Cell *c, const VolumeMembrane &sm)
	    : cell(c),
	      incompressibility(sm.incompressibility),
	      membraneStiffness(sm.membraneStiffness),
	      baseRadius(sm.baseRadius),
	      restRadius(sm.restRadius),
	      dynamicRadius(sm.dynamicRadius),
	      deducedRadius(sm.deducedRadius),
	      currentArea(sm.currentArea),
	      restVolume(sm.restVolume),
	      currentVolume(sm.currentVolume),
	      pressure(sm.pressure){};

	/**********************************************************
	                             GET
	***********************************************************/
	/************************ basics **************************/
	inline Cell *getCell() { return cell; }
	inline CCCM &getCellCellConnectionManager() { return cccm; }
	inline float_t getStiffness() const { return membraneStiffness; }
	inline float_t getRestRadius() const { return restRadius; }
	inline float_t getBaseRadius() const { return baseRadius; }
	inline float_t getDeducedRadius() const { return deducedRadius; }
	inline float_t getDynamicRadius() const { return dynamicRadius; }
	inline float_t getBoundingBoxRadius() const { return dynamicRadius; };
	inline float_t getPressure() const { return pressure; }
	inline float_t getSqRestRadius() const { return restRadius * restRadius; }
	inline float_t getCurrentVolume() const { return currentVolume; }
	inline float_t getRestVolume() const { return restVolume; }
	inline float_t getCurrentArea() const { return currentArea; }

	/************************ computed **************************/
	void division() {
		setRadius(getBaseRadius());
		dynamicRadius = getBaseRadius();
	}

	pair<Cell *, float_t> getConnectedCellAndMembraneDistance(const Vec &d) const {
		// /!\ assumes that d is normalized
		Cell *closestCell = nullptr;
		float_t closestDist = dynamicRadius;
		for (auto &cc : cccm.cellConnections) {
			auto con = CCCM::getConnection(cc);
			auto normal = cell == con->cells.first ? -con->normal : con->normal;
			float_t dot = normal.dot(d);
			if (dot < 0) {
				const auto &midpoint =
				    cell == con->cells.first ? con->midpoint.first : con->midpoint.second;
				float_t l = -midpoint / dot;
				if (l < closestDist) {
					closestDist = l;
					closestCell = con->cells.first == cell ? con->cells.second : con->cells.first;
				}
			}
		}
		return {closestCell, closestDist};
	}

	inline Cell *getConnectedCell(const Vec &d) const {
		return get<0>(getConnectedCellAndMembraneDistance(d));
	}
	inline float_t getPreciseMembraneDistance(const Vec &d) const {
		return get<1>(getConnectedCellAndMembraneDistance(d));
	}
	inline float_t getRestArea() const {
		return roundN(4.0 * M_PI * restRadius * restRadius);
	}
	inline void computeRestVolume() {
		restVolume = roundN(FOUR_THIRD_PI * restRadius * restRadius * restRadius);
	}
	inline float_t getBaseVolume() const {
		return roundN(FOUR_THIRD_PI * baseRadius * baseRadius * baseRadius);
	}
	inline float_t getRestMomentOfInertia() const {
		return 0.4 * cell->mass * restRadius * restRadius;
	}
	inline float_t getMomentOfInertia() const { return getRestMomentOfInertia(); }
	inline float_t getVolumeVariation() const { return restVolume - currentVolume; }

	void computeCurrentVolume() {
		float_t volumeLoss = 0;
		for (auto &cc : cccm.cellConnections) {
			auto con = CCCM::getConnection(cc);
			auto &midpoint =
			    cell == con->cells.first ? con->midpoint.first : con->midpoint.second;
			auto h = dynamicRadius - midpoint;
			volumeLoss += (M_PI * h / 6.0) * (3.0 * con->sqradius + h * h);
		}
		// TODO : soustraire les overlapps
		float_t baseVol = FOUR_THIRD_PI * dynamicRadius * dynamicRadius * dynamicRadius;
		currentVolume = baseVol - volumeLoss;
		const float_t minVol = 0.1 * getRestVolume();
		if (currentVolume < minVol) currentVolume = minVol;
	}

	float_t getVolume() const { return restVolume; }

	void computeAreaAndDeduceRadius() {
		float_t surfaceLoss = 0;
		for (auto &cc : cccm.cellConnections) {
			auto con = CCCM::getConnection(cc);
			auto &midpoint =
			    cell == con->cells.first ? con->midpoint.first : con->midpoint.second;
			surfaceLoss += 2.0 * M_PI * dynamicRadius * (dynamicRadius - midpoint) - con->area;
		}
		float_t baseArea = 4.0 * M_PI * dynamicRadius * dynamicRadius;
		currentArea = baseArea - surfaceLoss;
		// TODO : soustraire les overlapps, avoir une meilleure précision
		const float_t minArea =
		    0.1 * getRestArea();  // garde fou en attendant de soustraire les overlaps
		if (currentArea < minArea) currentArea = minArea;
		deducedRadius = sqrt(currentArea / 4.0 * M_PI);
	}

	/**********************************************************
	                             SET
	***********************************************************/
	void setIncompressibility(float_t i) { incompressibility = i; }
	void setStiffness(float_t k) { membraneStiffness = k; }
	void setRadius(float_t r) { restRadius = r; }
	void setBaseRadius(float_t r) { baseRadius = r; }
	void setRadiusRatio(float_t r) { restRadius = r * baseRadius; }
	void setVolume(float_t v) { setRadius(cbrt(v / (4.0 * M_PI / 3.0))); }

	/**********************************************************
	                           UPDATE
	***********************************************************/
	template <typename Integrator> void updatePositionsAndOrientations(double dt) {
		// Before updating positions and orientations we compute the cureent pressure
		computeCurrentVolume();
		float_t dA = currentArea - getRestArea();
		float_t dV = getRestVolume() - currentVolume;
		auto Fv = incompressibility * dV;
		auto Fa = membraneStiffness * dA;
		pressure = Fv / currentArea;
		float_t dynSpeed = (dynamicRadius - prevDynamicRadius) / dt;
		float_t c = 5.0;
		dynamicRadius += dt * dt * (Fv - Fa - dynSpeed * c);
		if (dynamicRadius > restRadius * MAX_DYN_RADIUS_RATIO)
			dynamicRadius = restRadius * MAX_DYN_RADIUS_RATIO;
		else if (dynamicRadius < restRadius)
			dynamicRadius = restRadius;
		cell->receiveTorque(-cell->getAngularVelocity() * 50.0);
		Integrator::updatePosition(*cell, dt);
		Integrator::updateOrientation(*cell, dt);
		cell->markAsNotTested();
	}

	void resetForces() {
		cell->force = Vec::zero();
		cell->torque = Vec::zero();
	}

	void updateStats() {
		computeRestVolume();
		computeCurrentVolume();
		computeAreaAndDeduceRadius();
	}

	/**********************************************************
	                       CONNECTIONS
	***********************************************************/
	/******************** between cells ***********************/
	template <typename C = Cell, typename... T>
	static inline float_t getAdhesion(T &&... t) {
		return SFINAE<C>::getAdhesion(std::forward<T>(t)...);
	}

	template <typename SpacePartition>
	static void checkForCellCellConnections(
	    vector<Cell *> &cells, CellCellConnectionContainer &cellCellConnections,
	    SpacePartition &grid) {
		vector<ordered_pair<Cell *>> newConnections;
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
						if (sqDistance < pow(op.first->getConstMembrane().dynamicRadius +
						                         op.second->getConstMembrane().dynamicRadius,
						                     2)) {
							if (!CCCM::areConnected(op.first, op.second) &&
							    !isInVector(op, newConnections) && op.first != op.second) {
								float_t dist = sqrt(sqDistance);
								Vec dir = AB / dist;
								auto d0 = op.first->getConstMembrane().getPreciseMembraneDistance(dir);
								auto d1 = op.second->getConstMembrane().getPreciseMembraneDistance(-dir);
								if (dist < 0.99999999 * (d0 + d1)) {
									newConnections.push_back(op);
								}
							}
						}
					}
				}
			}
		}
		for (const auto &nc : newConnections) {
			CCCM::createConnection(cellCellConnections, nc, nc);
		}
	}

	static void updateCellCellConnections(CellCellConnectionContainer &concon, float_t dt) {
		// we update forces & delete impossible connections (cells not in contact
		// anymore...)
		vector<CellCellConnectionType *> toDisconnect;
		for (auto &cc : concon) {
			CellCellConnectionType *con = CCCM::getConnection(cc);
			con->update(dt);
			if (con->area <= 0) {
				toDisconnect.push_back(con);
			}
		}
		for (CellCellConnectionType *c : toDisconnect) {
			CCCM::disconnect(concon, c->cells.first, c->cells.second, c);
		}
	}

	static inline void disconnectAndDeleteAllConnections(Cell *c0,
	                                                     CellCellConnectionContainer &con) {
		auto cop = c0->membrane.cccm.cellConnections;
		for (auto &cc : cop) {
			CCCM::disconnect(con, cc->cells.first, cc->cells.second, cc);
		}
	}
};
}

#endif
