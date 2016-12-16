#ifndef MECACEL_PARTICLE_HPP
#define MECACEL_PARTICLE_HPP
#include "../movable.h"
#include "../orientable.h"

namespace MecaCell {
struct OrientedParticle : public Movable, public Orientable {
	OrientedParticle(Vector3D pos) : Movable(pos){};
};
}
#endif
