#ifndef RENDERER_HPP
#define RENDERER_HPP
#include "slotsignalbase.h"
#include "cellgroup.hpp"
#include "deformablecellgroup.hpp"
#include "connectionsgroup.hpp"
#include "points.hpp"
#include "camera.hpp"
#include "model.hpp"
#include "skybox.hpp"
#include "renderquad.hpp"
#include "blurquad.hpp"
#include "gridviewer.hpp"
#include "plugins.hpp"
#include "macros.h"
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

using std::vector;

template <typename Scenario, typename... Plugins>
class Renderer : public SignalSlotRenderer {
	friend class SignalSlotBase;

 public:
	using World =
	    typename remove_reference<decltype(((Scenario *)nullptr)->getWorld())>::type;
	using Cell = typename World::cell_type;
	using Vec = decltype(((Cell *)nullptr)->getPosition());
	using ModelType = typename World::model_type;

 private:
	// Scenario
	int argc;
	char **argv;
	Scenario scenario;

	int frame = 0;

	// Display configuration
	bool fullscreenMode = false, fullscreenModeToggled = false, skyboxEnabled = true;
	double BLUR_COEF = 1.0, MSAA_COEF = 4, FSAA_COEF = 20.0;
	double screenCoef = 1.0;
	int menuSize = 200;
	bool worldUpdate = true;
	bool loopStep = true;
	bool takeScreen = false;
	string screenName;
	colorMode cMode;

	// Visual elements
	Camera camera;
	CellGroup<Cell> cells;
	ConnectionsGroup connections;
	Skybox skybox;
	unique_ptr<QOpenGLFramebufferObject> ssaoFBO, msaaFBO, finalFBO, fsaaFBO;
	QOpenGLFramebufferObjectFormat ssaoFormat, msaaFormat, finalFormat;
	GLuint depthTex;
	RenderQuad ssaoTarget;
	BlurQuad blurTarget;
	GridViewer gridViewer;
	Points pointsViewer;
	unordered_map<std::string, ModelViewer<ModelType>> modelViewers;

	// Events
	int mouseWheel = 0;
	QPointF mousePosition, mousePrevPosition;
	QFlags<Qt::MouseButtons> mouseClickedButtons, mouseDblClickedButtons,
	    mousePressedButtons;
	QMap<QVariant, std::function<void()>> clickMethods;
	set<int> pressedKeys;
	set<int> inputKeys;  // same as pressedKeys but true only once per press
	std::set<QString> clickedButtons;

	// Stats
	chrono::time_point<chrono::high_resolution_clock> t0, tfps;
	double viewDt;
	int nbFramesSinceLastTick = 0;
	QVariantMap stats, guiCtrl;
	const int fpsRefreshRate = 400;
	Cell *selectedCell = nullptr;

