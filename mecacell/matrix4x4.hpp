#ifndef MATRIX4X4_HPP
#define MATRIX4X4_HPP
#include <array>
using std::array;
class Matrix4x4 {
	array<array<double, 4>, 4> m = {{{{1, 0, 0, 0}}, {{0, 1, 0, 0}}, {{0, 0, 1, 0}}, {{0, 0, 0, 1}}}};
};
#endif
