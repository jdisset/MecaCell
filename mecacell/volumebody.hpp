#ifndef VOLUMEMEMBRANE_HPP
#define VOLUMEMEMBRANE_HPP
#include <map>
#include <utility>
#include <vector>
#include "cellcellconnectionmanager.hpp"
#include "contactsurface.hpp"
#include "utilities/introspect.hpp"
#include "utilities/utils.hpp"

#define FOUR_THIRD_PI 4.1887902047863909846168578443726705

#define MIN_MODEL_CONNECTION_SIMILARITY 0.8

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
		template <typename T = double>
		static inline typename enable_if<hasPreciseAdhesion, T>::type getAdhesion(
		    const Cell *a, const Cell *b, const Vec &ABnorm) {
			const auto B = a->getOrientation();
			bool zNeg = ABnorm.dot(B.X.cross(B.Y)) < 0;
			double phi = acos(ABnorm.dot(B.Y));
			double teta = zNeg ? acos(ABnorm.dot(B.X)) : M_PI * 2.0 - acos(ABnorm.dot(B.X));
			return a->getAdhesionWith(b, teta, phi);
		}
		// not precise: takes a pointer to the other cell
		template <typename T = double, typename... Whatever>
		static inline typename enable_if<!hasPreciseAdhesion && hasAdhesion, T>::type
		    getAdhesion(const Cell *a, const Cell *b, Whatever...) {
			return a->getAdhesionWith(b);
		}
		// not even declared: always 0
		template <typename T = double, typename... Whatever>
		static inline
		    typename enable_if<!hasPreciseAdhesion && !hasAdhesion, T>::type getAdhesion(
		        Whatever...) {
			return 0.0;
		}
	};

 public:
	using CCCM = CellCellConnectionManager_map<Cell, ContactSurface<Cell>>;
	friend CCCM;
	friend struct ContactSurface<Cell>;
	using CellCellConnectionType = typename CCCM::ConnectionType;
	using CellCellConnectionContainer = typename CCCM::CellCellConnectionContainer;
	static const bool forcesOnMembrane =
	    false;  // are forces applied directly to membrane or to the cell's center?

 protected:
	Cell *cell;
	CCCM cccm;

	// params
	double incompressibility = 0.003;
	double membraneStiffness = 3;

	// internal stuff
	double baseRadius = Config::DEFAULT_CELL_RADIUS;
	double restRadius = Config::DEFAULT_CELL_RADIUS;
	// dynamic target radius:
	double dynamicRadius = Config::DEFAULT_CELL_RADIUS;
	double prevDynamicRadius = Config::DEFAULT_CELL_RADIUS;
	static constexpr double MAX_DYN_RADIUS_RATIO = 2.0;
	// this is the effective Radius, the one we deduce from the membrane area:
	double deducedRadius = restRadius;
	double currentArea = 4.0 * M_PI * restRadius * restRadius;
	double restVolume = FOUR_THIRD_PI * restRadius * restRadius;
	double currentVolume = restVolume;
	double pressure = 0;

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
	inline double getStiffness() const { return membraneStiffness; }
	inline double getRestRadius() const { return restRadius; }
	inline double getBaseRadius() const { return baseRadius; }
	inline double getDeducedRadius() const { return deducedRadius; }
	inline double getDynamicRadius() const { return dynamicRadius; }
	inline double getBoundingBoxRadius() const { return dynamicRadius; };
	inline double getPressure() const { return pressure; }
	inline double getSqRestRadius() const { return restRadius * restRadius; }
	inline double getCurrentVolume() const { return currentVolume; }
	inline double getRestVolume() const { return restVolume; }
	inline double getCurrentArea() const { return currentArea; }

	/************************ computed **************************/
	void division() {
		setRadius(getBaseRadius());
		dynamicRadius = getBaseRadius();
	}

	pair<Cell *, double> getConnectedCellAndMembraneDistance(const Vec &d) const {
		// /!\ assumes that d is normalized
		Cell *closestCell = nullptr;
		double closestDist = dynamicRadius;
		for (auto &cc : cccm.cellConnections) {
			auto con = CCCM::getConnection(cc);
			auto normal = cell == con->cells.first ? -con->normal : con->normal;
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
		return {closestCell, closestDist};
	}

	inline Cell *getConnectedCell(const Vec &d) const {
		return get<0>(getConnectedCellAndMembraneDistance(d));
	}
	inline double getPreciseMembraneDistance(const Vec &d) const {
		return get<1>(getConnectedCellAndMembraneDistance(d));
	}
	inline double getRestArea() const { return (4.0 * M_PI * restRadius * restRadius); }
	inline void computeRestVolume() {
		restVolume = FOUR_THIRD_PI * restRadius * restRadius * restRadius;
	}
	inline double getBaseVolume() const {
		return FOUR_THIRD_PI * baseRadius * baseRadius * baseRadius;
	}
	inline double getRestMomentOfInertia() const {
		return 0.4 * cell->mass * restRadius * restRadius;
	}
	inline double getMomentOfInertia() const { return getRestMomentOfInertia(); }
	inline double getVolumeVariation() const { return restVolume - currentVolume; }

	void computeCurrentVolume() {
		double volumeLoss = 0;
		// cell connections
		for (auto &cc : cccm.cellConnections) {
			auto con = CCCM::getConnection(cc);
			auto &midpoint =
			    cell == con->cells.first ? con->midpoint.first : con->midpoint.second;
			auto h = dynamicRadius - midpoint;
			volumeLoss += (M_PI * h / 6.0) * (3.0 * con->sqradius + h * h);
		}
		// TODO : soustraire les overlapps
		double baseVol = FOUR_THIRD_PI * dynamicRadius * dynamicRadius * dynamicRadius;
		currentVolume = baseVol - volumeLoss;
		const double minVol = 0.1 * getRestVolume();
		if (currentVolume < minVol) currentVolume = minVol;
	}

	double getVolume() const { return restVolume; }

	void computeAreaAndDeduceRadius() {
		double surfaceLoss = 0;
		for (auto &cc : cccm.cellConnections) {
			auto con = CCCM::getConnection(cc);
			auto &midpoint =
			    cell == con->cells.first ? con->midpoint.first : con->midpoint.second;
			surfaceLoss += 2.0 * M_PI * dynamicRadius * (dynamicRadius - midpoint) - con->area;
		}
		double baseArea = 4.0 * M_PI * dynamicRadius * dynamicRadius;
		currentArea = baseArea - surfaceLoss;
		// TODO : soustraire les overlapps, avoir une meilleure précision
		const double minArea =
		    0.1 * getRestArea();  // garde fou en attendant de soustraire les overlaps
		if (currentArea < minArea) currentArea = minArea;
		deducedRadius = sqrt(currentArea / 4.0 * M_PI);
	}

	/**********************************************************
	                             SET
	***********************************************************/
	void setIncompressibility(double i) { incompressibility = i; }
	void setStiffness(double k) { membraneStiffness = k; }
	void setRadius(double r) { restRadius = r; }
	void setBaseRadius(double r) { baseRadius = r; }
	void setRadiusRatio(double r) { restRadius = r * baseRadius; }
	void setVolume(double v) { setRadius(cbrt(v / (4.0 * M_PI / 3.0))); }

	/**********************************************************
	                           UPDATE
	***********************************************************/
	template <typename Integrator> void updatePositionsAndOrientations(double dt) {
		// Before updating positions and orientations we compute the cureent pressure
		computeCurrentVolume();
		double dA = currentArea - getRestArea();
		double dV = getRestVolume() - currentVolume;
		auto Fv = incompressibility * dV;
		auto Fa = membraneStiffness * dA;
		pressure = Fv / currentArea;
		double dynSpeed = (dynamicRadius - prevDynamicRadius) / dt;
		double c = 5.0;
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
	static inline double getAdhesion(T &&... t) {
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
						double sqDistance = AB.sqlength();
						if (sqDistance < pow(op.first->getConstMembrane().dynamicRadius +
						                         op.second->getConstMembrane().dynamicRadius,
						                     2)) {
							if (!CCCM::areConnected(op.first, op.second) &&
							    !isInVector(op, newConnections) && op.first != op.second) {
								double dist = sqrt(sqDistance);
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
			CCCM::createConnection(cellCellConnections, nc,
			                       make_ordered_cell_pair(nc.first, nc.second));
		}
	}

	static void updateCellCellConnections(CellCellConnectionContainer &concon, double dt) {
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
