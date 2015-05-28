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

/*************************
 *      BASIC SCENARIO
 ************************/
// A scenario must contain (at least)
//  - a world and its getter getWorld(). Note : the getter must return a reference.
//  - an init() method
//  - a loop() method
class BasicScenario {
	BasicWorld<BasicCell, VerletEuler> world;
	const int N = 50;

 public:
	// Init will be called before the first paint call & after each reset
	// Non mecacell-specific command line arguments are forwarded to this method.
	// In this example we just create a few cells
	void init(int, char**) {
		std::default_random_engine globalRand(0);
		std::uniform_real_distribution<double> dist(0, 300);
		for (int i = 0; i < N; ++i) world.addCell(new BasicCell(Vec::randomUnit() * dist(globalRand)));
	}

	// loop() is called before drawing a frame.
	// This is where your scenario specific events should be coded.
	// In this example we just call the world's update method
	void loop() { world.update(); }

	const BasicWorld<BasicCell, VerletEuler>& getWorld() { return world; }
};

/*************************
 *        MAIN
 ************************/
int main(int argc, char** argv) {
	// Viewer instanciation (with our scenario as a template argument)
	QtViewer<BasicScenario> viewer;
	// & execution (requires the command line argument to be forwarded)
	return viewer.exec(argc, argv);
}
