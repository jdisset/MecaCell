#ifndef MECACELL_MATRIX4X4_HPP
#define MECACELL_MATRIX4X4_HPP
#include <array>
#include "../utilities/utils.hpp"
#include "rotation.h"

namespace MecaCell {

struct Matrix4x4 {
	std::array<std::array<double, 4>, 4> m = {
	    {{{1, 0, 0, 0}}, {{0, 1, 0, 0}}, {{0, 0, 1, 0}}, {{0, 0, 0, 1}}}};
	Matrix4x4() {}
	Matrix4x4(std::array<std::array<double, 4>, 4> a) : m(a) {}

	/**
	 * @brief Multiplies this matrix by another that scales coordinates by the components of
	 * the given vector
	 *
	 * @tparam V 3D vector type
	 * @param s scale vector
	 */
	template <typename V> void scale(const V &s) {
		Matrix4x4 sm(
		    {{{{s.x(), 0, 0, 0}}, {{0, s.y(), 0, 0}}, {{0, 0, s.z(), 0}}, {{0, 0, 0, 1}}}});
		*this = sm * (*this);
	}

	/**
	 * @brief Multiplies this matrix by another that translates coordinates by the
	 * components of the given vector
	 *
	 * @tparam V 3D vector type
	 * @param v translation vector
	 */
	template <typename V> void translate(const V &v) {
		Matrix4x4 t(
		    {{{{1, 0, 0, v.x()}}, {{0, 1, 0, v.y()}}, {{0, 0, 1, v.z()}}, {{0, 0, 0, 1}}}});
		*this = t * (*this);
	}

	/**
	 * @brief Multiplies this matrix by another that rotates coordinates by the
	 * according to the given rotation
	 *
	 * @tparam V Rotation type
	 * @param r the rotation
	 */
	template <typename R> void rotate(const R &r) {
		double squ = r.n.x() * r.n.x();
		double sqv = r.n.y() * r.n.y();
		double sqw = r.n.z() * r.n.z();
		double uv = r.n.x() * r.n.y();
		double uw = r.n.x() * r.n.z();
		double vw = r.n.y() * r.n.z();
		double costeta = cos(r.teta);
		double sinteta = sin(r.teta);

		Matrix4x4 rm(
		    {{{{squ + (1.0f - squ) * costeta, uv * (1.0f - costeta) - r.n.z() * sinteta,
		        uw * (1.0f - costeta) + r.n.y() * sinteta, 0}},
		      {{uv * (1.0f - costeta) + r.n.z() * sinteta, sqv + (1.0f - sqv) * costeta,
		        vw * (1.0f - costeta) - r.n.x() * sinteta, 0}},
		      {{uw * (1.0f - costeta) - r.n.y() * sinteta,
		        vw * (1.0f - costeta) + r.n.x() * sinteta, sqw + (1.0f - sqw) * costeta, 0}},
		      {{0, 0, 0, 1}}}});
		*this = rm * (*this);
	}

	inline Matrix4x4 operator*(const Matrix4x4 &N) {
		return Matrix4x4({{{{m[0][0] * N.m[0][0] + m[0][1] * N.m[1][0] + m[0][2] * N.m[2][0] +
		                         m[0][3] * N.m[3][0],
		                     m[0][0] * N.m[0][1] + m[0][1] * N.m[1][1] + m[0][2] * N.m[2][1] +
		                         m[0][3] * N.m[3][1],
		                     m[0][0] * N.m[0][2] + m[0][1] * N.m[1][2] + m[0][2] * N.m[2][2] +
		                         m[0][3] * N.m[3][2],
		                     m[0][0] * N.m[0][3] + m[0][1] * N.m[1][3] + m[0][2] * N.m[2][3] +
		                         m[0][3] * N.m[3][3]}},
		                   {{m[1][0] * N.m[0][0] + m[1][1] * N.m[1][0] + m[1][2] * N.m[2][0] +
		                         m[1][3] * N.m[3][0],
		                     m[1][0] * N.m[0][1] + m[1][1] * N.m[1][1] + m[1][2] * N.m[2][1] +
		                         m[1][3] * N.m[3][1],
		                     m[1][0] * N.m[0][2] + m[1][1] * N.m[1][2] + m[1][2] * N.m[2][2] +
		                         m[1][3] * N.m[3][2],
		                     m[1][0] * N.m[0][3] + m[1][1] * N.m[1][3] + m[1][2] * N.m[2][3] +
		                         m[1][3] * N.m[3][3]}},
		                   {{m[2][0] * N.m[0][0] + m[2][1] * N.m[1][0] + m[2][2] * N.m[2][0] +
		                         m[2][3] * N.m[3][0],
		                     m[2][0] * N.m[0][1] + m[2][1] * N.m[1][1] + m[2][2] * N.m[2][1] +
		                         m[2][3] * N.m[3][1],
		                     m[2][0] * N.m[0][2] + m[2][1] * N.m[1][2] + m[2][2] * N.m[2][2] +
		                         m[2][3] * N.m[3][2],
		                     m[2][0] * N.m[0][3] + m[2][1] * N.m[1][3] + m[2][2] * N.m[2][3] +
		                         m[2][3] * N.m[3][3]}},
		                   {{m[3][0] * N.m[0][0] + m[3][1] * N.m[1][0] + m[3][2] * N.m[2][0] +
		                         m[3][3] * N.m[3][0],
		                     m[3][0] * N.m[0][1] + m[3][1] * N.m[1][1] + m[3][2] * N.m[2][1] +
		                         m[3][3] * N.m[3][1],
		                     m[3][0] * N.m[0][2] + m[3][1] * N.m[1][2] + m[3][2] * N.m[2][2] +
		                         m[3][3] * N.m[3][2],
		                     m[3][0] * N.m[0][3] + m[3][1] * N.m[1][3] + m[3][2] * N.m[2][3] +
		                         m[3][3] * N.m[3][3]}}}});
	}
	template <typename V> inline V operator*(const V &v) {
		return Vec(m[0][0] * v.x() + m[0][1] * v.y() + m[0][2] * v.z() + m[0][3],
		           m[1][0] * v.x() + m[1][1] * v.y() + m[1][2] * v.z() + m[1][3],
		           m[2][0] * v.x() + m[2][1] * v.y() + m[2][2] * v.z() + m[2][3]);
	}

	inline std::array<double, 4> &operator[](const size_t index) { return m[index]; }
	inline const std::array<double, 4> &operator[](const size_t index) const {
		return m[index];
	}
	friend ostream &operator<<(ostream &, const Matrix4x4 &);
};

inline ostream &operator<<(ostream &out, const Matrix4x4 &M) {
	out << endl;
	for (auto &i : M.m) {
		out << "|";
		for (auto &j : i) {
			out << std::setw(5) << j << "|";
		}
		out << endl;
	}
	return out;
}
}  // namespace MecaCell
#endif
