#include "tools.h"
#include <sstream>

namespace MecaCell {

float_t closestDistToTriangleEdge(const Vec &v0, const Vec &v1, const Vec &v2,
                                  const Vec &p) {
	Vec a = v1 - v0;
	Vec b = v2 - v0;
	Vec c = v2 - v1;
	// on teste si la projection de p sur a est entre v1 et v0;
	Vec v0p = p - v0;
	Vec v1p = p - v1;
	Vec v2p = p - v2;
	float_t sqV0pa = v0p.dot(a);
	float_t sqV0pb = v0p.dot(b);
	float_t sqV1pc = v1p.dot(c);
	float_t adist, bdist, cdist;
	float_t v0dist, v1dist, v2dist;
	v0dist = v0p.sqlength();
	v1dist = v1p.sqlength();
	v2dist = v2p.sqlength();
	if (sqV0pa >= 0 && sqV0pa <= a.sqlength()) {
		adist = ((v0 + (sqV0pa / a.sqlength()) * a) - p).sqlength();
	} else if (sqV0pa < 0) {  // v0 is the closest
		adist = v0dist;
	} else {  // v1 is the closest
		adist = v1dist;
	}
	if (sqV0pb >= 0 && sqV0pb <= b.sqlength()) {
		bdist = ((v0 + (sqV0pb / b.sqlength()) * b) - p).sqlength();
	} else if (sqV0pb < 0) {  // v0 is the closest
		bdist = v0dist;
	} else {  // v2 is the closest
		bdist = v2dist;
	}
	if (sqV1pc >= 0 && sqV1pc <= c.sqlength()) {
		cdist = ((v1 + (sqV1pc / c.sqlength()) * c) - p).sqlength();
	} else if (sqV1pc < 0) {  // v1 is the closest
		cdist = v1dist;
	} else {  // v2 is the closest
		cdist = v2dist;
	}
	return sqrt(min(adist, min(bdist, cdist)));
}
std::pair<bool, Vec> rayInTriangle(const Vec &v0, const Vec &v1, const Vec &v2,
                                   const Vec &o, const Vec &r, const float_t tolerance) {
	Vec u = v1 - v0;
	Vec v = v2 - v0;
	Vec n = u.cross(v);
	float_t l = Vec::rayCast(v0, n, o, r);
	if (l > 0) {
		Vec p = o + l * r;
		Vec w = p - v0;
		float_t nsq = n.sqlength();
		float_t l = u.cross(w).dot(n) / nsq;
		float_t b = w.cross(v).dot(n) / nsq;
		float_t a = 1.0 - l - b;
		return {0 - tolerance <= a && a <= 1.0 + tolerance && 0 - tolerance <= b &&
		            b <= 1.0 + tolerance && 0 - tolerance <= l && l <= 1.0 + tolerance,
		        a * v0 + b * v1 + l * v2};
	} else {
		cerr << "l = " << l << endl;
		return {false, o};
	}
}

std::pair<bool, Vec> projectionIntriangle(const Vec &v0, const Vec &v1, const Vec &v2,
                                          const Vec &p, const float_t tolerance) {
	Vec u = v1 - v0;
	Vec v = v2 - v0;
	Vec n = u.cross(v);
	Vec w = p - v0;
	float_t nsq = n.sqlength();
	float_t l = u.cross(w).dot(n) / nsq;
	float_t b = w.cross(v).dot(n) / nsq;
	float_t a = 1.0 - l - b;
	return {0 - tolerance <= a && a <= 1.0 + tolerance && 0 - tolerance <= b &&
	            b <= 1.0 + tolerance && 0 - tolerance <= l && l <= 1.0 + tolerance,
	        a * v0 + b * v1 + l * v2};
}

Vec hsvToRgb(float_t h, float_t s, float_t v) {
	float_t hh, p, q, t, ff;
	long i;
	Vec out;

	if (s <= 0.0) {  // < is bogus, just shuts up warnings
		out.xRef() = v;
		out.yRef() = v;
		out.zRef() = v;
		return out;
	}
	hh = h;
	if (hh >= 360.0) hh = 0.0;
	hh /= 60.0;
	i = (long)hh;
	ff = hh - i;
	p = v * (1.0 - s);
	q = v * (1.0 - (s * ff));
	t = v * (1.0 - (s * (1.0 - ff)));

	switch (i) {
		case 0:
			out.xRef() = v;
			out.yRef() = t;
			out.zRef() = p;
			break;
		case 1:
			out.xRef() = q;
			out.yRef() = v;
			out.zRef() = p;
			break;
		case 2:
			out.xRef() = p;
			out.yRef() = v;
			out.zRef() = t;
			break;

		case 3:
			out.xRef() = p;
			out.yRef() = q;
			out.zRef() = v;
			break;
		case 4:
			out.xRef() = t;
			out.yRef() = p;
			out.zRef() = v;
			break;
		case 5:
		default:
			out.xRef() = v;
			out.yRef() = p;
			out.zRef() = q;
			break;
	}
	return out;
}
float_t dampingFromRatio(const float_t r, const float_t m, const float_t k) {
	return r * 2.0 * sqrt(m * k);  // for angular springs m is the moment of inertia
}
std::default_random_engine globalRand(std::random_device{}());

std::vector<std::string> splitStr(const std::string &s, char delim) {
	std::vector<std::string> res;
	std::stringstream ss(s);
	std::string st;
	while (std::getline(ss, st, delim)) {
		res.push_back(st);
	}
	return res;
}

float_t DEFAULT_CELL_DAMP_RATIO = 0.8;
float_t DEFAULT_CELL_MASS = 1.0;
float_t DEFAULT_CELL_RADIUS = 40.0;
float_t DEFAULT_CELL_STIFFNESS = 45.0;
float_t DEFAULT_CELL_ANG_STIFFNESS = 0.8;
float_t MIN_CELL_ADH_LENGTH = 0.6;
float_t MAX_CELL_ADH_LENGTH = 0.8;
float_t ADH_THRESHOLD = 0.1;
}
