#ifndef RENDERER_HPP
#define RENDERER_HPP
#include "slotsignalbase.h"
#include "cellgroup.hpp"
#include "connectionsgroup.hpp"
#include "camera.hpp"
#include "model.hpp"
#include "skybox.hpp"
#include "renderquad.hpp"
#include "blurquad.hpp"
#include "gridviewer.hpp"
#include <type_traits>
#include <QThread>
#include <cmath>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QVariant>
#include <QString>
#include <chrono>
#include <sstream>

namespace MecacellViewer {
template <typename Scenario> class Renderer : public SignalSlotRenderer {

private:
	using World = typename remove_reference<decltype(((Scenario *)nullptr)->getWorld())>::type;
	using Cell = typename World::cell_type;
	using Vec = decltype(((Cell *)nullptr)->getPosition());
	using ConnectType = typename World::connect_type;
	using ModelType = typename World::model_type;

	// Scenario
	int argc;
	char **argv;
	Scenario scenario;
	int frame = 0;

	// Visual elements
	Camera camera;
	CellGroup<Cell> cells;
	ConnectionsGroup<ConnectType> connections;
	Skybox skybox;
	unique_ptr<QOpenGLFramebufferObject> ssaoFBO, msaaFBO, finalFBO, fsaaFBO;
	QOpenGLFramebufferObjectFormat ssaoFormat, msaaFormat, finalFormat;
	GLuint depthTex;
	RenderQuad ssaoTarget;
	BlurQuad blurTarget;
	GridViewer gridViewer;
	unordered_map<std::string, ModelViewer<ModelType>> modelViewers;

	// Events
	int mouseWheel = 0;
	QPointF mousePosition, mousePrevPosition;
	QFlags<Qt::MouseButtons> mouseClickedButtons, mouseDblClickedButtons, mousePressedButtons;
	QMap<QVariant, std::function<void()>> clickMethods;
	set<int> pressedKeys;
	set<int> inputKeys; // same as pressedKeys but true only once per press

	// Stats
	chrono::time_point<chrono::high_resolution_clock> t0, tfps;
	double viewDt;
	int nbFramesSinceLastTick = 0;
	QVariantMap stats, guiCtrl;
	const int fpsRefreshRate = 400;
	Cell *selectedCell = nullptr;

	// options
	double BLUR_COEF = 1.0, MSAA_COEF = 4, FSAA_COEF = 20.0;
	double screenCoef = 1.0;
	int menuSize = 200;
	bool worldUpdate = true;
	bool loopStep = true;
	bool cut = false;

public:
	explicit Renderer(int c, char **v) : SignalSlotRenderer(), argc(c), argv(v) {
		clickMethods["select"] = bind(&Renderer<Scenario>::pickCell, this);
	}

	/***********************************
	 *            PAINTING             *
	 ***********************************/
	// main paint method, called every frame
	virtual void paint() {
		if (loopStep || worldUpdate) {
			scenario.loop();
			loopStep = false;
		}

		cells.cut = cut;
		FSAA_COEF = screenCoef == 2.0 ? 0.7 : 1.0;

		processEvents();
		msaaFBO->bind();
		GL->glViewport(0, 0, viewportSize.width() * screenCoef * FSAA_COEF,
		               viewportSize.height() * screenCoef * FSAA_COEF);
		clear();

		QMatrix4x4 view(camera.getViewMatrix());
		QMatrix4x4 projection(
		    camera.getProjectionMatrix((float)viewportSize.width() / (float)viewportSize.height()));

		// background
		skybox.draw(view, projection, camera);

		QStringList gc;
		if (guiCtrl.count("visibleElements")) {
			gc = qvariant_cast<QStringList>(guiCtrl.value("visibleElements"));
		}
		if (gc.contains("cells")) {
			cells.drawMode = plain;
			cells.draw(scenario.getWorld().cells, view, projection, camera.getViewVector(), camera.getPosition(),
			           selectedCell);
		}
		if (gc.contains("cellGrid")) {
			gridViewer.draw(scenario.getWorld().getCellGrid(), view, projection, QVector4D(0.99, 0.9, 0.4, 1.0));
		}
		if (gc.contains("modelGrid")) {
			gridViewer.draw(scenario.getWorld().getModelGrid(), view, projection, QVector4D(0.6, 0.1, 0.1, 1.0));
		}

		for (auto &m : scenario.getWorld().models) {
			if (!modelViewers.count(m.first)) {
				modelViewers[m.first];
				modelViewers[m.first].load(m.second);
			}
		}
		for (auto &m : modelViewers) {
			if (scenario.getWorld().models.count(m.first)) {
				m.second.draw(view, projection, scenario.getWorld().models.at(m.first));
			}
		}

		msaaFBO->release();
		QOpenGLFramebufferObject::blitFramebuffer(ssaoFBO.get(), msaaFBO.get(),
		                                          GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		finalFBO->bind();
		ssaoTarget.draw(ssaoFBO->texture(), depthTex, camera.getNearPlane(), camera.getFarPlane());

		// no ssao from here
		if (gc.contains("centers")) {
			cells.drawMode = centers;
			cells.draw(scenario.getWorld().cells, view, projection, camera.getViewVector(), camera.getPosition(),
			           selectedCell);
		}
		if (gc.contains("connections")) {
			connections.draw(scenario.getWorld().connections, view, projection);
		}

		GL->glViewport(0, 0, viewportSize.width() * screenCoef, viewportSize.height() * screenCoef);

		QOpenGLFramebufferObject::blitFramebuffer(
		    fsaaFBO.get(), QRect(QPoint(0, 0), viewportSize * screenCoef), finalFBO.get(),
		    QRect(QPoint(0, 0), viewportSize * FSAA_COEF * screenCoef), GL_COLOR_BUFFER_BIT, GL_LINEAR);

		blurTarget.draw(fsaaFBO->texture(), 5, viewportSize * screenCoef,
		                QRect(QPoint(0, 0), QSize(menuSize * screenCoef, viewportSize.height() * screenCoef)));

		// refresh stats
		auto t1 = chrono::high_resolution_clock::now();
		auto fpsDt = chrono::duration_cast<chrono::milliseconds>(t1 - tfps);
		nbFramesSinceLastTick++;
		if (fpsDt.count() > fpsRefreshRate) {
			stats["fps"] = 1000.0 * (double)nbFramesSinceLastTick / (double)fpsDt.count();
			nbFramesSinceLastTick = 0;
			tfps = chrono::high_resolution_clock::now();
		}
		stats["nbCells"] = QVariant((int)scenario.getWorld().cells.size());
		stats["nbUpdates"] = scenario.getWorld().getNbUpdates();
		if (window) {
			window->resetOpenGLState();
		}
		chrono::duration<double> dv = t1 - t0;
		viewDt = dv.count();
		t0 = chrono::high_resolution_clock::now();
		++frame;
		if (window) window->update();
	}

	/***********************************
	 *         INITIALIZATION          *
	 ***********************************/
	// called once just after the openGL context is created
	virtual void initialize() {
		scenario.init(argc, argv);
		// gl functions
		GL = QOpenGLContext::currentContext()->functions();
		GL->initializeOpenGLFunctions();

		// elements
		cells.load();
		connections.load();
		skybox.load();
		ssaoTarget.load(":/shaders/dumb.vert", ":/shaders/ssao.frag");
		blurTarget.load(":/shaders/dumb.vert", ":/shaders/blur.frag", viewportSize * screenCoef);
		gridViewer.load(":/shaders/mvp.vert", ":/shaders/flat.frag");

		// fbos
		ssaoFormat.setAttachment(QOpenGLFramebufferObject::Depth);
		ssaoFormat.setSamples(0);
		finalFormat.setSamples(0);
		finalFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
		msaaFormat.setAttachment(QOpenGLFramebufferObject::Depth);
		msaaFormat.setSamples(MSAA_COEF);
		fsaaFBO = unique_ptr<QOpenGLFramebufferObject>(new QOpenGLFramebufferObject(viewportSize, finalFormat));
		finalFBO = unique_ptr<QOpenGLFramebufferObject>(new QOpenGLFramebufferObject(viewportSize, finalFormat));
		ssaoFBO = unique_ptr<QOpenGLFramebufferObject>(new QOpenGLFramebufferObject(viewportSize, ssaoFormat));
		msaaFBO = unique_ptr<QOpenGLFramebufferObject>(new QOpenGLFramebufferObject(viewportSize, msaaFormat));
		fsaaFBO->setAttachment(QOpenGLFramebufferObject::NoAttachment);
		finalFBO->setAttachment(QOpenGLFramebufferObject::NoAttachment);
		ssaoFBO->setAttachment(QOpenGLFramebufferObject::Depth);
		msaaFBO->setAttachment(QOpenGLFramebufferObject::Depth);
		ssaoFBO->bind();
		GL->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
		ssaoFBO->release();

		// scenario
	}

	/***********************************
	 *           UTILS & SYNC          *
	 ***********************************/
	void clear() {
		GL->glDepthMask(true);
		GL->glClearColor(1.0, 1.0, 1.0, 1.0);
		GL->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		GL->glEnable(GL_DEPTH_TEST);
	}
	// depth texture initialisation
	void genDepthTexture(QSize s, GLuint &t) {
		GL->glGenTextures(1, &t);
		GL->glBindTexture(GL_TEXTURE_2D, t);
		GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		GL->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, s.width(), s.height(), 0, GL_DEPTH_COMPONENT,
		                 GL_UNSIGNED_INT, NULL);
	}

	// called when the openGL context is invalidated
	virtual void cleanupSlot() {}

	// useful for creating a new instance from QSGRenderThread
	// ugly trick... but hey, it works!
	virtual SignalSlotRenderer *clone() { return new Renderer<Scenario>(argc, argv); }

	// called after every frame, thread safe
	virtual void sync(SignalSlotBase *b) {
		// interface comm
		guiCtrl = b->getGuiCtrl();
		worldUpdate = b->worldUpdate;
		loopStep = b->loopStep;
		b->loopStep = false;
		// stats
		if (selectedCell)
			stats["selectedCell"] = cellToQVMap(selectedCell);
		else
			stats.remove("selectedCell");
		b->setStats(stats);
		b->statsChanged();
		// mouse
		mouseClickedButtons = b->mouseClickedButtons;
		b->mouseClickedButtons &= Qt::NoButton;
		mouseDblClickedButtons = b->mouseDblClickedButtons;
		b->mouseDblClickedButtons &= Qt::NoButton;
		mousePrevPosition = mousePosition;
		mousePosition = b->lastMouseEvent.localPos();
		if (mouseClickedButtons > 0) {
			mousePrevPosition = mousePosition;
		}
		mousePressedButtons = b->lastMouseEvent.buttons();
		mouseWheel = b->mouseWheel;
		b->mouseWheel = 0;

		// keyboard
		pressedKeys = b->pressedKeys;
	}

	/***********************************
	 *              EVENTS              *
	 ***********************************/
	// events handling routine
	void processEvents() {
		// mouse input
		// move
		QVector2D mouseMovement(mousePosition - mousePrevPosition);
		if (mousePressedButtons.testFlag(Qt::LeftButton)) {
			if (guiCtrl.value("tool") == "move") {
				camera.moveAroundTarget(-mouseMovement);
			}
		}
		// cell dragging
		if (mousePressedButtons.testFlag(Qt::RightButton)) {
			if (selectedCell) {
				QVector2D mouseNDC(2.0 * mousePosition.x() / (float)viewportSize.width() - 1.0,
				                   -((2.0 * (float)mousePosition.y()) / (float)viewportSize.height() - 1.0));
				QVector4D rayEye =
				    camera.getProjectionMatrix((float)viewportSize.width() / (float)viewportSize.height())
				        .inverted() *
				    QVector4D(mouseNDC, -1.0, 1.0);
				rayEye = QVector4D(rayEye.x(), rayEye.y(), -1.0, 0.0);
				QVector4D ray = camera.getViewMatrix().inverted() * rayEye;
				QVector3D l(ray.x(), ray.y(), ray.z());
				QVector3D l0 = camera.getPosition();
				QVector3D p0 = toQV3D(selectedCell->getPosition());
				QVector3D n = camera.getOrientation();
				if (QVector3D::dotProduct(l, n) != 0) {
					float d = (QVector3D::dotProduct((p0 - l0), n)) / QVector3D::dotProduct(l, n);
					QVector3D projectedPos = l0 + d * l;
					decltype(selectedCell->getPosition()) newPos(projectedPos.x(), projectedPos.y(), projectedPos.z());
					selectedCell->setPosition(newPos);
					selectedCell->resetVelocity();
				}
			}
		}
		// click
		if (mouseClickedButtons.testFlag(Qt::LeftButton)) {
			if (clickMethods.count(guiCtrl.value("tool")))
				clickMethods.value(guiCtrl.value("tool"))(); // we call the corresponding method
		}
		if (mouseClickedButtons.testFlag(Qt::MiddleButton)) {
			pickCell();
		}
		if (pressedKeys.count(Qt::Key_Up) || pressedKeys.count(Qt::Key_Z)) {
			camera.forward(viewDt);
		}
		if (pressedKeys.count(Qt::Key_Down) || pressedKeys.count(Qt::Key_S)) {
			camera.backward(viewDt);
		}
		if (pressedKeys.count(Qt::Key_Left) || pressedKeys.count(Qt::Key_Q)) {
			camera.left(viewDt);
		}
		if (pressedKeys.count(Qt::Key_Right) || pressedKeys.count(Qt::Key_D)) {
			camera.right(viewDt);
		}
		if (inputKeys.count(Qt::Key_C)) {
			cut = !cut;
		}
		if (pressedKeys.count(Qt::Key_Space)) {
			Cell *c = new Cell(QV3D2Vec(camera.getPosition()) - QV3D2Vec(camera.getUpVector() * 50.0));
			c->setVelocity(QV3D2Vec(camera.getViewVector() * 3000.0));
			scenario.getWorld().addCell(c);
		}
		// if (selectedCell && pressedKeys.count(Qt::Key_M)) {
		// selectedCell->startDivision();
		//}
	}
	Vec QV3D2Vec(const QVector3D &v) { return Vec(v.x(), v.y(), v.z()); }

	void setViewportSize(const QSize &s) {
		if (window) screenCoef = window->devicePixelRatio();
		viewportSize = s;
		GL->glDeleteTextures(1, &depthTex);
		genDepthTexture(FSAA_COEF * viewportSize * screenCoef, depthTex);
		fsaaFBO.reset(new QOpenGLFramebufferObject(viewportSize * screenCoef, ssaoFormat));
		finalFBO.reset(new QOpenGLFramebufferObject(FSAA_COEF * viewportSize * screenCoef, ssaoFormat));
		ssaoFBO.reset(new QOpenGLFramebufferObject(FSAA_COEF * viewportSize * screenCoef, ssaoFormat));
		msaaFBO.reset(new QOpenGLFramebufferObject(FSAA_COEF * viewportSize * screenCoef, msaaFormat));
		fsaaFBO->setAttachment(QOpenGLFramebufferObject::NoAttachment);
		finalFBO->setAttachment(QOpenGLFramebufferObject::NoAttachment);
		ssaoFBO->setAttachment(QOpenGLFramebufferObject::Depth);
		msaaFBO->setAttachment(QOpenGLFramebufferObject::Depth);
		ssaoFBO->bind();
		GL->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
		ssaoFBO->release();
	}

	QVariantMap cellToQVMap(Cell *c) {
		QVariantMap res;
		if (c) {
			res["Radius"] = c->getRadius();
			res["Stiffness"] = c->getStiffness();
			res["Volume"] = c->getVolume();
			res["Pressure"] = c->getPressure();
			res["Mass"] = c->getMass();
			res["Connections"] = c->getNbConnections();
		}
		return res;
	}
	void addCuttingPlane() {}
	void pickCell() {
		QVector2D mouseNDC(2.0 * mousePosition.x() / (float)viewportSize.width() - 1.0,
		                   -((2.0 * (float)mousePosition.y()) / (float)viewportSize.height() - 1.0));
		QVector4D rayEye =
		    camera.getProjectionMatrix((float)viewportSize.width() / (float)viewportSize.height()).inverted() *
		    QVector4D(mouseNDC, -1.0, 1.0);
		rayEye = QVector4D(rayEye.x(), rayEye.y(), -1.0, 0.0);
		QVector4D ray = camera.getViewMatrix().inverted() * rayEye;
		QVector3D vray(ray.x(), ray.y(), ray.z());
		vray.normalize();
		Cell *res = nullptr;
		double minEyeDist = 1e20;
		for (auto &c : scenario.getWorld().cells) {
			double sqRad = pow(c->getRadius(), 2);
			QVector3D EC = toQV3D(c->getPosition()) - camera.getPosition();
			QVector3D EV = vray * QVector3D::dotProduct(EC, vray);
			double eyeDist = EC.lengthSquared();
			double rayDist = eyeDist - EV.lengthSquared();
			if (rayDist <= sqRad && eyeDist < minEyeDist) {
				minEyeDist = eyeDist;
				res = c;
			}
		}
		selectedCell = res;
	}
};
}
#endif
