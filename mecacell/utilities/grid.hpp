#ifndef GRID_HPP
#define GRID_HPP

#include <array>
#include <deque>
#include <iostream>
#include <set>
#include <unordered_map>
#include <vector>
#include "../geometry/geometry.hpp"
#include "unique_vector.hpp"
#include "utils.hpp"

namespace MecaCell {

/**
 * @brief Infinite grid of fixed cell size for space partitioning
 *
 * @tparam O the type of object that are stored
 */
template <typename O> class Grid {
 public:
	using ivec_t = std::array<int, 3>;
	using AABB_t = std::pair<ivec_t, ivec_t>;

 private:
	// template <typename K, typename V> using umap = ska::flat_hash_map<K, V>;
	template <typename K, typename V> using umap = std::unordered_map<K, V>;
	num_t cellSize;  // actually it's 1/cellSize, just so we can multiply instead of divide
	umap<ivec_t, size_t> um;  // position -> orderedVec index
	std::vector<std::pair<ivec_t, std::vector<O>>> orderedVec;  // for determinism

 public:
	Grid(num_t cs) : cellSize(1.0 / cs) {}
	size_t size() { return um.size(); };
	std::vector<std::pair<ivec_t, std::vector<O>>> &getOrderedVec() { return orderedVec; }
	umap<ivec_t, size_t> &getUnorderedMap() { return um; }

	num_t getCellSize() const { return 1.0 / cellSize; }

	const std::vector<std::pair<ivec_t, std::vector<O>>> &getContent() const {
		return orderedVec;
	}

	std::array<vector<vector<O>>, 8> getThreadSafeGrid() const {
		// same color batches can be safely treated in parallel (if max O size < cellSize)
		std::array<std::vector<std::vector<O>>, 8> res;
		for (const auto &c : orderedVec) res[vecToColor(c.first)].push_back(c.second);
		return res;
	}

	std::array<vector<vector<O>>, 8> getThreadSafeGrid(size_t minEl) const {
		// same color batches can be safely treated in parallel (if max O size < cellSize)
		std::array<std::vector<std::vector<O>>, 8> res;
		for (const auto &c : orderedVec) res[vecToColor(c.first)].push_back(c.second);
		if (minEl > 1) {
			for (auto &color : res) {
				if (color.size() > 1) {
					for (auto it = std::next(color.begin()); it != color.end();) {
						if ((*std::prev(it)).size() <
						    minEl) {      // batch too small, we merge with the previous one
							auto &b = *it;  // current batch
							auto pv = std::prev(it);
							auto &a = *pv;  // prev one
							a.insert(a.end(), b.begin(), b.end());
							it = color.erase(it);
						} else {
							++it;
						}
					}
				}
			}
		}
		return res;
	}

	// inline size_t vecToColor(const Vec &v) const {
	// return (abs((int)v.x()) % 2) + (abs((int)v.y()) % 2) * 2 + (abs((int)v.z()) % 2) * 4;
	//}
	inline size_t vecToColor(const ivec_t &v) const {
		return (abs(v[0]) % 2) + (abs(v[1]) % 2) * 2 + (abs(v[2]) % 2) * 4;
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

	bool AABBCollision(const AABB_t &a, const AABB_t &b) const {
		return (a.first[0] <= b.second[0] && a.second[0] >= b.first[0]) &&
		       (a.first[1] <= b.second[1] && a.second[1] >= b.first[1]) &&
		       (a.first[2] <= b.second[2] && a.second[2] >= b.first[2]);
	}

	bool AABBCollision(const O &o1, const O &o2, const num_t radFactor = 1.0) const {
		auto a = getAABBVec(o1, radFactor);
		auto b = getAABBVec(o2, radFactor);
		return (a.first.x() <= b.second.x() && a.second.x() >= b.first.x()) &&
		       (a.first.y() <= b.second.y() && a.second.y() >= b.first.y()) &&
		       (a.first.z() <= b.second.z() && a.second.z() >= b.first.z());
	}

	static int fastFloor(const num_t &n) { return (int)floor(n); }

	static inline std::pair<Vec, Vec> getAABBVec(const O &obj,
	                                             const num_t radFactor = 1.0) {
		const Vec &center = ptr(obj)->getPosition();
		const Vec R{ptr(obj)->getBoundingBoxRadius() * radFactor};
		return std::make_pair<Vec, Vec>(center - R, center + R);
	}

	inline AABB_t getAABB(const std::pair<Vec, Vec> &realAABB) {
		Vec minCorner = realAABB.first * cellSize;
		Vec maxCorner = realAABB.second * cellSize;
		return std::make_pair<ivec_t, ivec_t>(
		    {{fastFloor(minCorner.x()), fastFloor(minCorner.y()), fastFloor(minCorner.z())}},
		    {{fastFloor(maxCorner.x()), fastFloor(maxCorner.y()), fastFloor(maxCorner.z())}});
	}

	inline AABB_t getAABB(const Vec &center, const num_t &rad) const {
		const Vec R{rad};
		Vec minCorner = (center - R) * cellSize;
		Vec maxCorner = (center + R) * cellSize;
		return std::make_pair<ivec_t, ivec_t>(
		    {{fastFloor(minCorner.x()), fastFloor(minCorner.y()), fastFloor(minCorner.z())}},
		    {{fastFloor(maxCorner.x()), fastFloor(maxCorner.y()), fastFloor(maxCorner.z())}});
	}
	inline AABB_t getAABB(const O &obj, const num_t radFactor = 1.0) const {
		const Vec &center = ptr(obj)->getPosition();
		return getAABB(center, radFactor * ptr(obj)->getBoundingBoxRadius());
	}

	void insert(const O &obj, const AABB_t &aabb) {
		for (int i = aabb.first[0]; i <= aabb.second[0]; ++i) {
			for (int j = aabb.first[1]; j <= aabb.second[1]; ++j) {
				for (int k = aabb.first[2]; k <= aabb.second[2]; ++k) {
					insert(ivec_t{{i, j, k}}, obj);
				}
			}
		}
	}

	void insert(const O &obj, const num_t radFactor = 1.0) {
		insert(obj, getAABB(obj, radFactor));
	}

	void remove(const O &obj, const AABB_t &aabb) {
		for (int i = aabb.first[0]; i <= aabb.second[0]; ++i) {
			for (int j = aabb.first[1]; j <= aabb.second[1]; ++j) {
				for (int k = aabb.first[2]; k <= aabb.second[2]; ++k) {
					ivec_t v{{i, j, k}};
					auto &gridVec = orderedVec[um[v]].second;
					gridVec.erase(std::remove(gridVec.begin(), gridVec.end(), obj), gridVec.end());
					if (gridVec.size() == 0) {
						// WARNING: slight memory leak because orderedVec[um[v]] is empty but not
						// erased.
						um.erase(v);
					}
				}
			}
		}
	}

	void insertPrecise(const O &) {
		/* // good for gridSize << boundingboxRadius*/
		// const Vec &center = ptr(obj)->getPosition();
		// const num_t &radius = ptr(obj)->getBoundingBoxRadius();
		// const num_t sqRadius = radius * radius;
		// const num_t cubeSize = 1.0 / cellSize;
		// Vec minCorner = getIndexFromPosition(center - Vec(radius));
		// Vec maxCorner = getIndexFromPosition(center + Vec(radius));
		// for (num_t i = minCorner.x(); i <= maxCorner.x(); ++i) {
		// for (num_t j = minCorner.y(); j <= maxCorner.y(); ++j) {
		// for (num_t k = minCorner.z(); k <= maxCorner.z(); ++k) {
		//// we test if this cube's center overlaps with the sphere
		//// i j k coords are the bottom front left coords of a 1/cellSize cube
		//// we need the closest corner of a grid cell relative to the center of the obj

		// num_t cx = (i + 0.5) * cubeSize;
		//// if (abs(cx - center.x()) > abs(cx + cubeSize - center.x())) cx += cubeSize;
		// num_t cy = (j + 0.5) * cubeSize;
		//// if (abs(cy - center.y()) > abs(cy + cubeSize - center.y())) cy += cubeSize;
		// num_t cz = (k + 0.5) * cubeSize;
		//// if (abs(cz - center.z()) > abs(cz + cubeSize - center.z())) cz += cubeSize;

		// Vec cubeCenter(cx, cy, cz);
		//// std::cerr << "Pour centre = " << center << ", rayon = " << radius
		////<< ", cubeSize = " << cubeSize << "    :   minCorner = " << minCorner
		////<< ", maxCorner = " << maxCorner << ", bfl = " << Vec(i, j, k)
		////<< "(soit " << Vec(i * cubeSize, j * cubeSize, k * cubeSize)
		////<< ") et closestCorner = " << cubeCenter;

		// if ((cubeCenter - center).squaredNorm() < sqRadius) {
		//// std::cerr << " [OK]" << std::endl;
		// insert(Vec(i, j, k), obj);
		//} else {
		//// std::cerr << " [REFUS]" << std::endl;
		//}
		//}
		//}
		/*}*/
	}

	// void insert(const O &obj, const Vec &p0, const Vec &p1,
	// const Vec &p2) {  // insert triangles
	/* Vec blf(min(p0.x(), min(p1.x(), p2.x())), min(p0.y(), min(p1.y(), p2.y())),*/
	// min(p0.z(), min(p1.z(), p2.z())));
	// Vec trb(max(p0.x(), max(p1.x(), p2.x())), max(p0.y(), max(p1.y(), p2.y())),
	// max(p0.z(), max(p1.z(), p2.z())));
	// num_t cs = 1.0 / cellSize;
	// getIndexFromPosition(blf).iterateTo(getIndexFromPosition(trb) + 1, [&](const Vec
	// &v) { Vec center = cs * v; std::pair<bool, Vec> projec = projectionIntriangle(p0,
	// p1, p2, center); if ((center - projec.second).squaredNorm() < 0.8 * cs * cs) { if
	// (projec.first || closestDistToTriangleEdge(p0, p1, p2, center) < 0.87 * cs) {
	// insert(v, obj);
	//}
	/*}*/
	//});
	//}

	Vec getIndexFromPosition(const Vec &v) const {
		Vec res = v * cellSize;
		return Vec(floor(res.x()), floor(res.y()), floor(res.z()));
	}

	vector<O> retrieve(const Vec &center, num_t radius) const {
		unique_vector<O> res;
		auto aabb = getAABB(center, radius);
		for (int i = aabb.first[0]; i <= aabb.second[0]; ++i) {
			for (int j = aabb.first[1]; j <= aabb.second[1]; ++j) {
				for (int k = aabb.first[2]; k <= aabb.second[2]; ++k) {
					ivec_t v{{i, j, k}};
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

	num_t computeSurface() const {
		if (Vec::dimension == 3) {
			num_t res = 0.0;  // first = surface, second = volume;
			num_t faceArea = pow(1.0 / cellSize, 2);
			for (auto &i : um) {
				res += (6.0 - static_cast<num_t>(getNbNeighbours(i.first))) * faceArea;
			}
			return res;
		}
		return pow(1.0 / cellSize, 2) * static_cast<num_t>(um.size());
	}

	num_t getCellVolume() const { return pow(1.0 / cellSize, 3); }
	num_t getVolume() const {
		if (Vec::dimension == 3) return getCellVolume() * static_cast<num_t>(um.size());
		return 0.0;
	}

	num_t computeSphericity() const {
		auto s = computeSurface();
		if (s <= 0) return -1;
		return (cbrt(M_PI) * (pow(6.0 * getVolume(), (2.0 / 3.0)))) / s;
	}

	ivec_t vtoi(const Vec &vec) {
		return ivec_t{{fastFloor(vec.x()), fastFloor(vec.y()), fastFloor(vec.z())}};
	}

	// nb of occupied neighbour grid cells
	int getNbNeighbours(const Vec &cell) const {
		int res = 0;
		if (um.count(vtoi(cell - Vec(0, 0, 1)))) ++res;
		if (um.count(vtoi(cell - Vec(0, 1, 0)))) ++res;
		if (um.count(vtoi(cell - Vec(1, 0, 0)))) ++res;
		if (um.count(vtoi(cell + Vec(0, 0, 1)))) ++res;
		if (um.count(vtoi(cell + Vec(0, 1, 0)))) ++res;
		if (um.count(vtoi(cell + Vec(1, 0, 0)))) ++res;
		return res;
	}

	void clear() {
		um.clear();
		orderedVec = decltype(orderedVec)();
	}
};
}  // namespace MecaCell
#endif
