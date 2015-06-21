#include "qtviewer.hpp"
#include "../MecaCell/mecacell.h"
#include "../MecaCell/basiccell.hpp"
#include "../MecaCell/basicworld.hpp"
#include <random>

/*****************************************************************
 *                 Minimalistic viewer exemple
 *****************************************************************
 In this example we set up a simple scenario and start the viewer.
 */

using namespace MecaCell;
using namespace MecaCellViewer;
using World = BasicWorld<BasicCell, Euler>;
/*************************
 *      BASIC SCENARIO
 ************************/
// A scenario must contain (at least)
//  - a world and its getter getWorld(). Note : the getter must return a reference.
//  - an init() method
//  - a loop() method
class BasicScenario {
	World world;
	const int N = 100;

public:
	// Init will be called before the first paint call & after each reset
	// Non mecacell-specific command line arguments are forwarded to this method.
	// In this example we just create a few cells
	void init(int, char **) {
		std::default_random_engine globalRand(0);
		std::uniform_real_distribution<double> dist(0, 1000);
		std::uniform_real_distribution<double> dist2(0, 100);
		double ecart = 40.0;
		// BasicCell *c0 = new BasicCell(Vec(0, 0, 0));
		////c0->setVelocity(Vec(0,50,0));
		// world.addCell(c0);
		// world.addCell(new BasicCell(Vec(-60, 0, 0)));
		// world.addCell(new BasicCell(Vec(60, 0, 0)));
		for (int i = 0; i < N; ++i) {
			BasicCell *c = new BasicCell(Vec::randomUnit() * dist(globalRand));
			c->setVelocity(Vec::randomUnit() * dist2(globalRand));
			world.addCell(c);
		}
	}

	// loop() is called before drawing a frame.
	// This is where your scenario specific events should be coded.
	// In this example we just call the world's update method
	void loop() { world.update(); }

	World &getWorld() { return world; }
};

/*************************
 *        MAIN
 ************************/
int main(int argc, char **argv) {
	// Viewer instanciation (with our scenario as a template argument)
	QtViewer<BasicScenario> viewer;
	// & execution (requires the command line argument to be forwarded)
	return viewer.exec(argc, argv);
}
