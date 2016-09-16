#ifndef GRID_HPP
#define GRID_HPP

#include <array>
#include <deque>
#include <iostream>
#include <set>
#include <unordered_map>
#include <vector>
#include "../geometry/geometry.h"
#include "unique_vector.hpp"
#include "utils.h"

namespace MecaCell {

/**
 * @brief Infinite grid of fixed cell size for space partitioning
 *
 * @tparam O the type of object that are stored
 */
template <typename O> class Grid {
 private:
	double cellSize;  // actually it's 1/cellSize, just so we can multiply instead of divide
	std::unordered_map<Vec, size_t> um;                      // position -> orderedVec index
	std::vector<std::pair<Vec, std::vector<O>>> orderedVec;  // for determinism

 public:
	Grid(double cs) : cellSize(1.0 / cs) {}
	size_t size() { return um.size(); };

	double getCellSize() const { return 1.0 / cellSize; }

	const std::vector<std::pair<Vec, std::vector<O>>> &getContent() const {
		return orderedVec;
	}

	array<vector<vector<O>>, 8> getThreadSafeGrid() const {
		// same color batches can be safely treated in parallel (if max O size < cellSize)
		std::array<std::vector<std::vector<O>>, 8> res;
		for (const auto &c : orderedVec) res[vecToColor(c.first)].push_back(c.second);
		return res;
	}

	inline size_t vecToColor(const Vec &v) const {
		return (abs((int)v.x()) % 2) + (abs((int)v.y()) % 2) * 2 + (abs((int)v.z()) % 2) * 4;
	}

	template <typename V> void insert(V &&k, const O &o) {
		if (!um.count(std::forward<V>(k))) {
			um[std::forward<V>(k)] = orderedVec.size();
			orderedVec.push_back({std::forward<V>(k), std::vector<O>()});
		}
		orderedVec[um[std::forward<V>(k)]].second.push_back(o);
	}

	void insertOnlyCenter(const O &obj) {
		insert(getIndexFromPosition(ptr(obj)->getPosition()), obj);
	}

	void insert(const O &obj) {
		const Vec &center = ptr(obj)->getPosition();
		const double &radius = ptr(obj)->getBoundingBoxRadius();
		Vec minCorner = getIndexFromPosition(center - radius);
		Vec maxCorner = getIndexFromPosition(center + radius);
		for (double i = minCorner.x(); i <= maxCorner.x(); ++i) {
			for (double j = minCorner.y(); j <= maxCorner.y(); ++j) {
				for (double k = minCorner.z(); k <= maxCorner.z(); ++k) {
					insert(Vec(i, j, k), obj);
				}
			}
		}
	}

	void insertPrecise(const O &obj) {
		// good for gridSize << boundingboxRadius
		const Vec &center = ptr(obj)->getPosition();
		const double &radius = ptr(obj)->getBoundingBoxRadius();
		const double sqRadius = radius * radius;
		const double cubeSize = 1.0 / cellSize;
		Vec minCorner = getIndexFromPosition(center - radius);
		Vec maxCorner = getIndexFromPosition(center + radius);
		for (double i = minCorner.x(); i <= maxCorner.x(); ++i) {
			for (double j = minCorner.y(); j <= maxCorner.y(); ++j) {
				for (double k = minCorner.z(); k <= maxCorner.z(); ++k) {
					// we test if this cube's center overlaps with the sphere
					// i j k coords are the bottom front left coords of a 1/cellSize cube
					Vec cubeCenter(i + cubeSize, j + cubeSize, k + cubeSize);
					if ((cubeCenter - center).sqlength() < sqRadius) {
						insert(Vec(i, j, k), obj);
					}
				}
			}
		}
	}

	void insert(const O &obj, const Vec &p0, const Vec &p1,
	            const Vec &p2) {  // insert triangles
		Vec blf(min(p0.x(), min(p1.x(), p2.x())), min(p0.y(), min(p1.y(), p2.y())),
		        min(p0.z(), min(p1.z(), p2.z())));
		Vec trb(max(p0.x(), max(p1.x(), p2.x())), max(p0.y(), max(p1.y(), p2.y())),
		        max(p0.z(), max(p1.z(), p2.z())));
		double cs = 1.0 / cellSize;
		getIndexFromPosition(blf).iterateTo(getIndexFromPosition(trb) + 1, [&](const Vec &v) {
			Vec center = cs * v;
			std::pair<bool, Vec> projec = projectionIntriangle(p0, p1, p2, center);
			if ((center - projec.second).sqlength() < 0.8 * cs * cs) {
				if (projec.first || closestDistToTriangleEdge(p0, p1, p2, center) < 0.87 * cs) {
					insert(v, obj);
				}
			}
		});
	}

	Vec getIndexFromPosition(const Vec &v) const {
		Vec res = v * cellSize;
		return Vec(floor(res.x()), floor(res.y()), floor(res.z()));
	}

	vector<O> retrieve(const Vec &center, double radius) const {
		unique_vector<O> res;
		Vec minCorner = getIndexFromPosition(center - radius);
		Vec maxCorner = getIndexFromPosition(center + radius);
		for (double i = minCorner.x(); i <= maxCorner.x(); ++i) {
			for (double j = minCorner.y(); j <= maxCorner.y(); ++j) {
				for (double k = minCorner.z(); k <= maxCorner.z(); ++k) {
					const Vector3D v(i, j, k);
					if (um.count(v))
						res.insert(orderedVec[um.at(v)].second.begin(),
						           orderedVec[um.at(v)].second.end());
				}
			}
		}
		return res.getUnderlyingVector();
	}

	vector<O> retrieve(const O &obj) const {
		return retrieve(ptr(obj)->getPosition(), ptr(obj)->getBoundingBoxRadius());
	}

	double computeSurface() const {
		if (Vec::dimension == 3) {
			double res = 0.0;  // first = surface, second = volume;
			double faceArea = pow(1.0 / cellSize, 2);
			for (auto &i : um) {
				res += (6.0 - static_cast<double>(getNbNeighbours(i.first))) * faceArea;
			}
			return res;
		}
		return pow(1.0 / cellSize, 2) * static_cast<double>(um.size());
	}

	double getVolume() const {
		if (Vec::dimension == 3)
			return pow(1.0 / cellSize, 3) * static_cast<double>(um.size());
		return 0.0;
	}

	double computeSphericity() const {
		auto s = computeSurface();
		if (s <= 0) return -1;
		return (cbrt(M_PI) * (pow(6.0 * getVolume(), (2.0 / 3.0)))) / s;
	}

	// nb of occupied neighbour grid cells
	int getNbNeighbours(const Vec &cell) const {
		int res = 0;
		if (um.count(cell - Vec(0, 0, 1))) ++res;
		if (um.count(cell - Vec(0, 1, 0))) ++res;
		if (um.count(cell - Vec(1, 0, 0))) ++res;
		if (um.count(cell + Vec(0, 0, 1))) ++res;
		if (um.count(cell + Vec(0, 1, 0))) ++res;
		if (um.count(cell + Vec(1, 0, 0))) ++res;
		return res;
	}

	void clear() {
		um.clear();
		orderedVec = decltype(orderedVec)();
	}
};
}
#endif
