#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <random>

namespace MecaCell {

using num_t = double;

struct Config {
	// TYPES
	using random_engine_t = std::mt19937;  // used for random vectors

	// CONST
	static constexpr double DEFAULT_CELL_DAMP_RATIO = 1.0;
	static constexpr double DEFAULT_CELL_RADIUS = 40;
	static constexpr double DEFAULT_CELL_MASS = DEFAULT_CELL_RADIUS / 1000.0;
	static constexpr double DEFAULT_CELL_STIFFNESS = 4.0;
	static constexpr double DEFAULT_CELL_YOUNGMOD = 1.0;
	static constexpr double DEFAULT_CELL_POISSONCOEF = 0.3;

	/**
	 * @brief max distance for two doubles to be considered equals (only used for some
	 * geometric operations)
	 */
	static constexpr double DOUBLE_EPSILON = 1e-20;

	/**
	 * @brief access to the static global random engine.This pseudo -
	 * random generator is* used in random 3D vector generation
	.* This method can be used by the user to
	 * change* the default seed** @ return reference to the engine.
	 */
	static random_engine_t& globalRand() {
		static random_engine_t engine;
		return engine;
	}
};
}  // namespace MecaCell
#endif
