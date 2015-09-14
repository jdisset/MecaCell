#ifndef TOOLS_H
#define TOOLS_H
#include "vector3D.h"
#include "assert.h"
#include <random>
#include <string>
#include <vector>
#include <iostream>
#define dispV(v) "(" << v.x << "," << v.y << "," << v.z << ")"
#define PURPLE "\033[1;35m"
#define BLUE "\033[34m"
#define GREY "\033[1;30m"
#define YELLOW "\033[1;33m"
#define RED "\033[1;31m"
#define CYAN "\033[36m"
#define CYANBOLD "\033[1;36m"
#define GREEN "\033[32m"
#define GREENBOLD "\033[1;32m"
#define NORMAL "\033[0m"

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
double closestDistToTriangleEdge(const Vec &v0, const Vec &v1, const Vec &v2,
                                 const Vec &n);

std::pair<bool, Vec> projectionIntriangle(const Vec &v0, const Vec &v1, const Vec &v2,
                                          const Vec &p, const double tolerance = 0.0);

std::pair<bool, Vec> rayInTriangle(const Vec &v0, const Vec &v1, const Vec &v2,
                                   const Vec &o, const Vec &r,
                                   const double tolerance = 0.0);

Vec hsvToRgb(double h, double s, double v);
extern std::default_random_engine globalRand;
std::vector<std::string> splitStr(const std::string &s, char delim);

// return a pointer (transform reference into pointer)
template <typename T> T *ptr(T &obj) { return &obj; }
template <typename T> T *ptr(T *obj) { return obj; }
}
#endif
