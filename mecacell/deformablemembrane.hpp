#ifndef DEFORMABLEMEMBRANE_HPP
#define DEFORMABLEMEMBRANE_HPP
#include "spheremembrane.hpp"
#include "surfacecontrolpoint.hpp"

namespace MecaCell {
template <typename Cell, unsigned int N = 20>
class DeformableMembrane : public SphereMembrane<Cell> {
	friend class SphereMembrane<Cell>;
	static const size_t NbScp = N;
	array<SurfaceControlPoint<Cell>, NbScp> scps;

 public:
	DeformableMembrane(Cell* c) : SphereMembrane<Cell>(c) {
		auto points = getSpherePointsPacking(NbScp);
		for (size_t i = 0; i < NbScp; ++i) {
			scps[i] = SurfaceControlPoint<Cell>(this->cell, points[i]);
		}
	}

	DeformableMembrane(Cell* c, const DeformableMembrane& dm)
	    : SphereMembrane<Cell>(c, static_cast<const SphereMembrane<Cell>&>(dm)) {
		for (size_t i = 0; i < NbScp; ++i) {
			scps[i] = SurfaceControlPoint<Cell>(this->cell, dm.scps[i]);
		}
	}
};
}
#endif
