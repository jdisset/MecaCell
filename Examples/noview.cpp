#include "mecacell.h"
#include "basiccell.hpp"
#include <iostream>
#include <random>

using namespace MecaCell;
using namespace std;

int main(int, char**) {
	unsigned int nbCells = 100;
	unsigned int nbSteps = 100;
	double clusterRadius = 1000;

	uniform_real_distribution<double> doubleDist(0.0, 1.0);
	default_random_engine randEngine(time(NULL));

	cout << " -- Creating world ";
	BasicWorld<BasicCell,VerletEuler> world;
	cout << " [OK]" << endl;

	cout << " -- Adding " << nbCells << " cells ";
	for (unsigned int i = 0; i < nbCells; ++i) {
		Vec dir = Vec::randomUnit();
		world.addCell(new BasicCell(dir * clusterRadius * doubleDist(randEngine)));
	}
	cout << " [OK]" << endl;

	auto t0 = chrono::system_clock::now();

	for (unsigned int i = 0; i < nbSteps; ++i) {
		world.update();
	}

	auto t1 = chrono::system_clock::now();
	chrono::duration<float> t = t1 - t0;

	cout << nbSteps << " updates in " << t.count() << "s (" << (double)nbSteps / t.count() << " fps)" << endl;

	return 0;
}
