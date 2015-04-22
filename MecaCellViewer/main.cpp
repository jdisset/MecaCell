#include "qtviewer.hpp"
#include "../MecaCell/mecacell.h"
#include "../MecaCell/basiccell.hpp"
#include "../MecaCell/basicworld.hpp"

/*************************************************************
 *               Minimalistic viewer exemple
 ************************************************************
 In this example we set up minimalistic init and loop routines
 We then start the viewer.
 */

using namespace MecaCell;
using namespace MecaCellViewer;

typedef BasicCell C;                   // our custom cell type
typedef BasicWorld<C, VerletEuler> W;  // our custom world type

/*************************
 *      INIT & LOOP
 ************************/
// Init func, will be called once before the first paint routine
void init(W& w) {
	// for this example we just create 1 cell
	w.addCell(new BasicCell(Vec::zero()));
}

// Loop function, called before every frame.
void loop(W& w) {
	// In this example we just call the world's update method
	w.update();
}

/*************************
 *        MAIN
 ************************/
int main(int argc, char** argv) {
	// we create our viewer with the init and loop methods
	QtViewer<W, C> viewer(init, loop);
	// and we start the application
	return viewer.exec(argc, argv);
}
