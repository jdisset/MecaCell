#include "qtviewer.hpp"
#include "../MecaCell/mecacell.h"
#include "../MecaCell/basiccell.hpp"
#include "../MecaCell/basicworld.hpp"

using namespace MecaCell;
using namespace MecaCellViewer;
int main (int argc, char** argv){
	QtViewer<BasicWorld<BasicCell,VerletEuler>,BasicCell> viewer(argc, argv);
	return viewer.exec();
}
