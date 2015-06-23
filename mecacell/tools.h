#ifndef TOOLS_H
#define TOOLS_H
#include "vector3D.h"
#include "assert.h"
#include <random>

namespace MecaCell {
typedef Vector3D Vec;
extern double DEFAULT_CELL_DAMP_RATIO;
extern double DEFAULT_SPRING_DAMP_RATIO;
extern double DEFAULT_JOINT_DAMP_RATIO;
extern double DEFAULT_CELL_MASS;
extern double DEFAULT_CELL_RADIUS;
extern double DEFAULT_CELL_STIFFNESS;
extern double DEFAULT_CELL_ANG_STIFFNESS;
extern double MIN_CELL_ADH_LENGTH;
extern double MAX_CELL_ADH_LENGTH;
extern double ADH_THRESHOLD;
int double2int(double d);
double dampingFromRatio(const double r, const double m, const double k);
template <typename T> constexpr T mix(const T &a, const T &b, const double &c) {
	return a * (1.0 - c) + c * b;
}
Vec hsvToRgb(double h, double s, double v);
extern std::default_random_engine globalRand;
}
#endif
