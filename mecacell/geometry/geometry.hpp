#ifndef MECACELL_GEOMETRY_H
#define MECACELL_GEOMETRY_H
#include <vector>
#include "../utilities/utils.hpp"
namespace MecaCell {

/**
 * @brief test if a ray hits a triangle
 *
 * @param v0 triangle vertex 0
 * @param v1 triangle vertex 1
 * @param v2 triangle vertex 2
 * @param o origin of ray
 * @param r direction of ray
 * @param tolerance
 *
 * @return a pair containing a boolean (true if the triangle was hit) and the hit position
 */
std::pair<bool, Vec> inline rayInTriangle(const Vec &v0, const Vec &v1, const Vec &v2,
                                          const Vec &o, const Vec &r,
                                          const double tolerance = 0) {
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

/**
 * @brief tests if the projection of a point along the normal of a triangle is inside said
 * triangle
 *
 * @param v0 triangle vertex 0
 * @param v1 triangle vertex 1
 * @param v2 triangle vertex 2
 * @param p point to be projected
 * @param tolerance
 *
 * @return a pair containing a boolean (true if the projection is inside the triangle) and
 * the projection position
 */
std::pair<bool, Vec> inline projectionIntriangle(const Vec &v0, const Vec &v1,
                                                 const Vec &v2, const Vec &p,
                                                 const double tolerance = 0) {
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

/**
 * @brief computes the smallest distance to a triangle edge from a given point
 *
 * @param v0 triangle vertex 0
 * @param v1 triangle vertex 1
 * @param v2 triangle vertex 2
 * @param p considered point
 *
 * @return the smallest distance to an edge
 */
double inline closestDistToTriangleEdge(const Vec &v0, const Vec &v1, const Vec &v2,
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



}  // namespace MecaCell
#endif
