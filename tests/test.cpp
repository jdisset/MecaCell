#include "../mecacell/mecacell.h"
#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

using namespace MecaCell;

bool doubleEq(double a, double b) { return abs(a - b) < 0.00000000001; }

TEST_CASE("Trigo, vectors & rotations") {
	Vec a(-10, -5, 2);
	Vec b(10, -5, 2);
	Vec c(0, 15, 2);

	// inTriangle

	// closestDistToTriangleEdge
	REQUIRE(closestDistToTriangleEdge(a, b, c, Vec(0, 0, 2)) == 5);
	REQUIRE(closestDistToTriangleEdge(a, b, c, Vec(0, -5, 2)) == 0);
	REQUIRE(closestDistToTriangleEdge(a, b, c, Vec(-10, -5, 2)) == 0);
	REQUIRE(closestDistToTriangleEdge(a, b, c, Vec(-11, -5, 2)) == 1);
	REQUIRE(doubleEq(closestDistToTriangleEdge(a, b, c, Vec(-7, -6.3, 2)), 1.3));
	REQUIRE(doubleEq(closestDistToTriangleEdge(a, b, c, Vec(-7, -6.3, 3)), sqrt(1.0 + 1.3 * 1.3)));
}
