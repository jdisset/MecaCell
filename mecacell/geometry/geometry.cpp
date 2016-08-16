#include <vector>
#include "geometry.h"
namespace MecaCell {
std::vector<Vec> getSpherePointsPacking(unsigned int n) {
	// we simulate n mutually repulsive points on a sphere
	std::vector<Vec> p;
	double prevminl = 0;
	double avgDelta = 1;
	double prevAvgDelta = 1;
	double dt = 2.0 / sqrt(static_cast<double>(n));
	p.reserve(n);
	double maxD = sqrt((4.0 * M_PI / static_cast<double>(n)) / M_PI);

	// init with golden spiral
	double inc = M_PI * (3.0 - sqrt(5.0));
	double off = 2.0 / (double)n;
	for (unsigned int i = 0; i < n; ++i) {
		double y = i * off - 1.0 + (off * 0.5);
		double r = sqrt(1.0 - y * y);
		double phi = i * inc;
		p.push_back(Vec(cos(phi) * r, y, sin(phi) * r).normalized());
	}

	// then perfect with a few electrostatic repulsion iterations
	int cpt = 0;
	double prevExactDelta = 0;
	double exactDelta = 0;
	do {
		++cpt;
		auto minlAndNewDt = updateElectrostaticPointsOnSphere(p, dt);
		double minl = minlAndNewDt.first;
		dt = minlAndNewDt.second;
		prevExactDelta = exactDelta;
		exactDelta = minl - prevminl;
		if (exactDelta < 0) {
			dt *= 0.8;
		}
		avgDelta = lerp(prevAvgDelta, exactDelta, 0.7);
		prevAvgDelta = avgDelta;
		prevminl = minl;
	} while (cpt < 10 || ((prevExactDelta > 0 || exactDelta > 0) && dt > 0.000001 &&
	                      cpt < 200 && abs(avgDelta) / dt > 0.001 * maxD));
	return p;
}

std::pair<double, double> updateElectrostaticPointsOnSphere(vector<Vec> &p,
                                                              double dt) {
	double maxD = sqrt((4.0 * M_PI / static_cast<double>(p.size())) / M_PI) * 0.3;
	double maxF = 0;
	vector<Vec> f(p.size());
	for (size_t i = 0; i < p.size(); ++i) {
		Vec force;
		for (size_t j = 0; j < p.size(); ++j) {
			if (i != j) {
				Vec unprojected = p[i] - p[j];
				double sql = unprojected.sqlength();
				if (sql != 0) {
					unprojected /= sql;
					force += (unprojected - unprojected.dot(p[i]) * p[i]);
				}
			}
		}
		double fIntensity = force.sqlength();
		if (fIntensity > maxF) maxF = fIntensity;
		f[i] = force;
	}
	maxF = sqrt(maxF);
	if (maxF * dt > maxD) {
		dt *= (maxD / (maxF * dt));
	}
	double totalDisplacement = 0;
	// now we update the position of each point;
	for (size_t i = 1; i < p.size(); ++i) {
		Vec pprev = p[i];
		p[i] += f[i] * dt;
		p[i].normalize();
		totalDisplacement += (pprev - p[i]).length();
	}
	double minminsql = 10000;
	for (size_t i = 0; i < p.size(); ++i) {
		double minsql = 10000;
		for (size_t j = i + 1; j < p.size(); ++j) {
			double sql = (p[i] - p[j]).sqlength();
			if (sql < minsql) minsql = sql;
		}
		if (minsql < minminsql) minminsql = minsql;
	}
	return make_pair(sqrt(minminsql), dt);
}

double closestDistToTriangleEdge(const Vec &v0, const Vec &v1, const Vec &v2,
                                  const Vec &p) {
	Vec a = v1 - v0;
	Vec b = v2 - v0;
	Vec c = v2 - v1;
	// on teste si la projection de p sur a est entre v1 et v0;
	Vec v0p = p - v0;
	Vec v1p = p - v1;
	Vec v2p = p - v2;
	double sqV0pa = v0p.dot(a);
	double sqV0pb = v0p.dot(b);
	double sqV1pc = v1p.dot(c);
	double adist, bdist, cdist;
	double v0dist, v1dist, v2dist;
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
                                   const Vec &o, const Vec &r, const double tolerance) {
	Vec u = v1 - v0;
	Vec v = v2 - v0;
	Vec n = u.cross(v);
	double l = Vec::rayCast(v0, n, o, r);
	if (l > 0) {
		Vec p = o + l * r;
		Vec w = p - v0;
		double nsq = n.sqlength();
		l = u.cross(w).dot(n) / nsq;
		double b = w.cross(v).dot(n) / nsq;
		double a = 1.0 - l - b;
		return {0 - tolerance <= a && a <= 1.0 + tolerance && 0 - tolerance <= b &&
		            b <= 1.0 + tolerance && 0 - tolerance <= l && l <= 1.0 + tolerance,
		        a * v0 + b * v1 + l * v2};
	}
	return {false, o};
}

std::pair<bool, Vec> projectionIntriangle(const Vec &v0, const Vec &v1, const Vec &v2,
                                          const Vec &p, const double tolerance) {
	Vec u = v1 - v0;
	Vec v = v2 - v0;
	Vec n = u.cross(v);
	Vec w = p - v0;
	double nsq = n.sqlength();
	double l = u.cross(w).dot(n) / nsq;
	double b = w.cross(v).dot(n) / nsq;
	double a = 1.0 - l - b;
	return {0 - tolerance <= a && a <= 1.0 + tolerance && 0 - tolerance <= b &&
	            b <= 1.0 + tolerance && 0 - tolerance <= l && l <= 1.0 + tolerance,
	        a * v0 + b * v1 + l * v2};
}
}
