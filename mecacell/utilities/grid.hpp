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
	// using ivec_t = std::array<int, 3>;
	// using AABB_t = std::pair<ivec_t, ivec_t>;
	using ivec_t = Vec;
	using AABB_t = std::pair<Vec, Vec>;

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

	inline size_t vecToColor(const ivec_t &v) const {
		decltype(v[0]) M = 2;
		return static_cast<size_t>(fmod(abs(v[0]), M) + fmod(abs(v[1]), M) * 2 + fmod(abs(v[2]), M) * 4);
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

	// static int fastFloor(const num_t &n) { return static_cast<int>(std::floor(n)); }
	static auto fastFloor(const num_t &n) { return std::floor(n); }

	// static inline int fastFloor(const num_t &n) { return static_cast<int>(n + 10000000) -
	// 10000000; } // faster but dangerous

	static inline std::pair<Vec, Vec> getAABBVec(const O &obj,
	                                             const num_t radFactor = 1.0){
		const Vec &center = ptr(obj)->getPosition();
		const Vec R{ptr(obj)->getBoundingBoxRadius() * radFactor};
		return std::make_pair<Vec, Vec>(center - R, center + R);
	}

	inline AABB_t getAABB(const std::pair<Vec, Vec> &realAABB) const {
		Vec minCorner = realAABB.first * cellSize;
		Vec maxCorner = realAABB.second * cellSize;
		return std::make_pair<ivec_t, ivec_t>(
		    ivec_t{fastFloor(minCorner.x()), fastFloor(minCorner.y()), fastFloor(minCorner.z())},
		    ivec_t{fastFloor(maxCorner.x()), fastFloor(maxCorner.y()), fastFloor(maxCorner.z())});
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

	// insert when the aabb is already computed
	void insert(const O &obj, const AABB_t &aabb) {
		for (auto i = aabb.first[0]; i <= aabb.second[0]; ++i) {
			for (auto j = aabb.first[1]; j <= aabb.second[1]; ++j) {
				for (auto k = aabb.first[2]; k <= aabb.second[2]; ++k) {
					insert(ivec_t{i, j, k}, obj);
				}
			}
		}
	}

	void insert(const O &obj, const num_t radFactor = 1.0) {
		insert(obj, getAABB(obj, radFactor));
	}

	void remove(const O &obj, const AABB_t &aabb) {
		for (auto i = aabb.first[0]; i <= aabb.second[0]; ++i) {
			for (auto j = aabb.first[1]; j <= aabb.second[1]; ++j) {
				for (auto k = aabb.first[2]; k <= aabb.second[2]; ++k) {
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

	Vec getIndexFromPosition(const Vec &v) const {
		Vec res = v * cellSize;
		return Vec(floor(res.x()), floor(res.y()), floor(res.z()));
	}

	vector<O> retrieve(const Vec &center, num_t radius) const {
		unique_vector<O> res;
		auto aabb = getAABB(center, radius);
		for (auto i = aabb.first[0]; i <= aabb.second[0]; ++i) {
			for (auto j = aabb.first[1]; j <= aabb.second[1]; ++j) {
				for (auto k = aabb.first[2]; k <= aabb.second[2]; ++k) {
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
