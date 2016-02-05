#ifndef GRID_HPP
#define GRID_HPP

#include <vector>
#include <set>
#include <unordered_set>
#include <deque>
#include <iostream>
#include <unordered_map>
#include <array>
#include "tools.h"
using namespace std;

namespace MecaCell {
template <typename O> class Grid {
 private:
	float_t cellSize;  // actually it's 1/cellSize, just so we can multiply
	unordered_map<Vec, vector<O>> um;

 public:
	Grid(float_t cs) : cellSize(1.0 / cs) {}

	float_t getCellSize() const { return 1.0 / cellSize; }
	const unordered_map<Vec, vector<O>> &getContent() const { return um; }

	array<vector<vector<O>>, 8> getThreadSafeGrid() const {
		array<vector<vector<O>>, 8> res;
		for (const auto &c : um) {
			size_t color = vecToColor(c.first);
			// vector<O> vec = c.second;
			// sort(vec.begin(), vec.end());
			// vec.erase(unique(vec.begin(), vec.end()), vec.end());
			// res[color].push_back(vec);
			res[color].push_back(c.second);
		}
		return res;
	}

	inline size_t vecToColor(const Vec &v) const {
		return (abs((int)v.x()) % 2) + (abs((int)v.y()) % 2) * 2 + (abs((int)v.z()) % 2) * 4;
	}

	void insert(const O &obj) {
		const Vec &center = ptr(obj)->getPosition();
		const float_t &radius = ptr(obj)->getBoundingBoxRadius();
		Vec minCorner = getIndexFromPosition(center - radius);
		Vec maxCorner = getIndexFromPosition(center + radius);
		for (double i = minCorner.x(); i <= maxCorner.x(); ++i) {
			for (double j = minCorner.y(); j <= maxCorner.y(); ++j) {
				for (double k = minCorner.z(); k <= maxCorner.z(); ++k) {
					um[Vec(i, j, k)].push_back(obj);
				}
			}
		}
	}

	void insertPrecise(const O &obj) {
		// good for gridSize << boundingboxRadius
		const Vec &center = ptr(obj)->getPosition();
		const float_t &radius = ptr(obj)->getBoundingBoxRadius();
		const float_t sqRadius = radius * radius;
		const float_t cubeSize = 1.0 / cellSize;
		Vec minCorner = getIndexFromPosition(center - radius);
		Vec maxCorner = getIndexFromPosition(center + radius);
		for (double i = minCorner.x(); i <= maxCorner.x(); ++i) {
			for (double j = minCorner.y(); j <= maxCorner.y(); ++j) {
				for (double k = minCorner.z(); k <= maxCorner.z(); ++k) {
					// we test if this cube's center overlaps with the sphere
					// i j k coords are the bottom front left coords of a 1/cellSize cube
					Vec cubeCenter(i + cubeSize, j + cubeSize, k + cubeSize);
					if ((cubeCenter - center).sqlength() < sqRadius) {
						um[Vec(i, j, k)].push_back(obj);
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
		float_t cs = 1.0 / cellSize;
		getIndexFromPosition(blf).iterateTo(getIndexFromPosition(trb) + 1, [&](const Vec &v) {
			Vec center = cs * v;
			std::pair<bool, Vec> projec = projectionIntriangle(p0, p1, p2, center);
			if ((center - projec.second).sqlength() < 0.8 * cs * cs) {
				if (projec.first || closestDistToTriangleEdge(p0, p1, p2, center) < 0.87 * cs) {
					um[v].push_back(obj);
				}
			}
		});
	}

	Vec getIndexFromPosition(const Vec &v) const {
		Vec res = v * cellSize;
		return Vec(floor(res.x()), floor(res.y()), floor(res.z()));
	}

	// set<O> retrieveUnique(const Vec &coord, float_t r) const {
	// set<O> res;
	// Vec center = coord * cellSize;
	// float_t radius = r * cellSize;
	// Vec minCorner = center - radius;
	// Vec maxCorner = center + radius;
	//// TODO check if faster with a set (uniques...) and by removing  selfcollision
	// minCorner.iterateTo(maxCorner, [&](const Vec &v) {
	// if (um.count(v)) {
	// for (auto &e : um.at(v)) {
	// res.insert(e);
	//}
	//}
	//});
	// return res;
	//}

	unordered_set<O> retrieve(const Vec &center, float_t radius) const {
		unordered_set<O> res;
		Vec minCorner = getIndexFromPosition(center - radius);
		Vec maxCorner = getIndexFromPosition(center + radius);
		for (double i = minCorner.x(); i <= maxCorner.x(); ++i) {
			for (double j = minCorner.y(); j <= maxCorner.y(); ++j) {
				for (double k = minCorner.z(); k <= maxCorner.z(); ++k) {
					const Vector3D v(i, j, k);
					if (um.count(v)) res.insert(um.at(v).begin(), um.at(v).end());
				}
			}
		}
		return res;
	}

	unordered_set<O> retrieve(const O &obj) const {
		unordered_set<O> res;
		Vec center = ptr(obj)->getPosition() * cellSize;
		float_t radius = ptr(obj)->getBoundingBoxRadius() * cellSize;
		Vec minCorner = center - radius;
		Vec maxCorner = center + radius;
		minCorner.iterateTo(maxCorner, [this, &res](const Vec &v) {
			if (um.count(v)) res.insert(res.end(), um.at(v).begin(), um.at(v).end());
		});
		return res;
	}

	float_t computeSurface() const {
		if (Vec::dimension == 3) {
			float_t res = 0.0;  // first = surface, second = volume;
			float_t faceArea = pow(1.0 / cellSize, 2);
			for (auto &i : um) {
				res += (6.0 - static_cast<float_t>(getNbNeighbours(i.first))) * faceArea;
			}
			return res;
		}
		return pow(1.0 / cellSize, 2) * static_cast<float_t>(um.size());
	}

	float_t getVolume() const {
		if (Vec::dimension == 3)
			return pow(1.0 / cellSize, 3) * static_cast<float_t>(um.size());
		return 0.0;
	}

	float_t computeSphericity() const {
		return (cbrt(M_PI) * (pow(6.0 * getVolume(), (2.0 / 3.0)))) / computeSurface();
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

	void clear() { um.clear(); }
};
}
#endif
