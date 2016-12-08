#ifndef CONFIG_HPP
#define CONFIG_HPP
namespace MecaCell {
struct Config {
	static constexpr double DEFAULT_CELL_DAMP_RATIO = 1.0;
	static constexpr double DEFAULT_CELL_RADIUS = 40;
	static constexpr double DEFAULT_CELL_MASS = DEFAULT_CELL_RADIUS / 1000.0;
	static constexpr double DEFAULT_CELL_STIFFNESS = 1.0;
	static constexpr double DOUBLE_EPSILON = 1e-20;
};
}
#endif
