#ifndef MECACELL_MATRIX4X4_HPP
#define MECACELL_MATRIX4X4_HPP
#include <array>
#include "../utilities/utils.h"
#include "rotation.h"

namespace MecaCell {

struct Matrix4x4 {
	std::array<std::array<double, 4>, 4> m = {
	    {{{1, 0, 0, 0}}, {{0, 1, 0, 0}}, {{0, 0, 1, 0}}, {{0, 0, 0, 1}}}};
	Matrix4x4() {}
	Matrix4x4(std::array<std::array<double, 4>, 4> a) : m(a) {}
	void scale(const Vec &s);
	void translate(const Vec &t);
	void rotate(const Rotation<Vec> &r);
	Matrix4x4 operator*(const Matrix4x4 &mm);
	Vec operator*(const Vec &);
	std::array<double, 4> &operator[](const size_t index);
	const std::array<double, 4> &operator[](const size_t index) const;
	friend ostream &operator<<(ostream &, const Matrix4x4 &);
};
}
#endif
