#ifndef GEOMETRY_H
#define GEOMETRY_H
#include "../utilities/utils.h"
namespace MecaCell {

// triangle ray & point collisions helpers
std::pair<bool, Vec> rayInTriangle(const Vec &v0, const Vec &v1, const Vec &v2,
                                   const Vec &o, const Vec &r, const double tolerance);
double closestDistToTriangleEdge(const Vec &, const Vec &, const Vec &, const Vec &);
std::pair<bool, Vec> projectionIntriangle(const Vec &v0, const Vec &v1, const Vec &v2,
                                          const Vec &p, const double tolerance);

// getSpherePointsPacking(n) returns the positions of n points uniformly distributed on
// the surface of a unit sphere
std::vector<Vec> getSpherePointsPacking(unsigned int n);
std::pair<double, double> updateElectrostaticPointsOnSphere(vector<Vec> &p, double dt);
}
#endif
