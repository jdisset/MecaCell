#ifndef RENDERER_HPP
#define RENDERER_HPP
#include "slotsignalbase.h"
#include "paintstep.hpp"
#include "screenmanager.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <functional>

struct Dummy {};

// The renderer glues every plugins together (and is the only class they can access).
// It handles events, scenario control, synchronisation and painting of the openGL scene.
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
	using R = Renderer<Scenario, Plugins...>;

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
	QMap<QString, Button<R>> buttons;

	// Stats
	unsigned long currentFrameNumber = 0;
	Cell *selectedCell = nullptr;

	QMatrix4x4 viewMatrix, projectionMatrix;
	/************************
	 *       PLUGINS
	 ************************/
	std::tuple<Dummy, Plugins...> plugins;
	std::vector<std::function<void()>> plugins_onLoad;
	std::vector<std::function<void()>> plugins_onDraw;
	std::vector<std::function<void()>> plugins_preDraw;
	std::vector<std::function<void()>> plugins_preLoop;
	std::vector<std::function<void()>> plugins_postDraw;

	// paint steps : ordered list of callable paint actions
	// (draw objects, set up effects...)
	std::vector<unique_ptr<PaintStep>> paintSteps;

	// screen managers might affect the display. Usually manipulate fbos
	// Inherit from paintStep because they also usually need to be called
	// during painting. Ex: screen space ambient oclusion defines some fbos,
	// makes operations on them and draw an object (a texture) to the screen.
	std::vector<ScreenManager *> screenManagers;

	// init function for the renderer. Create all the defaults paint steps and
	// screen managers, initializes scenario and users additions.
	virtual void initialize() {
		scenario.init(argc, argv);
		GL = QOpenGLContext::currentContext()->functions();
		GL->initializeOpenGLFunctions();
		////////////////////////////////
		// list of default paint steps
		/////////////////////////////////
		paintSequence.emplace_back(new MSAA(viewportSize));
		paintSequence.emplace_back(new Skybox<R>());
		paintSequence.emplace_back(new CellGroup<R>());
		paintSequence.emplace_back(
		    new ArrowsGroup<R>("Forces", [](R *r) {
			    auto f0 = r->scenario.getWorld().getAllForces();
			    vector<pair<QVector3D, QVector3D>> f;
			    f.reserve(f0.size());
			    for (auto &p : f0) {
				    f.push_back(make_pair(toQV3D(p.first), toQV3D(p.second)));
			    }
			    return f;
			  }), QVector4D(1.0, 0.7, 0.1));
		paintSequence.emplace_back(
		    new ArrowsGroup<R>("Velocities", [](R *r) {
			    auto f0 = r->scenario.getWorld().getAllVelocities();
			    vector<pair<QVector3D, QVector3D>> f;
			    f.reserve(f0.size());
			    for (auto &p : f0) {
				    f.push_back(make_pair(toQV3D(p.first), toQV3D(p.second)));
			    }
			    return f;
			  }), QVector4D(1.0, 0.7, 0.1));
		paintSequence.emplace_back(new GridViewer<R>("Cells grid", [](R *r) {
			                           return r->getScenario().getCellGrid();
			                         }), ":shaders/mvp.vert", ":/shaders/flat.frag");
		paintSequence.emplace_back(new GridViewer<R>("Models grid", [](R *r) {
			                           return r->getScenario().getModelGrid();
			                         }), ":shaders/mvp.vert", ":/shaders/flat.frag");
		forEach(plugins, PluginLoader<R>{this});
		for (auto &p : plugins_onLoad) p();
		for (auto &vo : visualObjects) vo.second.load(this);
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
	// called after every frame, thread safe
	// synchronization between Qt threads
	virtual void sync(SignalSlotBase *b) {
		applyInterfaceAdditions(b);

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
	void processEvents() {}

	// paint method, called for every frame.
	virtual void paint() {
		processEvents();
		viewMatrix = camera.getViewMatrix();
		projectionMatrix = camera.getProjectionMatrix((float)viewportSize.width() /
		                                              (float)viewportSize.height());
		updateScenario();
		prepareScreen();
		for (auto &p : plugins_preDraw) p();
		for (auto &s : paintSequence) {
			if (!s.checkable || enabledPaintSequence.contains(s.name)) s->call(this);
		}
		for (auto &p : plugins_postDraw) p();
	}

	// called on redimensioning events.
	void setViewportSize(const QSize &s) {
		viewportSize = s;
		if (window) screenScaleCoef = window->devicePixelRatio();
		for (auto &sm : screenManagers) {
			sm->screenChanged(this);
		}
	}

 public:
	// Ugly but allows to create a new instance from QSGRenderThread
	// and bypass Qt template restrictions...
	virtual SignalSlotRenderer *clone() { return new R(argc, argv); }

	/**************************
	 *           SET
	 **************************/
	void setCurrentFBO(QOpenGLFramebufferObject *fbo) { currentFBO = fbo; }
	void addButton(Button<R> b) { buttons[b.getName()] = b; }
	void addButton(std::string name, std::string menu, std::string label,
	               std::function<void(R *, Button<R> *)> onClicked) {
		Button<R> b(QString::fromStdString(name), QString::fromStdString(menu),
		            QString::fromStdString(label), onClicked);
		buttonMap[QString::fromStdString(name)] = b;
	}
	/**************************
	 *           GET
	 **************************/
	Scenario &getScenario() { return scenario; }
	const QMatrix4x4 &getViewMatrix() { return viewMatrix; }
	const QMatrix4x4 &getProjectionMatrix() { return projectionMatrix; }
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
};
#endif
