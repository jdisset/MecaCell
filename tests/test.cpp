#include "catch.hpp"
#include "../mecacell/mecacell.h"
#include "../mecacell/logger.hpp"
#include "../mecacell/tools.h"
#include <iostream>
using namespace MecaCell;

bool doubleEq(double a, double b) { return abs(a - b) < 0.000000001; }

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

class Cell1 : public MecaCell::ConnectableCell<Cell1, SphereMembrane> {
 public:
	using Base = MecaCell::ConnectableCell<Cell1, SphereMembrane>;
	using Base::Base;
	double getAdhesionWith(const Cell1*) const { return 0.8; }
	Cell1* updateBehavior(double) { return nullptr; }
};

// class Cell2 : public MecaCell::ConnectableCell<Cell2, VectorSphereMembrane> {
// public:
// using Base2 = MecaCell::ConnectableCell<Cell2, VectorSphereMembrane>;
// using Base2::Base2;
// double getAdhesionWith(const Cell2*) const { return 0.8; }
// Cell2* updateBehavior(double) { return nullptr; }
//};

template <typename CellType>
void testAdhesionAndCopyBetween2Cells(Vec initialPosition, Vec direction) {
	Vec dir = direction.normalized();
	CellType c(initialPosition);
	// checking construction && membrane
	REQUIRE(c.getPosition() == initialPosition);
	REQUIRE(c.getMembraneDistance(initialPosition) == c.getBoundingBoxRadius());
	REQUIRE(c.getMembrane().getCell() == &c);
	c.getMembrane().setStiffness(c.getMembrane().getStiffness() * 1.1);
	c.getMembrane().setAngularStiffness(c.getMembrane().getAngularStiffness() * 1.1);
	c.getMembrane().setBaseRadius(c.getMembrane().getRadius() * 1.1);
	double r = c.getBoundingBoxRadius();
	CellType c2(c, r * dir);
	// checking copy constructor
	REQUIRE(
	    doubleEq((c2.getPosition() - c.getPosition()).dot(r * dir), (r * dir).sqlength()));
	REQUIRE(c2.getMembrane().getCell() == &c2);
	REQUIRE(c2.getMembrane().getBaseRadius() == c.getMembrane().getBaseRadius());
	REQUIRE(c2.getMembrane().getRadius() == c.getMembrane().getRadius());
	REQUIRE(c2.getMembrane().getCorrectedRadius() == c.getMembrane().getCorrectedRadius());
	REQUIRE(c2.getMembrane().getStiffness() == c.getMembrane().getStiffness());
	REQUIRE(c2.getMembrane().getDampRatio() == c.getMembrane().getDampRatio());
	REQUIRE(c2.getMembrane().getAngularStiffness() ==
	        c.getMembrane().getAngularStiffness());
	vector<CellType*> cells;
	cells.push_back(&c);
	cells.push_back(&c2);
	typename CellType::CellCellConnectionContainer connections;
	Grid<CellType*> cellGrid(200);
	cellGrid.clear();
	cellGrid.insert(&c);
	cellGrid.insert(&c2);
	// TODO: test grid;
	CellType::checkForCellCellConnections(cells, connections, cellGrid);
	REQUIRE(connections.size() == 1);
	REQUIRE(c.getConnectedCells().size() == 1);
	REQUIRE(c2.getConnectedCells().size() == 1);
	REQUIRE(*(c.getConnectedCells().begin()) == &c2);
	REQUIRE(*(c2.getConnectedCells().begin()) == &c);
	REQUIRE(c.getMembraneDistance(-dir) == c.getBoundingBoxRadius());
	REQUIRE(c2.getMembraneDistance(dir) == c2.getBoundingBoxRadius());
	REQUIRE(c.getMembraneDistance(dir.ortho()) == c.getBoundingBoxRadius());
	REQUIRE(c2.getMembraneDistance(dir.ortho()) == c2.getBoundingBoxRadius());
	REQUIRE(c.getMembraneDistance(dir) < c.getBoundingBoxRadius());
	REQUIRE(c2.getMembraneDistance(-dir) < c2.getBoundingBoxRadius());
	REQUIRE(c.getMembraneDistance(dir) == c2.getMembraneDistance(-dir));
	//REQUIRE(c.getMembrane().getConnectedCell(dir) == &c2);
	//REQUIRE(c2.getMembrane().getConnectedCell(-dir) == &c);
}

TEST_CASE("SphereMembrane") {
	testAdhesionAndCopyBetween2Cells<Cell1>(Vec(0, 0, 0), Vec(1, 0, 0));
	testAdhesionAndCopyBetween2Cells<Cell1>(Vec(-10, -340, 0), Vec(-1, 1, 0));
	testAdhesionAndCopyBetween2Cells<Cell1>(
	    Vec(-12314240, 0.0000000000000001, 1234823483.2342342342),
	    Vec(23131.4, -231231230, 12222.12342343420));
	// testAdhesionAndCopyBetween2Cells<Cell2>(Vec(0, 0, 0), Vec(1, 0, 0));
	// testAdhesionAndCopyBetween2Cells<Cell2>(Vec(-10, -340, 0), Vec(-1, 1, 0));
	// testAdhesionAndCopyBetween2Cells<Cell2>(
	// Vec(-12314240, 0.0000000000000001, 1234823483.2342342342),
	// Vec(23131.4, -231231230, 12222.12342343420));
}
