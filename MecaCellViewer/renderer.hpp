#ifndef RENDERER_HPP
#define RENDERER_HPP
#include "slotsignalbase.h"
#include "cellGroup.hpp"
#include "camera.hpp"
#include "skybox.hpp"
#include <QThread>
#include <cmath>
#include <QOpengLContext>

template <typename World, typename Cell> class Renderer : public SignalSlotRenderer {
 private:
	World w;

	Camera camera;
	CellGroup<Cell> cells;
	Skybox skybox;

	function<void(World&)> init;
	function<void(World&)> loop;

 public:
	explicit Renderer(function<void(World&)> i, function<void(World&)> l)
	    : SignalSlotRenderer(), init(i), loop(l) {}

	// main paint method, called every frame
	virtual void paint() {
		// events handling
		processEvents();

		// world update
		loop(w);

		// we clear the screen and enable Depth testing
		GL->glDepthMask(true);
		GL->glClearColor(0.0, 0.5, 0.2, 1.0);
		GL->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		GL->glEnable(GL_DEPTH_TEST);

		// we compute view and projection matrices
		QMatrix4x4 view(camera.getViewMatrix());
		QMatrix4x4 projection(
		    camera.getProjectionMatrix((float)viewportSize.width() / (float)viewportSize.height()));

		// background
		skybox.draw(camera.getViewVector());
		cells.draw(w.cells, view, projection);
	}

	// called once just after the openGL context is created
	virtual void initialize() {
		GL = QOpenGLContext::currentContext()->functions();
		GL->initializeOpenGLFunctions();
		cells.load();
		skybox.load();
		init(w);
	}

	// called when the openGL context is invalidated
	virtual void cleanupSlot() {}

	// called after every frame, thread safe
	virtual void sync(SignalSlotBase*) {}

	// events handling routine
	void processEvents() {}

	// useful for creating a new instance from QSGRenderThread
	// ugly trick... but hey, it works!
	virtual SignalSlotRenderer* clone() { return new Renderer<World, Cell>(init, loop); }
};
#endif
