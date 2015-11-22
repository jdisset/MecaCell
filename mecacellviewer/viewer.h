#ifndef QTVIEWER_H
#define QTVIEWER_H
#include "signalslotbase.h"
#include "keyboardmanager.hpp"
#include "mousemanager.hpp"
#include "paintstep.hpp"
#include "screenmanager.hpp"
#include "camera.hpp"
#include "button.hpp"
#include "viewtools.h"
#include "arrowsgroup.hpp"
#include "cellgroup.hpp"
#include "gridviewer.hpp"
#include "plugins.hpp"
#include "skybox.hpp"
#include "msaa.hpp"
#include "ssao.hpp"
#include "blur.hpp"
#include <QMap>
#include <QOpenGLFramebufferObject>
#include <QMatrix4x4>
#include <QQuickView>
#include <QQmlContext>
#include <QGuiApplication>
#include <QApplication>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QQuickItem>
#include <QQmlContext>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <utility>
#include <functional>

#define MECACELL_VIEWER
#include "macros.h"

using namespace std;
namespace MecacellViewer {
template <typename Scenario> class Viewer : public SignalSlotRenderer {
	friend class SignalSlotBase;

 public:
	using World =
	    typename remove_reference<decltype(((Scenario *)nullptr)->getWorld())>::type;
	using Cell = typename World::cell_type;
	using Vec = decltype(((Cell *)nullptr)->getPosition());
	using ModelType = typename World::model_type;
	using R = Viewer<Scenario>;
	using Rfunc = std::function<void(R *)>;

	Viewer(int c, char **v) : argc(c), argv(v) {
#if __APPLE__
#include "TargetConditionals.h"
#if TARGET_OS_MAC
		// compatibility profile (Qt's default) is not available on mac os...
		// we have to use a core profile
		QSurfaceFormat f;
		f.setProfile(QSurfaceFormat::CoreProfile);
		f.setVersion(3, 3);
		f.setAlphaBufferSize(8);
		f.setRenderableType(QSurfaceFormat::OpenGL);
		QSurfaceFormat::setDefaultFormat(f);
#endif
#endif
		registerPlugin(km);
		registerPlugin(mm);
	};
	// default "plugins"
	KeyboardManager km;
	MouseManager mm;

	int argc;
	char **argv;
	Scenario scenario;
	int frame = 0;

	// Visual elements & config
	Camera camera;
	float screenScaleCoef = 1.0;
	bool fullscreenMode = false;
	unsigned long leftMenuSize = 200;
	QOpenGLFramebufferObject *currentFBO = nullptr;
	QSize viewportSize;
	QMatrix4x4 viewMatrix, projectionMatrix;

	// Events
	int mouseWheel = 0;
	QPointF mousePosition, mousePrevPosition;
	QFlags<Qt::MouseButtons> mouseClickedButtons, mouseDblClickedButtons,
	    mousePressedButtons;
	std::set<Qt::Key> keyDown, keyPress;
	std::set<QString> clickedButtons;

	// Stats
	std::chrono::time_point<std::chrono::high_resolution_clock> t0, tfps;
	double viewDt;
	int nbFramesSinceLastTick = 0;
	unsigned long currentFrameNumber = 0;
	Cell *selectedCell = nullptr;
	bool worldUpdate = true;
	bool loopStep = true;
	double fpsRefreshRate = 0.4;
	QVariantMap stats;

 public:
	std::vector<Rfunc> plugins_onLoad;
	std::vector<Rfunc> plugins_preLoop;
	std::vector<Rfunc> plugins_preDraw;
	std::vector<Rfunc> plugins_onDraw;
	std::vector<Rfunc> plugins_postDraw;

 private:
	std::map<Qt::Key, Rfunc> keyDownMethods;
	std::map<Qt::Key, Rfunc> keyPressMethods;
	std::map<Qt::MouseButton, Rfunc> mouseDragMethods;
	std::map<Qt::MouseButton, Rfunc> mouseClickMethods;
	std::map<QString, Button<R>> buttons;

	// paint steps : ordered list of callable paint actions
	// (draw objects, set up effects...)
	std::vector<unique_ptr<PaintStep<R>>> paintSteps;
	std::set<QString> enabledPaintSteps;
	bool paintStepsNeedsUpdate =
	    true;  // do we need to refresh the list of checkable paint steps?

	// screen managers might affect the display. Usually manipulate fbos
	// Inherit from paintStep because they also usually need to be called
	// during painting. Ex: screen space ambient oclusion defines some fbos,
	// makes operations on them and draw an object (a texture) to the screen.
	std::vector<ScreenManager<R> *> screenManagers;

	// init function for the renderer. Create all the defaults paint steps and
	// screen managers, initializes scenario and users additions.
	virtual void initialize() {
		scenario.init(argc, argv);
		GL = QOpenGLContext::currentContext()->functions();
		GL->initializeOpenGLFunctions();
		////////////////////////////////
		// list of default paint steps
		/////////////////////////////////
		paintSteps.emplace_back(new MSAA<R>(viewportSize));
		paintSteps.emplace_back(new Skybox<R>());
		paintSteps.emplace_back(new CellGroup<R>());
		paintSteps.emplace_back(new ArrowsGroup<R>("Forces", [](R *r) {
			auto f0 = r->scenario.getWorld().getAllForces();
			vector<pair<QVector3D, QVector3D>> f;
			f.reserve(f0.size());
			for (auto &p : f0) {
				f.push_back(make_pair(toQV3D(p.first), toQV3D(p.second)));
			}
			return f;
		}, QVector4D(1.0, 0.7, 0.1, 1.0)));
		paintSteps.emplace_back(new ArrowsGroup<R>("Velocities", [](R *r) {
			auto f0 = r->scenario.getWorld().getAllVelocities();
			vector<pair<QVector3D, QVector3D>> f;
			f.reserve(f0.size());
			for (auto &p : f0) {
				f.push_back(make_pair(toQV3D(p.first), toQV3D(p.second)));
			}
			return f;
		}, QVector4D(1.0, 0.7, 0.1, 1.0)));
		paintSteps.emplace_back(
		    new GridViewer<R, typename remove_reference<decltype(
		                          getScenario().getWorld().getCellGrid())>::type>(
		        "Cells grid", [](R *r) {
			        return r->getScenario().getWorld().getCellGridPtr();
			      }, ":shaders/mvp.vert", ":/shaders/flat.frag"));
		paintSteps.emplace_back(
		    new GridViewer<R, typename remove_reference<decltype(
		                          getScenario().getWorld().getModelGrid())>::type>(
		        "Models grid", [](R *r) {
			        return r->getScenario().getWorld().getModelGridPtr();
			      }, ":shaders/mvp.vert", ":/shaders/flat.frag"));
		for (auto &p : plugins_onLoad) p(this);
	}

	void applyInterfaceAdditions(SignalSlotBase *b) {
		QObject *root = b->parentItem();
		for (auto &b : buttons) {
			auto &bt = b.second;
			if (bt.needsToBeUpdated()) {
				QMetaObject::invokeMethod(root, "addButton", Q_ARG(QVariant, bt.getName()),
				                          Q_ARG(QVariant, bt.getMenu()),
				                          Q_ARG(QVariant, bt.getLabel()),
				                          Q_ARG(QVariant, bt.getColor()));
				bt.updateOK();
			}
		}
		if (paintStepsNeedsUpdate) {
			for (auto &ps : paintSteps) {
				QMetaObject::invokeMethod(
				    root, "addPaintStepComponent", Q_ARG(QVariant, ps->name),
				    Q_ARG(QVariant, ps->category), Q_ARG(QVariant, ps->checkable));
			}
			paintStepsNeedsUpdate = false;
		}
	}
	// called after every frame, thread safe
	// synchronization between Qt threads
	virtual void sync(SignalSlotBase *b) {
		applyInterfaceAdditions(b);

		// loop
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
		keyPress = b->keyPress;
		keyDown = b->keyDown;
		b->keyPress.clear();
	}

	/***********************************
	 *              EVENTS              *
	 ***********************************/
	// events handling routine
	void processEvents() {
		const vector<Qt::MouseButton> acceptedButtons = {
		    {Qt::LeftButton, Qt::RightButton, Qt::MiddleButton}};
		// mouse drag (mouse down)
		for (auto &b : acceptedButtons) {
			if (mousePressedButtons.testFlag(b) && mouseDragMethods.count(b)) {
				mouseDragMethods[b](this);
			}
		}
		// mouse click
		for (auto &b : acceptedButtons) {
			if (mouseClickedButtons.testFlag(b) && mouseClickMethods.count(b)) {
				mouseClickMethods[b](this);
			}
		}
		// keyboard press (only once per key press)
		for (auto &k : keyPress) {
			if (keyPressMethods.count(k)) {
				keyPressMethods.at(k)(this);
			}
		}
		// keyboard down (key is down)
		for (auto &k : keyDown) {
			if (keyDownMethods.count(k)) {
				keyDownMethods.at(k)(this);
			}
		}
		// buttons
		for (auto &bName : clickedButtons) {
			if (buttons.count(bName)) {
				buttons[bName].clicked(this);
			}
		}
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

	virtual void paint() {
		processEvents();
		viewMatrix = camera.getViewMatrix();
		projectionMatrix = camera.getProjectionMatrix((float)viewportSize.width() /
		                                              (float)viewportSize.height());
		updateScenario();

		GL->glDepthMask(true);
		GL->glClearColor(1.0, 1.0, 1.0, 1.0);
		GL->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		GL->glEnable(GL_DEPTH_TEST);
		for (auto &p : plugins_preDraw) p(this);

		for (auto &s : paintSteps) {
			if (enabledPaintSteps.count(s->name)) s->call(this);
		}

		for (auto &p : plugins_postDraw) p(this);

		updateStats();
		if (window) {
			window->resetOpenGLState();
			window->update();
		}
	}

	void updateStats() {
		auto t1 = std::chrono::high_resolution_clock::now();
		auto fpsDt = t1 - tfps;
		nbFramesSinceLastTick++;
		if (fpsDt.count() > fpsRefreshRate) {
			stats["fps"] = (double)nbFramesSinceLastTick / (double)fpsDt.count();
			nbFramesSinceLastTick = 0;
			tfps = chrono::high_resolution_clock::now();
		}
		stats["nbCells"] = QVariant((int)scenario.getWorld().cells.size());
		stats["nbUpdates"] = scenario.getWorld().getNbUpdates();
		if (window) {
			window->resetOpenGLState();
		}
		std::chrono::duration<double> dv = t1 - t0;
		viewDt = dv.count();
		t0 = std::chrono::high_resolution_clock::now();
		camera.updatePosition(viewDt);
		++frame;
	}

	// called on redimensioning events.
	void setViewportSize(const QSize &s) {
		viewportSize = s;
		if (window) screenScaleCoef = window->devicePixelRatio();
		for (auto &sm : screenManagers) {
			sm->screenChanged(this);
		}
	}

	void updateScenario() {
		if (loopStep || worldUpdate) {
			for (auto &p : plugins_preLoop) p(this);
			scenario.loop();
			if (!selectedCellStillExists()) selectedCell = nullptr;
			loopStep = false;
		}
	}

 public:
	/**************************
	 *           SET
	 **************************/
	void setCurrentFBO(QOpenGLFramebufferObject *fbo) { currentFBO = fbo; }
	void setSelectedCell(Cell *c) { selectedCell = c; }
	/**************************
	 *           GET
	 **************************/
	Scenario &getScenario() { return scenario; }
	const QMatrix4x4 &getViewMatrix() { return viewMatrix; }
	const QMatrix4x4 &getProjectionMatrix() { return projectionMatrix; }
	double getTimeSinceLastFrame() { return viewDt; }
	Camera &getCamera() { return camera; }
	Cell *getSelectedCell() { return selectedCell; }
	QSize getViewportSize() { return viewportSize; }
	QOpenGLFramebufferObject *getCurrentFBO() { return currentFBO; }
	float getScreenScaleCoef() { return screenScaleCoef; }
	unsigned long getCurrentFrame() { return currentFrameNumber; }
	bool isFullscreen() { return fullscreenMode; }
	unsigned long getLeftMenuSize() { return leftMenuSize; }
	bool selectedCellStillExists() {
		return (std::find(scenario.getWorld().cells.begin(), scenario.getWorld().cells.end(),
		                  selectedCell) != scenario.getWorld().cells.end());
	}

	/*************************
	 *    UI ADDITIONS
	 *************************/
	template <typename P> void registerPlugin(P &p) {
		qDebug() << "registering a plugin";
		loadPluginHooks(this, p);
	}
	void addKeyDownMethod(Qt::Key k, Rfunc f) { keyDownMethods[k] = f; }
	void addKeyPressMethod(Qt::Key k, Rfunc f) { keyPressMethods[k] = f; }
	void addMouseDragMethod(Qt::MouseButton b, Rfunc f) { mouseDragMethods[b] = f; }
	void addMouseClickMethod(Qt::MouseButton b, Rfunc f) { mouseClickMethods[b] = f; }
	QPointF getMousePosition() { return mousePosition; }
	QPointF getPreviousMousePosition() { return mousePrevPosition; }
	void addButton(Button<R> b) { buttons[b.getName()] = b; }
	void addButton(std::string name, std::string menu, std::string label,
	               std::function<void(R *, Button<R> *)> onClicked) {
		Button<R> b(QString::fromStdString(name), QString::fromStdString(menu),
		            QString::fromStdString(label), onClicked);
		buttons[QString::fromStdString(name)] = b;
	}

	int exec() {
		QGuiApplication app(argc, argv);
		app.setQuitOnLastWindowClosed(true);
		QQuickView view;
		view.setFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint |
		              Qt::WindowTitleHint | Qt::WindowCloseButtonHint |
		              Qt::WindowFullscreenButtonHint);
		view.setSurfaceType(QSurface::OpenGLSurface);
		view.setColor(QColor(Qt::transparent));
		view.setClearBeforeRendering(true);
		view.setResizeMode(QQuickView::SizeRootObjectToView);
		qmlRegisterType<SignalSlotBase>("SceneGraphRendering", 1, 0, "Renderer");
		view.setSource(QUrl("qrc:/main.qml"));
		QObject *root = view.rootObject();
		SignalSlotBase *ssb = root->findChild<SignalSlotBase *>("renderer");
		view.rootContext()->setContextProperty("glview", ssb);
		ssb->init(this);
		view.show();
		return app.exec();
	}
};
}
#endif
