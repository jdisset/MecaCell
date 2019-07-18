#pragma once
#include <vector>
#include "../geometry/geometry.hpp"
#include "unique_vector.hpp"
#include "utils.hpp"

namespace MecaCell {
/**
 * @brief 2D grid containing a value of type T
 *
 * @tparam T the type of qtty that will be stored in the grid
 */
template <typename T> struct Simple2DGrid {
	using coord_t = std::array<size_t, 2>;
	using AABB_t = std::pair<coord_t, coord_t>;

	std::vector<std::pair<size_t, std::reference_wrapper<std::vector<T>>>> orderedVec;
	inline decltype(orderedVec)& getOrderedVec() { return orderedVec; }

	std::vector<std::vector<T>> grid;

	size_t w, h;
	num_t bottomLeft_x;
	num_t bottomLeft_z;
	num_t invSide;  // 1 / side

	/**
	 * @brief Constructs a grid defined by its corners, the grid cell area and the initial
	 * (nutrient) density
	 * @param bl_x X component of bottom left corner
	 * @param bl_z Z component of bottom left corner
	 * @param tr_x X component of top right corner
	 * @param tr_z Z component of top right corner
	 * @param side length of the sides of each grid cell
	 * @param init initial value
	 */
	Simple2DGrid(num_t bl_x, num_t bl_z, num_t tr_x, num_t tr_z, num_t side)
	    : bottomLeft_x(bl_x), bottomLeft_z(bl_z) {
		assert(tr_x > bl_x);
		assert(tr_z > bl_z);
		invSide = 1.0 / side;
		w = std::ceil((tr_x - bl_x) * invSide);
		h = std::ceil((tr_z - bl_z) * invSide);
		grid = std::vector<std::vector<T>>(w * h);
	}

	inline size_t coordToId(const size_t& x, const size_t& z) const { return x + z * w; }

	inline std::vector<T>& at(const size_t& x, const size_t& z) {
		return grid[coordToId(x, z)];
	}

	inline coord_t worldToGridCoord(const num_t& wx, const num_t& wz) const {
		num_t rx = (wx - bottomLeft_x);
		num_t rz = (wz - bottomLeft_z);
		size_t x = static_cast<size_t>(rx * invSide);
		size_t z = static_cast<size_t>(rz * invSide);
		assert(rx >= 0 && rz >= 0 && x < w && z < h);
		return {{x, z}};
	}

	inline void clear() {
		// TODO: reserve in grid. (use orderedVec to get an idea)
		for (const auto& p : orderedVec) grid[p.first].clear();
		const size_t prevS = orderedVec.size();
		orderedVec.clear();
		orderedVec.reserve(prevS + (prevS/20));
	}

	inline AABB_t getAABB(const std::pair<Vec, Vec>& p) const {
		return std::make_pair(worldToGridCoord(p.first.x(), p.first.z()),
		                      worldToGridCoord(p.second.x(), p.second.z()));
	}

	void insert(const T& t, const AABB_t& aabb) {
		for (auto i = aabb.first[0]; i <= aabb.second[0]; ++i) {
			for (auto j = aabb.first[1]; j <= aabb.second[1]; ++j) {
				auto id = coordToId(i, j);
				grid[id].push_back(t);
				if (grid[id].size() == 1) {
					// just got interesting, should be added to orderedVec;
					orderedVec.push_back(
					    std::make_pair(id, std::reference_wrapper<std::vector<T>>(grid[id])));
				}
			}
		}
	}

	static inline bool AABBCollision(const AABB_t& a, const AABB_t& b) {
		return (a.first[0] <= b.second[0] && a.second[0] >= b.first[0]) &&
		       (a.first[1] <= b.second[1] && a.second[1] >= b.first[1]);
	}

	num_t getTotalArea() const {
		const num_t side = 1.0 / invSide;
		return w * h * side * side;
	}
};  // namespace MecaCell

}  // namespace MecaCell
