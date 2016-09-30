#ifndef MECACELL_GEOMETRY_H
#define MECACELL_GEOMETRY_H
#include "../utilities/utils.h"
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
std::pair<bool, Vec> rayInTriangle(const Vec &v0, const Vec &v1, const Vec &v2,
                                   const Vec &o, const Vec &r,
                                   const double tolerance = 0);

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
std::pair<bool, Vec> projectionIntriangle(const Vec &v0, const Vec &v1, const Vec &v2,
                                          const Vec &p, const double tolerance = 0);

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
double closestDistToTriangleEdge(const Vec &v0, const Vec &v1, const Vec &v2,
                                 const Vec &p);

/**
 * @brief returns the positions of n points uniformly distributed on the surface of the
 * unit sphere
 *
 * @param n number of points
 *
 * @return vector of positions
 */
std::vector<Vec> getSpherePointsPacking(unsigned int n);

std::pair<double, double> updateElectrostaticPointsOnSphere(vector<Vec> &p, double dt);
}
#endif
