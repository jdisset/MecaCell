#ifndef TOOLS_H
#define TOOLS_H
#define TERMINAL_COLORS
#include "logger.hpp"
#include "vector3D.h"
#include "assert.h"
#include <random>
#include <string>
#include <vector>
#include <deque>
#include <iostream>
#include <stdio.h>
#define dispV(v) "(" << v.x << "," << v.y << "," << v.z << ")"
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
std::pair<double, double> updateElectrostaticPointsOnSphere(vector<Vec> &p,
                                                            double dt = 1);

Vec hsvToRgb(float_t h, float_t s, float_t v);
extern std::default_random_engine globalRand;
std::vector<std::string> splitStr(const std::string &s, char delim);

// return a pointer (transform reference into pointer)
template <typename T> inline T *ptr(T &obj) { return &obj; }
template <typename T> inline T *ptr(T *obj) { return obj; }

template <typename T> struct ordered_pair {
	T first, second;
	bool operator==(const ordered_pair &other) const {
		return (first == other.first && second == other.second);
	}
};
template <typename T> inline ordered_pair<T *> make_ordered_cell_pair(T *a, T *b) {
	if (a->id < b->id) return {a, b};
	return {b, a};
}
template <typename T> inline ordered_pair<T> make_ordered_pair(const T &a, const T &b) {
	if (a < b) return {a, b};
	return {b, a};
}
template <typename T> inline bool isInVector(const T &elem, const std::vector<T> &vec) {
	return std::find(vec.begin(), vec.end(), elem) != vec.end();
}

string hexstr(const Vector3D &v);

template <typename T> std::string hexstr(T d) {
	char buffer[30];
	snprintf(buffer, 30, "%A", d);
	return buffer;
}

template <typename T> inline bool fuzzyEqual(const T &a, const T &b) {
	return (fabs(a - b) < 1e-8 * fabs(a));
}

template <typename T> inline void eraseFromVector(const T &elem, std::deque<T> &vec) {
	size_t s0 = vec.size();
	vec.erase(std::remove(vec.begin(), vec.end(), elem), vec.end());
	assert(vec.size() == s0 - 1);
}
template <typename T> inline void eraseFromVector(const T &elem, std::vector<T> &vec) {
	size_t s0 = vec.size();
	vec.erase(std::remove(vec.begin(), vec.end(), elem), vec.end());
	assert(vec.size() == s0 - 1);
}
template <typename T> inline ostream &operator<<(ostream &out, const vector<T> &v) {
	out << "[ ";
	for (auto &e : v) {
		out << e->id << " ";
	}
	out << "]";
	return out;
}
template <typename T> inline constexpr T constpow(const T base, unsigned const exponent) {
	return (exponent == 0) ? 1 : (base * constpow(base, exponent - 1));
}

template <typename T, size_t N = 10> T inline roundN(const T &t) {
	// const int p = constpow(10, N);
	// return (round(t * p) / p) + 0.0;
	return t;
}
template <size_t N = 10> inline Vec roundN(const Vec &v) {
	return Vec(roundN(v.x()), roundN(v.y()), roundN(v.z()));
}
}
namespace std {
template <typename T> struct hash<MecaCell::ordered_pair<T>> {
	size_t operator()(const MecaCell::ordered_pair<T> &x) const {
		return hash<T>()(x.first) ^ hash<T>()(x.second);
	}
};
}
#endif
