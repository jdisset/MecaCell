#include "../mecacell/mecacell.h"
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one
                           // cpp file
#include "catch.hpp"

using namespace MecaCell;

bool doubleEq(double a, double b) { return abs(a - b) < 0.00000000001; }

TEST_CASE("Vectors & trigo") {
	Vec a(-10, -5, 2);
	Vec b(10, -5, 2);
	Vec c(0, 15, 2);
	Vec d(0.002, -12345668948937.1234, 19283.2);
	Vec e(20000000000000000, -168948937.134, 0);
	Vec f(0, 0, 0);
	Vec g(0, 1, 0);

	// basic operations
	REQUIRE(Vec(f *= 3) == f);
	REQUIRE((g - g + 2 * g - 2 * g / 1) == f);

	// length & normalization
	REQUIRE(g.length() == g.sqlength());
	REQUIRE(g.normalized() == g);
	REQUIRE(g.sqlength() == 1.0);
	REQUIRE(a.sqlength() == 129.0);

	// dot, ortho, cross
	REQUIRE(a.dot(a) == a.sqlength());
	REQUIRE(b.dot(b) == b.sqlength());
	REQUIRE(c.dot(c) == c.sqlength());
	REQUIRE(d.dot(d.ortho()) == 0);
	REQUIRE(e.dot(e.ortho()) == 0);
	REQUIRE(f.dot(f.ortho()) == 0);
	REQUIRE(d.dot(e.cross(f)) == 0);
	REQUIRE(b.dot(b.cross(c)) == 0);

	// RayCast
	REQUIRE(doubleEq(Vec::rayCast(f, g, g, -g), 1.0));
	REQUIRE(doubleEq(Vec::rayCast(e, g, e + g, -g), 1.0));
	REQUIRE(Vec::rayCast(e, g, e + g, g) < 0);
	REQUIRE(Vec::rayCast(e, g, e + g, Vec(1, 0, 0)) == 0);

	// closestDistToTriangleEdge
	REQUIRE(closestDistToTriangleEdge(a, b, c, Vec(0, 0, 2)) == 5);
	REQUIRE(closestDistToTriangleEdge(a, b, c, Vec(0, -5, 2)) == 0);
	REQUIRE(closestDistToTriangleEdge(a, b, c, Vec(-10, -5, 2)) == 0);
	REQUIRE(closestDistToTriangleEdge(a, b, c, Vec(-11, -5, 2)) == 1);
	REQUIRE(doubleEq(closestDistToTriangleEdge(a, b, c, Vec(-7, -6.3, 2)), 1.3));
	REQUIRE(doubleEq(closestDistToTriangleEdge(a, b, c, Vec(-7, -6.3, 3)),
	                 sqrt(1.0 + 1.3 * 1.3)));
}
