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

extern float_t DEFAULT_CELL_DAMP_RATIO;
extern float_t DEFAULT_SPRING_DAMP_RATIO;
extern float_t DEFAULT_JOINT_DAMP_RATIO;
extern float_t DEFAULT_CELL_MASS;
extern float_t DEFAULT_CELL_RADIUS;
extern float_t DEFAULT_CELL_STIFFNESS;
extern float_t DEFAULT_CELL_ANG_STIFFNESS;
extern float_t MIN_CELL_ADH_LENGTH;
extern float_t MAX_CELL_ADH_LENGTH;
extern float_t ADH_THRESHOLD;
float_t dampingFromRatio(const float_t r, const float_t m, const float_t k);
template <typename T> constexpr T mix(const T &a, const T &b, const float_t &c) {
	return a * (1.0 - c) + c * b;
}
float_t closestDistToTriangleEdge(const Vec &v0, const Vec &v1, const Vec &v2,
                                  const Vec &n);

std::pair<bool, Vec> projectionIntriangle(const Vec &v0, const Vec &v1, const Vec &v2,
                                          const Vec &p, const float_t tolerance = 0.0);

std::pair<bool, Vec> rayInTriangle(const Vec &v0, const Vec &v1, const Vec &v2,
                                   const Vec &o, const Vec &r,
                                   const float_t tolerance = 0.0);

vector<Vec> getSpherePointsPacking(unsigned int);
double updateElectrostaticPointsOnSphere(vector<Vec> &p, double dt = 1);

Vec hsvToRgb(float_t h, float_t s, float_t v);
extern std::default_random_engine globalRand;
std::vector<std::string> splitStr(const std::string &s, char delim);

// return a pointer (transform reference into pointer)
template <typename T> T *ptr(T &obj) { return &obj; }
template <typename T> T *ptr(T *obj) { return obj; }
}
#endif
