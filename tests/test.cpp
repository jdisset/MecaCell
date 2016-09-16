#include "catch.hpp"
#define protected public
#define private public
#include "../mecacell/mecacell.h"
#undef protected
#undef private
#include <iostream>

using namespace MecaCell;

bool doubleEq(double a, double b) { return abs(a - b) < 0.000000001; }

class VolCell : public MecaCell::ConnectableCell<VolCell, ContactSurfaceBody> {
 public:
	using Base = MecaCell::ConnectableCell<VolCell, ContactSurfaceBody>;
	using Base::Base;
	double getAdhesionWith(const VolCell*, const MecaCell::Vec&) const { return 0.0; }
	template <typename W> void updateBehavior(W&) {}
};

template <typename W> void checkThatWorldssAreIdentical(W& w0, W& w1) {
	for (size_t c = 0; c < w0.cells.size(); ++c) {
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

template <typename W> void printCells(W& w) {
	std::cerr << " ---- (update " << w.getNbUpdates() << ") ---- " << std::endl;
	std::cerr << "     " << w.cells.size() << " cells:" << std::endl;
	for (auto& c : w.cells) {
		std::cerr << YELLOW << "cell " << c->id << RESET << " ( adr = " << c
		          << "):" << std::endl;
		std::cerr << " -- connectedCells:" << std::endl;
		for (auto& co : c->connectedCells)
			std::cerr << "    |> " << BOLDBLUE << co->id << RESET << " (" << co << ")"
			          << std::endl;
		std::cerr << " -- cellConnections:" << std::endl;
		for (auto& co : c->membrane.cccm.cellConnections) {
			std::cerr << "    |> Connection " << co << " btwn " << co->c0->id << " & "
			          << co->c1->id << std::endl;
		}
	}
	std::cerr << BOLDYELLOW << ">>> World stored connections ("
	          << w.cellCellConnections.size() << "): " << RESET << std::endl;
	for (auto& co : w.cellCellConnections) {
		std::cerr << "    |> (" << co.second.get() << ") between cells " << BOLDBLUE
		          << co.second->c0->id << " & " << co.second->c1->id << RESET << std::endl;
	}
}

TEST_CASE("World creation, cell additions & deletion") {
	MecaCell::World<VolCell> w;
	REQUIRE(w.getNbUpdates() == 0);
	w.update();
	REQUIRE(w.cells.size() == 0);
	REQUIRE(w.getNbUpdates() == 1);
	w.addCell(new VolCell());
	w.update();
	REQUIRE(w.cells.size() == 1);
	REQUIRE(w.cells[0]->getPosition() == MecaCell::Vector3D(0, 0, 0));
	w.update();
	REQUIRE(w.cells[0]->getPosition() == MecaCell::Vector3D(0, 0, 0));
	Vector3D secondCellPos(50, 0, 0);
	w.addCell(new VolCell(secondCellPos));
	w.update();
	REQUIRE(w.cells.size() == 2);
	REQUIRE(w.cells[1]->getPosition() == secondCellPos);
}

// TEST_CASE("Cells update determinism") {
// using Cell = VolCell;
// using World = MecaCell::BasicWorld<Cell>;
// for (int n = 0; n < 15; ++n) {
// World w0, w1, w2;
// const int nbC = 8;
// for (int i = 0; i < nbC; ++i) {
// auto pos = MecaCell::Vec::randomUnit() * 50.0;
// w0.addCell(new Cell(pos));
// w1.addCell(new Cell(pos));
// w2.addCell(new Cell(pos));
//}
// REQUIRE(w0.cells.size() == nbC);
// REQUIRE(w0.cells.size() == w1.cells.size());
// REQUIRE(w1.cells.size() == w2.cells.size());
// for (int l = 0; l < 300; ++l) {
// w0.update();
// w1.update();
// w2.update();
// checkThatCellsAreIdentical(w0, w1);
// checkThatCellsAreIdentical(w1, w2);
//}
// w0.cells[1]->die();
// w1.cells[1]->die();
// w2.cells[1]->die();
// for (int l = 0; l < 500; ++l) {
// w0.update();
// w1.update();
// w2.update();
// checkThatCellsAreIdentical(w0, w1);
// checkThatCellsAreIdentical(w1, w2);
//}
// REQUIRE(w0.cells.size() == nbC - 1);
// REQUIRE(w0.cells.size() == w1.cells.size());
// REQUIRE(w1.cells.size() == w2.cells.size());
//}
//}
