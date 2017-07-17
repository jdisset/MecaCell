#ifndef UTILS_H
#define UTILS_H
#include <deque>
#include <random>
#include <unordered_map>
#include <utility>
#include <vector>
#include "../geometry/vector3D.h"
#include "config.hpp"
#include "logger.hpp"

/**
 * @brief this file contains various miscellanious utility functions & helpers     *
 */

namespace MecaCell {


/**
 * @brief alias for Vector3D
 */
using Vec = Vector3D;

/**
 * @brief returns a pointer (transforms reference into pointer)
 *
 * @tparam T 
 * @param obj
 *
 * @return 
 */
template <typename T> inline T *ptr(T &obj) { return &obj; }
/**
 * @brief returns a pointer (transforms reference into pointer)
 *
 * @tparam T 
 * @param obj
 *
 * @return 
 */
template <typename T> inline T *ptr(T *obj) { return obj; }

/**
 * @brief linear interpolation
 *
 * @tparam T
 * @param a
 * @param b
 * @param c coef (0.0 -> 1.0)
 *
 * @return 
 */
template <typename T> constexpr T lerp(const T &a, const T &b, const double &c) {
	return a * (1.0 - c) + c * b;
}

template <typename T> inline bool fuzzyEqual(const T &a, const T &b, double eps = 1e-6) {
	return (fabs(a - b) < eps * fabs(a));
}

// containers helpers
template <typename T> inline bool isInVector(const T &elem, const std::vector<T> &vec) {
	return std::find(vec.begin(), vec.end(), elem) != vec.end();
}
template <typename T> inline void eraseFromVector(const T &elem, std::deque<T> &vec) {
	vec.erase(std::remove(vec.begin(), vec.end(), elem), vec.end());
}
template <typename T> inline void eraseFromVector(const T &elem, std::vector<T> &vec) {
	vec.erase(std::remove(vec.begin(), vec.end(), elem), vec.end());
}

// string manipulation (splits a string)
std::vector<std::string> splitStr(const std::string &s, char delim);

// color spaces transforms
std::array<double, 3> hsvToRgb(double h, double s, double v);

// -------- metaprog (compile time) general helpersi ----------
// power
template <typename T> inline constexpr T constpow(const T base, unsigned const exponent) {
	return (exponent == 0) ? 1 : (base * constpow(base, exponent - 1));
}
// converts a class enum to its corresponding size_t
template <typename T> constexpr size_t eToUI(const T &t) {
	return static_cast<size_t>(t);
}

inline double dampingFromRatio(const double r, const double m, const double k) {
	return r * 2.0 * sqrt(m * k);  // for angular springs m is the moment of inertia
}
}  // namespace MecaCell

namespace std {
// hash function} for generic pair
template <typename T, typename U> struct hash<pair<T, U>> {
	size_t operator()(const pair<T, U> &x) const {
		return hash<T>()(x.first) ^ hash<U>()(x.second);
	}
};
}  // namespace std
#endif
