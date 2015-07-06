#include "tools.h"
#include <sstream>

namespace MecaCell {

int double2int(double d) {
	d += 6755399441055744.0;
	return reinterpret_cast<int &>(d);
}
Vec hsvToRgb(double h, double s, double v) {
	double hh, p, q, t, ff;
	long i;
	Vec out;

	if (s <= 0.0) { // < is bogus, just shuts up warnings
		out.x = v;
		out.y = v;
		out.z = v;
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
			out.x = v;
			out.y = t;
			out.z = p;
			break;
		case 1:
			out.x = q;
			out.y = v;
			out.z = p;
			break;
		case 2:
			out.x = p;
			out.y = v;
			out.z = t;
			break;

		case 3:
			out.x = p;
			out.y = q;
			out.z = v;
			break;
		case 4:
			out.x = t;
			out.y = p;
			out.z = v;
			break;
		case 5:
		default:
			out.x = v;
			out.y = p;
			out.z = q;
			break;
	}
	return out;
}
double dampingFromRatio(const double r, const double m, const double k) {
	return r * 2.0 * sqrt(m * k); // for angular springs m is the moment of inertia
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

double DEFAULT_CELL_DAMP_RATIO = 1.0;
double DEFAULT_CELL_MASS = 1.0;
double DEFAULT_CELL_RADIUS = 40.0;
double DEFAULT_CELL_STIFFNESS = 30.0;
double DEFAULT_CELL_ANG_STIFFNESS = 70.0;
double MIN_CELL_ADH_LENGTH = 0.5;
double MAX_CELL_ADH_LENGTH = 0.8;
double ADH_THRESHOLD = 0.1;
}
