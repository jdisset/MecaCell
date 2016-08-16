#ifndef CONFIG_HPP
#define CONFIG_HPP
namespace MecaCell {
namespace Config {
static constexpr double DEFAULT_CELL_DAMP_RATIO = 1.0;
static constexpr double DEFAULT_CELL_RADIUS = 40;
static constexpr double DEFAULT_CELL_MASS = DEFAULT_CELL_RADIUS / 1000.0;
static constexpr double DEFAULT_CELL_STIFFNESS = 40.0;
static constexpr double DEFAULT_CELL_ANG_STIFFNESS = 3.0;
static constexpr double MIN_CELL_ADH_LENGTH = 0.6;
static constexpr double MAX_CELL_ADH_LENGTH = 0.8;
static constexpr double ADH_THRESHOLD = 0.1;
}
}
#endif