	/***********************************
	 *              UTILS              *
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
		GL->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, s.width(), s.height(), 0,
		                 GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
	}

	/***********************************
	 *            PAINTING             *
	 ***********************************/
	// main paint method, called every frame
	virtual void paint() {
		if (loopStep || worldUpdate) {
			for (auto &p : plugins_preLoop) p();
			scenario.loop();
			if (!selectedCellStillExists()) selectedCell = nullptr;
			loopStep = false;
		}

		FSAA_COEF = screenCoef == 2.0 ? 0.5 : 1.0;

		processEvents();
		msaaFBO->bind();
		GL->glViewport(0, 0, viewportSize.width() * screenCoef * FSAA_COEF,
		               viewportSize.height() * screenCoef * FSAA_COEF);
		clear();
		for (auto &p : plugins_preDraw) p();

		QMatrix4x4 view(camera.getViewMatrix());
		QMatrix4x4 projection(camera.getProjectionMatrix((float)viewportSize.width() /
		                                                 (float)viewportSize.height()));

		if (skyboxEnabled) {
			// background
			skybox.draw(view, projection, camera);
		}

		QStringList gc;
		if (guiCtrl.count("visibleElements")) {
			gc = qvariant_cast<QStringList>(guiCtrl.value("visibleElements"));
		}
		if (gc.contains("cells")) {
			cells.drawMode = plain;
			cells.draw(scenario.getWorld().cells, view, projection, camera.getViewVector(),
			           camera.getPosition(), cMode, selectedCell);
		}
		if (gc.contains("cellGrid")) {
			gridViewer.draw(scenario.getWorld().getCellGrid(), view, projection,
			                QVector4D(0.99, 0.9, 0.4, 1.0));
		}
		if (gc.contains("modelGrid")) {
			gridViewer.draw(scenario.getWorld().getModelGrid(), view, projection,
			                QVector4D(0.6, 0.1, 0.1, 1.0));
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

		for (auto &p : plugins_onDraw) p();

		msaaFBO->release();
		QOpenGLFramebufferObject::blitFramebuffer(ssaoFBO.get(), msaaFBO.get(),
		                                          GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		finalFBO->bind();
		ssaoTarget.draw(ssaoFBO->texture(), depthTex, camera.getNearPlane(),
		                camera.getFarPlane());

		// no ssao from here
		if (gc.contains("centers")) {
			cells.drawMode = centers;
			cells.draw(scenario.getWorld().cells, view, projection, camera.getViewVector(),
			           camera.getPosition(), cMode, selectedCell);
		}
		if (gc.contains("connections")) {
			connections.draw<Cell>(scenario.getWorld().getConnectedCellsList(), view,
			                              projection);
			// connections.drawModelConnections<Cell>(scenario.getWorld().cells, view,
			// projection);
		}

		QOpenGLFramebufferObject::blitFramebuffer(
		    fsaaFBO.get(), QRect(QPoint(0, 0), viewportSize * screenCoef), finalFBO.get(),
		    QRect(QPoint(0, 0), viewportSize * FSAA_COEF * screenCoef), GL_COLOR_BUFFER_BIT,
		    GL_LINEAR);

		if (guiCtrl.count("takeScreen")) {
			stringstream screenshotName;
			screenshotName << "screen" << frame << ".jpg";
			fsaaFBO->toImage().save(QString::fromStdString(screenshotName.str()), 0, 97);
		}
		if (takeScreen) {
			fsaaFBO->toImage().save(QString::fromStdString(screenName), 0, 97);
			takeScreen = false;
		}
		GL->glViewport(0, 0, viewportSize.width() * screenCoef,
		               viewportSize.height() * screenCoef);
		blurTarget.draw(fsaaFBO->texture(), 5, viewportSize * screenCoef,
		                QRect(QPoint(0, 0), QSize(fullscreenMode ? 0 : menuSize * screenCoef,
		                                          viewportSize.height() * screenCoef)));

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
		camera.updatePosition(viewDt);
		++frame;
		if (window) window->update();
		for (auto &p : plugins_postDraw) p();
	}

	colorMode strToColorMode(const QString &cm) {
		if (cm == "pressure") return pressure;
		if (cm == "owncolor") return owncolor;
	}

	bool selectedCellStillExists() {
		return (std::find(scenario.getWorld().cells.begin(), scenario.getWorld().cells.end(),
		                  selectedCell) != scenario.getWorld().cells.end());
	}

	/***********************************
	 *         INITIALIZATION          *
	 ***********************************/

	// plugins
	struct Dummy {};
	std::tuple<Dummy, Plugins...> plugins;

 public:
	std::vector<std::function<void()>> plugins_onLoad;
	std::vector<std::function<void()>> plugins_onDraw;
	std::vector<std::function<void()>> plugins_preDraw;
	std::vector<std::function<void()>> plugins_preLoop;
	std::vector<std::function<void()>> plugins_postDraw;

 private:
	virtual void initialize() {
		scenario.init(argc, argv);
		// initInterface();
		// gl functions
		GL = QOpenGLContext::currentContext()->functions();
		GL->initializeOpenGLFunctions();

		// loading plugins
		forEach(plugins, PluginLoader<Renderer<Scenario, Plugins...>>{this});

		for (auto &p : plugins_onLoad) p();

		// elements
		cells.load();
		connections.load();
		skybox.load();
		ssaoTarget.load(":/shaders/dumb.vert", ":/shaders/ssao.frag");
		blurTarget.load(":/shaders/dumb.vert", ":/shaders/blur.frag",
		                viewportSize * screenCoef);
		gridViewer.load(":/shaders/mvp.vert", ":/shaders/flat.frag");
		pointsViewer.load();

		// fbos
		ssaoFormat.setAttachment(QOpenGLFramebufferObject::Depth);
		ssaoFormat.setSamples(0);
		finalFormat.setSamples(0);
		finalFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
		msaaFormat.setAttachment(QOpenGLFramebufferObject::Depth);
		msaaFormat.setSamples(MSAA_COEF);
		fsaaFBO = unique_ptr<QOpenGLFramebufferObject>(
		    new QOpenGLFramebufferObject(viewportSize, finalFormat));
		finalFBO = unique_ptr<QOpenGLFramebufferObject>(
		    new QOpenGLFramebufferObject(viewportSize, finalFormat));
		ssaoFBO = unique_ptr<QOpenGLFramebufferObject>(
		    new QOpenGLFramebufferObject(viewportSize, ssaoFormat));
		msaaFBO = unique_ptr<QOpenGLFramebufferObject>(
		    new QOpenGLFramebufferObject(viewportSize, msaaFormat));
		fsaaFBO->setAttachment(QOpenGLFramebufferObject::NoAttachment);
		finalFBO->setAttachment(QOpenGLFramebufferObject::NoAttachment);
		ssaoFBO->setAttachment(QOpenGLFramebufferObject::Depth);
		msaaFBO->setAttachment(QOpenGLFramebufferObject::Depth);
		ssaoFBO->bind();
		GL->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
		                           depthTex, 0);
		ssaoFBO->release();

		// scenario
	}

	/***********************************
	 *           UTILS & SYNC          *
	 ***********************************/
	// called when the openGL context is invalidated
	virtual void cleanupSlot() {}

	// useful for creating a new instance from QSGRenderThread
	// ugly trick... but hey, it works!
	virtual SignalSlotRenderer *clone() {
		return new Renderer<Scenario, Plugins...>(argc, argv);
	}

	// called after every frame, thread safe
	virtual void sync(SignalSlotBase *b) {
		applyInterfaceAdditions(b);
		// interface comm
		guiCtrl = b->getGuiCtrl();
		worldUpdate = b->worldUpdate;
		loopStep = b->loopStep;
		b->loopStep = false;
		clickedButtons = b->clickedButtons;
		b->clickedButtons.clear();
		if (fullscreenModeToggled) {
			cerr << "fsmode toggled" << endl;
			QObject *root = b->parentItem();
			QMetaObject::invokeMethod(root, "fullscreenMode", Q_ARG(QVariant, fullscreenMode));
			fullscreenModeToggled = false;
		}
		// stats
		if (selectedCell)
			stats["selectedCell"] = cellToQVMap(selectedCell);
		else
			stats.remove("selectedCell");
		b->setStats(stats);
		b->statsChanged();
		cMode = guiCtrl.contains("colorMode") ?
		            strToColorMode(guiCtrl["colorMode"].toString()) :
		            owncolor;
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
			if (guiCtrl.value("tool") == "move" || fullscreenMode) {
				camera.moveAroundTarget(-mouseMovement);
			}
		}
		// cell dragging
		if (mousePressedButtons.testFlag(Qt::RightButton)) {
			if (selectedCell) {
				QVector2D mouseNDC(
				    2.0 * mousePosition.x() / (float)viewportSize.width() - 1.0,
				    -((2.0 * (float)mousePosition.y()) / (float)viewportSize.height() - 1.0));
				QVector4D rayEye = camera.getProjectionMatrix((float)viewportSize.width() /
				                                              (float)viewportSize.height())
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
					decltype(selectedCell->getPosition()) newPos(projectedPos.x(), projectedPos.y(),
					                                             projectedPos.z());
					selectedCell->setPosition(newPos);
					selectedCell->resetVelocity();
				}
			}
		}
		// click
		if (mouseClickedButtons.testFlag(Qt::LeftButton)) {
			if (clickMethods.count(guiCtrl.value("tool")))
				clickMethods.value(guiCtrl.value("tool"))();  // we call the corresponding method
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

		for (auto &bName : clickedButtons) {
			if (buttonMap.count(bName)) {
				buttonMap[bName].onClicked(this);
			}
		}
	}

