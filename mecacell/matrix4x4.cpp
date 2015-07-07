#include "matrix4x4.hpp"
#include <iomanip>

namespace MecaCell {
void Matrix4x4::scale(const Vec &s) {
	Matrix4x4 sm({{{{s.x, 0, 0, 0}}, {{0, s.y, 0, 0}}, {{0, 0, s.z, 0}}, {{0, 0, 0, 1}}}});
	*this = sm * (*this);
}
void Matrix4x4::translate(const Vec &v) {
	Matrix4x4 t({{{{1, 0, 0, v.x}}, {{0, 1, 0, v.y}}, {{0, 0, 1, v.z}}, {{0, 0, 0, 1}}}});
	*this = t * (*this);
}

void Matrix4x4::rotate(const Rotation<Vec> &r) {
	double squ = r.n.x * r.n.x;
	double sqv = r.n.y * r.n.y;
	double sqw = r.n.z * r.n.z;
	double uv = r.n.x * r.n.y;
	double uw = r.n.x * r.n.z;
	double vw = r.n.y * r.n.z;
	double costeta = cos(r.teta);
	double sinteta = sin(r.teta);

	Matrix4x4 rm({{{{squ + (1.0 - squ) * costeta, uv * (1.0 - costeta) - r.n.z * sinteta,
	                 uw * (1.0 - costeta) + r.n.y * sinteta, 0}},
	               {{uv * (1.0 - costeta) + r.n.z * sinteta, sqv + (1.0 - sqv) * costeta,
	                 vw * (1.0 - costeta) - r.n.x * sinteta, 0}},
	               {{uw * (1.0 - costeta) - r.n.y * sinteta, vw * (1.0 - costeta) + r.n.x * sinteta,
	                 sqw + (1.0 - sqw) * costeta, 0}},
	               {{0, 0, 0, 1}}}});
	*this = rm * (*this);
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4 &N) {
	return Matrix4x4(
	    {{{{m[0][0] * N.m[0][0] + m[0][1] * N.m[1][0] + m[0][2] * N.m[2][0] + m[0][3] * N.m[3][0],
	        m[0][0] * N.m[0][1] + m[0][1] * N.m[1][1] + m[0][2] * N.m[2][1] + m[0][3] * N.m[3][1],
	        m[0][0] * N.m[0][2] + m[0][1] * N.m[1][2] + m[0][2] * N.m[2][2] + m[0][3] * N.m[3][2],
	        m[0][0] * N.m[0][3] + m[0][1] * N.m[1][3] + m[0][2] * N.m[2][3] + m[0][3] * N.m[3][3]}},
	      {{m[1][0] * N.m[0][0] + m[1][1] * N.m[1][0] + m[1][2] * N.m[2][0] + m[1][3] * N.m[3][0],
	        m[1][0] * N.m[0][1] + m[1][1] * N.m[1][1] + m[1][2] * N.m[2][1] + m[1][3] * N.m[3][1],
	        m[1][0] * N.m[0][2] + m[1][1] * N.m[1][2] + m[1][2] * N.m[2][2] + m[1][3] * N.m[3][2],
	        m[1][0] * N.m[0][3] + m[1][1] * N.m[1][3] + m[1][2] * N.m[2][3] + m[1][3] * N.m[3][3]}},
	      {{m[2][0] * N.m[0][0] + m[2][1] * N.m[1][0] + m[2][2] * N.m[2][0] + m[2][3] * N.m[3][0],
	        m[2][0] * N.m[0][1] + m[2][1] * N.m[1][1] + m[2][2] * N.m[2][1] + m[2][3] * N.m[3][1],
	        m[2][0] * N.m[0][2] + m[2][1] * N.m[1][2] + m[2][2] * N.m[2][2] + m[2][3] * N.m[3][2],
	        m[2][0] * N.m[0][3] + m[2][1] * N.m[1][3] + m[2][2] * N.m[2][3] + m[2][3] * N.m[3][3]}},
	      {{m[3][0] * N.m[0][0] + m[3][1] * N.m[1][0] + m[3][2] * N.m[2][0] + m[3][3] * N.m[3][0],
	        m[3][0] * N.m[0][1] + m[3][1] * N.m[1][1] + m[3][2] * N.m[2][1] + m[3][3] * N.m[3][1],
	        m[3][0] * N.m[0][2] + m[3][1] * N.m[1][2] + m[3][2] * N.m[2][2] + m[3][3] * N.m[3][2],
	        m[3][0] * N.m[0][3] + m[3][1] * N.m[1][3] + m[3][2] * N.m[2][3] + m[3][3] * N.m[3][3]}}}});
}
Vec Matrix4x4::operator*(const Vec &v) {
	return Vec(m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3],
	           m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3],
	           m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3]);
}
ostream &operator<<(ostream &out, const Matrix4x4 &M) {
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
}
