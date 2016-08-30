#include <random>
#include <sstream>
#include <vector>
#include "utils.h"

namespace MecaCell {
std::default_random_engine globalRand(std::random_device{}());

std::vector<std::string> splitStr(const std::string &s, char delim) {
	std::vector<std::string> res;
	std::stringstream ss(s);
	std::string st;
	while (std::getline(ss, st, delim)) res.push_back(st);
	return res;
}

std::array<double, 3> hsvToRgb(double h, double s, double v) {
	if (s <= 0.0) return {{v, v, v}};
	double hh = h;
	if (hh >= 360.0) hh = 0.0;
	hh /= 60.0;
	unsigned int i = static_cast<unsigned int>(hh);
	double ff = hh - static_cast<double>(i);
	double p = v * (1.0 - s);
	double q = v * (1.0 - (s * ff));
	double t = v * (1.0 - (s * (1.0 - ff)));
	switch (i) {
		case 0:
			return {{v, t, p}};
		case 1:
			return {{q, v, p}};
		case 2:
			return {{p, v, t}};
		case 3:
			return {{p, q, v}};
		case 4:
			return {{t, p, v}};
		case 5:
		default:
			return {{v, p, q}};
	}
}
}