	Vec QV3D2Vec(const QVector3D &v) { return Vec(v.x(), v.y(), v.z()); }

	void setViewportSize(const QSize &s) {
		if (window) screenCoef = window->devicePixelRatio();
		viewportSize = s;
		GL->glDeleteTextures(1, &depthTex);
		genDepthTexture(FSAA_COEF * viewportSize * screenCoef, depthTex);
		fsaaFBO.reset(new QOpenGLFramebufferObject(viewportSize * screenCoef, ssaoFormat));
		finalFBO.reset(
		    new QOpenGLFramebufferObject(FSAA_COEF * viewportSize * screenCoef, ssaoFormat));
		ssaoFBO.reset(
		    new QOpenGLFramebufferObject(FSAA_COEF * viewportSize * screenCoef, ssaoFormat));
		msaaFBO.reset(
		    new QOpenGLFramebufferObject(FSAA_COEF * viewportSize * screenCoef, msaaFormat));
		fsaaFBO->setAttachment(QOpenGLFramebufferObject::NoAttachment);
		finalFBO->setAttachment(QOpenGLFramebufferObject::NoAttachment);
		ssaoFBO->setAttachment(QOpenGLFramebufferObject::Depth);
		msaaFBO->setAttachment(QOpenGLFramebufferObject::Depth);
		ssaoFBO->bind();
		GL->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
		                           depthTex, 0);
		ssaoFBO->release();
	}

	QVariantMap cellToQVMap(Cell *c) {
		QVariantMap res;
		if (c) {
			res["Radius"] = c->getBoundingBoxRadius();
			res["Volume"] = c->getVolume();
			res["Pressure"] = c->getPressure();
			res["Mass"] = c->getMass();
			res["Connections"] = c->getNbConnections();
		}
		return res;
	}
	void addCuttingPlane() {}
	void pickCell() {
		QVector2D mouseNDC(
		    2.0 * mousePosition.x() / (float)viewportSize.width() - 1.0,
		    -((2.0 * (float)mousePosition.y()) / (float)viewportSize.height() - 1.0));
		QVector4D rayEye = camera.getProjectionMatrix((float)viewportSize.width() /
		                                              (float)viewportSize.height())
		                       .inverted() *
		                   QVector4D(mouseNDC, -1.0, 1.0);
		rayEye = QVector4D(rayEye.x(), rayEye.y(), -1.0, 0.0);
		QVector4D ray = camera.getViewMatrix().inverted() * rayEye;
		QVector3D vray(ray.x(), ray.y(), ray.z());
		vray.normalize();
		Cell *res = nullptr;
		double minEyeDist = 1e20;
		for (auto &c : scenario.getWorld().cells) {
			if (c->getVisible()) {
				double sqRad = pow(c->getBoundingBoxRadius(), 2);
				QVector3D EC = toQV3D(c->getPosition()) - camera.getPosition();
				QVector3D EV = vray * QVector3D::dotProduct(EC, vray);
				double eyeDist = EC.lengthSquared();
				double rayDist = eyeDist - EV.lengthSquared();
				if (rayDist <= sqRad && eyeDist < minEyeDist) {
					minEyeDist = eyeDist;
					res = c;
				}
			}
		}
		selectedCell = res;
	}

	void applyInterfaceAdditions(SignalSlotBase *b) {
		QObject *root = b->parentItem();
		for (auto &b : buttonMap) {
			if (b.second.updt) {
				QMetaObject::invokeMethod(root, "addButton", Q_ARG(QVariant, b.second.name),
				                          Q_ARG(QVariant, b.second.menu),
				                          Q_ARG(QVariant, b.second.label),
				                          Q_ARG(QVariant, b.second.color));
				b.second.updt = false;
			}
		}
	}

 public:
	explicit Renderer(int c, char **v) : SignalSlotRenderer(), argc(c), argv(v) {
		clickMethods["select"] = bind(&Renderer<Scenario, Plugins...>::pickCell, this);
	}

	Cell *getSelectedCell() { return selectedCell; }
	Scenario &getScenario() { return scenario; }

	Camera &getCamera() { return camera; }

	void screenCapture(std::string name) {
		takeScreen = true;
		screenName = name;
	}
	void disableSkybox() { skyboxEnabled = false; }
	void enableSkybox() { skyboxEnabled = true; }

	void setFullScreenMode(bool f) {
		fullscreenMode = f;
		fullscreenModeToggled = true;
		cerr << "setFsMode called" << endl;
	}

	class Button {
		friend class Renderer<Scenario, Plugins...>;

	 private:
		QString name, menu, label;
		std::function<void(Renderer<Scenario, Plugins...> *)> onClicked;
		QColor color = QColor(255, 255, 255);

	 public:
		Button() {}
		Button(QString n, QString m, QString l,
		       std::function<void(Renderer<Scenario, Plugins...> *)> c)
		    : name(n), menu(m), label(l), onClicked(c){};
		bool updt = true;
		// void setLabel(const std::string &l) {
		// label = QString::fromStdString(l);
		// updt = true;
		//}
		void setLabel(const QString &l) {
			label = l;
			updt = true;
		}
		void setOnClicked(std::function<void(Renderer<Scenario, Plugins...> *)> f) {
			onClicked = f;
			updt = true;
		}
		void setColor(const int r, const int g, const int b) {
			color = QColor(r, g, b);
			updt = true;
		}
		void setColor(const QColor &c) {
			color = c;
			updt = true;
		}
		void setColor(const double r, const double g, const double b) {
			color = QColor::fromRgbF(r, g, b);
			updt = true;
		}
	};

	Button &getButton(const std::string &name) {
		return buttonMap.at(QString::fromStdString(name));
	}
	std::map<QString, Button> buttonMap;
	void addButton(std::string name, std::string menu, std::string label,
	               std::function<void(Renderer<Scenario, Plugins...> *)> onClicked) {
		Button b(QString::fromStdString(name), QString::fromStdString(menu),
		         QString::fromStdString(label), onClicked);

		buttonMap[QString::fromStdString(name)] = b;
	}
};
}
#endif
