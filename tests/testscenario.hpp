#ifndef TESTSCENARIO_HPP
#define TESTSCENARIO_HPP
#include "../mecacell/mecacell.h"
using std::vector;

template <typename Cell> class Scenario1 {
 protected:
	MecaCell::BasicWorld<Cell> w;

 public:
	MecaCell::BasicWorld<Cell>& getWorld() { return w; }

	void init(int, char**) {
		MecaCell::Vec origin(0, 0, 0);
		int cote = 3;
		int ecart = 50;
		for (int x = 0; x < cote; ++x) {
			for (int y = 0; y < cote; ++y) {
				for (int z = 0; z < cote; ++z) {
					auto* c = new Cell(origin + MecaCell::Vec(x, y, z) * ecart);
					w.addCell(c);
				}
			}
		}
	}

	int i = 0;
	void loop() { w.update(); }
};
#endif
