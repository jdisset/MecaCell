#include "vector3D.h"
#include "basis.h"
#include <iostream>

namespace MecaCell {
template <typename T> std::ostream &operator<<(std::ostream &out, const Basis<T> &b) {
	out << "[ " << b.X << ", " << b.Y << ", "<<b.Z<<" ]";
	return out;
}
}
