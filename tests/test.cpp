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

class SpCell : public MecaCell::ConnectableCell<SpCell, VolumeMembrane> {
 public:
	using Base = MecaCell::ConnectableCell<SpCell, VolumeMembrane>;
	using Base::Base;
	double getAdhesionWith(const SpCell*, const MecaCell::Vec&) const { return 0.0; }
	SpCell* updateBehavior(double) { return nullptr; }
};

class VolCell : public MecaCell::ConnectableCell<VolCell, VolumeMembrane> {
 public:
	using Base = MecaCell::ConnectableCell<VolCell, VolumeMembrane>;
	using Base::Base;
	double getAdhesionWith(const VolCell*, const MecaCell::Vec&) const { return 0.0; }
	VolCell* updateBehavior(double) { return nullptr; }
};

template <typename W> void checkThatCellsAreIdentical(W& w0, W& w1) {
	for (int c = 0; c < w0.cells.size(); ++c) {
		REQUIRE(w0.cells[c]->id == w1.cells[c]->id);
		REQUIRE(w0.cells[c]->getPosition() == w1.cells[c]->getPosition());
		REQUIRE(w0.cells[c]->getVelocity() == w1.cells[c]->getVelocity());
		REQUIRE(w0.cells[c]->getForce() == w1.cells[c]->getForce());
		REQUIRE(w0.cells[c]->getExternalForces() == w1.cells[c]->getExternalForces());
		REQUIRE(w0.cells[c]->getConnectedCells().size() ==
		        w1.cells[c]->getConnectedCells().size());
		auto connected = w0.cells[c]->getConnectedCells();
		for (const auto& other : connected) {
			bool sameConnections = false;
			for (auto& other1 : w1.cells[c]->getConnectedCells()) {
				if (other1->id == other->id) {
					sameConnections = true;
					break;
				}
			}
			REQUIRE(sameConnections);
		}
	}
}

TEST_CASE("cells update are deterministic") {
	using Cell = VolCell;
	using World = MecaCell::BasicWorld<Cell>;
	for (int n = 0; n < 10; ++n) {
		World w0, w1, w2;
		const int nbC = 10;
		for (int i = 0; i < nbC; ++i) {
			auto pos = MecaCell::Vec::randomUnit() * 50.0;
			w0.addCell(new Cell(pos));
			w1.addCell(new Cell(pos));
			w2.addCell(new Cell(pos));
		}
		REQUIRE(w0.cells.size() == nbC);
		REQUIRE(w0.cells.size() == w1.cells.size());
		REQUIRE(w1.cells.size() == w2.cells.size());
		for (int l = 0; l < 700; ++l) {
			w0.update();
			w1.update();
			w2.update();
			checkThatCellsAreIdentical(w0, w1);
			checkThatCellsAreIdentical(w1, w2);
		}
	}
}
