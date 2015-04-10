#ifndef RENDERER_HPP
#define RENDERER_HPP
#include "slotsignalbase.h"
#include <cmath>

template <typename World, typename Cell> class Renderer : public SignalSlotRenderer {
 private:
	World w;
	double shift = 0;

 public:
	explicit Renderer() : SignalSlotRenderer() {};

	virtual void paint() {
		shift +=0.01;
		GL->glClearColor(fmod(shift,1.0), 1.0 - fmod(shift,1.0), 0.8, 1);
		GL->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	virtual void sync(SignalSlotBase* ssb) {}
};
#endif
