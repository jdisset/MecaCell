#define protected public
#define private public
#undef protected
#undef private
#include <iostream>
#include <list>
#include "../mecacell/mecacell.h"
#include "../mecacell/pbdbody.hpp"
#include "catch.hpp"

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

TEST_CASE("GRID") {
	const double RAD = 1;
	struct Sphere {
		MecaCell::Vec center;
		double radius = 1;
		MecaCell::Vec getPosition() { return center; }
		double getBoundingBoxRadius() { return radius; }
	};

	const double GRIDSIZE = 200;
	MecaCell::Grid<Sphere*> grid{GRIDSIZE};
	std::list<Sphere> spheres;
	for (double x = RAD; x < GRIDSIZE - RAD; x += RAD * 2.0) {
		for (double y = RAD; y < GRIDSIZE - RAD; y += RAD * 2.0) {
			spheres.emplace_back(Sphere{MecaCell::Vec(x, y, RAD)});
		}
	}
	for (auto& s : spheres) grid.insert(&s);
	REQUIRE(grid.getOrderedVec().size() == 1);
	REQUIRE(grid.getUnorderedMap().size() == 1);
	REQUIRE(grid.getOrderedVec()[0].second.size() == spheres.size());

	const double eps = 1e-10;
	grid.clear();
	REQUIRE(grid.getOrderedVec().size() == 0);
	REQUIRE(grid.getUnorderedMap().size() == 0);
	Sphere s0{
	    MecaCell::Vec(GRIDSIZE / 2.0 - eps, GRIDSIZE / 2.0 - eps, GRIDSIZE / 2.0 - eps),
	    GRIDSIZE / 2.0 - eps};
	Sphere s1{MecaCell::Vec(0, 0, 0), GRIDSIZE - eps};
	Sphere s2{MecaCell::Vec(0, 0, 0), GRIDSIZE * 4 - eps};

	grid.clear();
	grid.insert(&s0);
	REQUIRE(grid.getOrderedVec().size() == 1);
	grid.insert(&s1);
	REQUIRE(grid.getOrderedVec().size() == 2 * 2 * 2);
	grid.insert(&s2);
	REQUIRE(grid.getOrderedVec().size() == 8 * 8 * 8);
}

template <typename W> size_t getNbOverlaps(W& w) {
	const double eps = 1e-5;
	size_t nbOverlaps = 0;
	for (size_t i = 0; i < w.cells.size(); ++i) {
		for (size_t j = i + 1; j < w.cells.size(); ++j) {
			if ((w.cells[i]->getPosition() - w.cells[j]->getPosition()).length() <
			    w.cells[i]->getBody().getBoundingBoxRadius() +
			        w.cells[j]->getBody().getBoundingBoxRadius() - eps)
				++nbOverlaps;
		}
	}
	return nbOverlaps;
}
TEST_CASE("PBD") {
	struct Cell : public MecaCell::BaseCell<Cell, MecaCell::PBDBody_singleParticle> {
		using Base = MecaCell::BaseCell<Cell, MecaCell::PBDBody_singleParticle>;
		using Base::Base;  // constructor inheritance
	};

	const double GRIDSIZE = 10;
	MecaCell::World<Cell> w;
	const int n = 1500;
	auto offset = MecaCell::Vec::randomUnit() * GRIDSIZE;
	for (int i = 0; i < n; ++i) {
		w.addCell(new Cell(0.1 * MecaCell::Vec::randomUnit() + offset));
	}
	REQUIRE(w.cells.size() == 0);
	w.addNewCells();
	REQUIRE(w.cells.size() == n);
	REQUIRE(getNbOverlaps(w) == (n * (n - 1)) / 2);
	w.cellPlugin.setGridSize(GRIDSIZE);
	w.cellPlugin.reinsertCellsInGrid = true;
	w.cellPlugin.reinsertAllCellsInGrid(&w);
	// REQUIRE(w.cellPlugin.grid.getOrderedVec().size() <= 8);
	// REQUIRE(w.cellPlugin.grid.getOrderedVec()[0].second.size() == n);
	w.cellPlugin.refreshConstraints(&w);
	REQUIRE(w.cellPlugin.constraints.size<0>() == ((n * (n - 1)) / 2));
	w.cellPlugin.iterations = 20;
	for (int i = 0; i < 10; ++i) {
		w.cellPlugin.solveConstraints(&w);
		w.cellPlugin.updateParticles(&w);
		for (auto& c : w.cells) c->getBody().setVelocity(MecaCell::Vec::zero());
		// w.cellPlugin.reinsertAllCellsInGrid(&w);
		w.cellPlugin.updateGrid(&w);
	}
	REQUIRE(getNbOverlaps(w) == 0);
}
