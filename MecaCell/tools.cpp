#include "tools.h"

namespace MecaCell {

int double2int(double d) {
	d += 6755399441055744.0;
	return reinterpret_cast<int&>(d);
}

double dampingFromRatio(const double r, const double m, const double k) {
	return r * 2.0 * sqrt(m * k);  // for angular springs m is the moment of inertia
}

double DEFAULT_CELL_DAMP_RATIO = 0.1;
double DEFAULT_CELL_MASS = 1.0;
double DEFAULT_CELL_RADIUS = 40.0;
double DEFAULT_CELL_STIFFNESS = 20.0;
double DEFAULT_CELL_ANG_STIFFNESS = 100000.0;
double MIN_CELL_ADH_LENGTH = 0.5;
double MAX_CELL_ADH_LENGTH = 0.8;
double ADH_THRESHOLD = 0.1;
}
