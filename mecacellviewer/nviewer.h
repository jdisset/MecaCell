#ifndef MECACELLVIEWER_H
#define MECACELLVIEWER_H
#include <QMatrix4x4>
#include <QOpenGLFramebufferObject>
#include <QPointF>
#include <QSize>
#include <QSurfaceFormat>
#include <functional>
#include "camera.hpp"
#include "managers/keyboardmanager.h"
#include "signalslotbase.h"
#include "utilities/mousemanager.h"

namespace MecacellViewer {
template <typename Scenario> class Viewer : public SignalSlotRenderer {
	friend class SignalSlotBase;

 public:
	using World =
	    typename remove_reference<decltype(((Scenario *)nullptr)->getWorld())>::type;
	using Cell = typename World::cell_t;
	using Vec = decltype(((Cell *)nullptr)->getPosition());
	using R = Viewer<Scenario>;
	using Rfunc = std::function<void(R *)>;
	using ButtonType = Button<R>;

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
	int nbLoopsPerFrame = 1;

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

	// Stats
	std::chrono::time_point<std::chrono::high_resolution_clock> t0, tfps;
	double viewDt;
	int nbFramesSinceLastTick = 0;
	unsigned long currentFrameNumber = 0;
	Cell *selectedCell = nullptr;
	bool worldUpdate = false;
	bool loopStep = false;
	double fpsRefreshRate = 0.4;
	QVariantMap guiCtrl, stats;
	QList<QVariant> enabledPaintSteps;
	std::vector<std::pair<QList<QVariant>, bool>> displayMenuToggled;

	MenuElement<R> displayMenu;
	bool displayMenuChanged = true;
	ColorMode currentColorMode = color_normal;

	QQuickWindow *view;
	QQmlApplicationEngine *engine;

 public:
	std::vector<Rfunc> plugins_preLoad;
	std::vector<Rfunc> plugins_onLoad;
	std::vector<Rfunc> plugins_preLoop;
	std::vector<Rfunc> plugins_preDraw;
	std::vector<Rfunc> plugins_postDraw;
	std::vector<Rfunc> plugins_onSync;

 private:
	std::map<Qt::Key, Rfunc> keyDownMethods;
	std::map<Qt::Key, Rfunc> keyPressMethods;
	std::map<Qt::MouseButton, Rfunc> mouseDragMethods;
	std::map<Qt::MouseButton, Rfunc> mouseClickMethods;
	std::map<QString, Button<R>> buttons;

	// this is just so we can store paint steps instances without making a mess
	std::map<QString, unique_ptr<PaintStep<R>>> paintSteps;

	// the actual paint steps method to be called
	std::map<int, Rfunc> paintStepsMethods;

	bool paintStepsNeedsUpdate =
	    true;  // do we need to refresh the list of checkable paint steps?

	// screen managers might affect the display. Usually manipulate fbos
	// Inherit from paintStep because they also usually need to be called
	// during painting. Ex: screen space ambient oclusion defines some fbos,
	// makes operations on them and draw an object (a texture) to the screen.
	std::vector<ScreenManager<R> *> screenManagers;

	// init function for the renderer. Create all the defaults paint steps and
	// screen managers, initializes scenario and users additions.
	using psptr = std::unique_ptr<PaintStep<R>>;
	virtual void initialize(QQuickWindow *wdw) {
		MenuElement<R> cellsMenu = {
		    "Cells",
		    {
		        {"Mesh type",
		         elementType::exclusiveGroup,
		         {
		             {"None", false}, {"Centers only", false}, {"Sphere", false},
		         }},
		        {"Display connections", false},
		    }};

		this->window = wdw;
		viewportSize = QSize(static_cast<int>(wdw->width()), static_cast<int>(wdw->height()));
		scenario.init(argc, argv);
		GL = QOpenGLContext::currentContext()->functions();
		GL()->initializeOpenGLFunctions();
		////////////////////////////////
		// list of default paint steps
		/////////////////////////////////
		paintSteps.emplace("MSAA", psptr(new MSAA<R>(this)));
		paintSteps.emplace("Skybox", psptr(new Skybox<R>()));
		paintSteps.emplace("SphereCells", psptr(new CellGroup<R>()));
		paintSteps.emplace("Connections", psptr(new ConnectionsGroup<R>()));
		paintSteps.emplace("SSAO", psptr(new SSAO<R>(this)));
		paintSteps.emplace("Blur", psptr(new MenuBlur<R>(this)));
		screenManagers.push_back(dynamic_cast<ScreenManager<R> *>(paintSteps["MSAA"].get()));
		screenManagers.push_back(dynamic_cast<ScreenManager<R> *>(paintSteps["SSAO"].get()));
		screenManagers.push_back(dynamic_cast<ScreenManager<R> *>(paintSteps["Blur"].get()));

		cellsMenu.onToggled = [&](R *r, MenuElement<R> *me) {
			if (me->isChecked()) {
				if (me->at("Mesh type").at("Sphere").isChecked()) {
					paintStepsMethods[10] = [&](R *r) {
						CellGroup<R> *cells =
						    dynamic_cast<CellGroup<R> *>(paintSteps["SphereCells"].get());
						cells->call(r, false, r->currentColorMode);
					};
				} else if (me->at("Mesh type").at("Centers only").isChecked()) {
					paintStepsMethods[10] = [&](R *r) {
						CellGroup<R> *cells =
						    dynamic_cast<CellGroup<R> *>(paintSteps["SphereCells"].get());
						cells->call(r, true, r->currentColorMode);
					};
				} else
					paintStepsMethods.erase(10);
			} else
				paintStepsMethods.erase(10);
		};

		cellsMenu.at("Display connections").onToggled = [&](R *r, MenuElement<R> *me) {
			if (me->isChecked()) {
				paintStepsMethods[17] = [&](R *r) {
					ConnectionsGroup<R> *connections =
					    dynamic_cast<ConnectionsGroup<R> *>(paintSteps["Connections"].get());
					connections->template draw<Cell>(
					    r->getScenario().getWorld().getConnectedCellsList(), r->getViewMatrix(),
					    r->getProjectionMatrix());
				};
			} else
				paintStepsMethods.erase(17);
		};
		MenuElement<R> ssaoPostproc = {"SSAO"};
		ssaoPostproc.onToggled = [&](R *r, MenuElement<R> *me) {
			if (me->isChecked()) {
				paintStepsMethods[1000000] = [&](R *r) { paintSteps["SSAO"]->call(r); };
			} else {
				paintStepsMethods[1000000] = [&](R *r) {
					dynamic_cast<SSAO<R> *>(paintSteps["SSAO"].get())->callDumb(r);
				};
			}
		};

		MenuElement<R> postProcMenu = {"Post processing", {ssaoPostproc}};
		displayMenu = {"Enabled elements", {cellsMenu, postProcMenu}};

		// non disablable elements
		paintStepsMethods[0] = [&](R *r) { paintSteps["MSAA"]->call(r); };
		paintStepsMethods[5] = [&](R *r) { paintSteps["Skybox"]->call(r); };
		paintStepsMethods[2000000] = [&](R *r) { paintSteps["Blur"]->call(r); };

		for (auto &p : plugins_onLoad) p(this);
		displayMenu.callAll(this);
	}

	/**
	 * @brief the method to call when starting the view.
	 *
	 * @return app's error code
	 */
	int exec() {
		QGuiApplication app(argc, argv);
		app.setQuitOnLastWindowClosed(true);

		qmlRegisterType<SignalSlotBase>("SceneGraphRendering", 1, 0, "Renderer");
		engine = new QQmlApplicationEngine((QUrl("qrc:/main.qml")));

		QObject *root = engine->rootObjects().first();

		view = qobject_cast<QQuickWindow *>(root);
		view->setFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint |
		               Qt::WindowTitleHint | Qt::WindowCloseButtonHint |
		               Qt::WindowFullscreenButtonHint);
		SignalSlotBase *ssb = root->findChild<SignalSlotBase *>("renderer");

		engine->rootContext()->setContextProperty("glview", ssb);
		ssb->init(this);
		view->show();
		for (auto &p : plugins_preLoad) p(this);
		QObject::connect(view, SIGNAL(closing(QQuickCloseEvent *)), &app, SLOT(quit()));
		return app.exec();
	}
	// updates Interface Additions (new buttons, new menu, ...)
	void applyInterfaceAdditions(SignalSlotBase *b) {
		QObject *root = b->window();
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
		if (displayMenuChanged) {
			QMetaObject::invokeMethod(root, "createDisplayMenu",
			                          Q_ARG(QVariant, displayMenu.toJSON()));
			displayMenuChanged = false;
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

		guiCtrl = b->getGuiCtrl();

		// stats
		if (selectedCell)
			stats["selectedCell"] = cellToQVMap(selectedCell);
		else
			stats.remove("selectedCell");
		b->setStats(stats);
		b->statsChanged();

		// menu
		displayMenuToggled = b->displayMenuToggled;
		b->displayMenuToggled.clear();
		displayMenu.updateCheckedFromList(this, displayMenuToggled);
		if (displayMenuToggled.size() > 0) displayMenu.callAll(this);

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
		processEvents(b);
		for (auto &p : plugins_postDraw) p(this);
	}

	/***********************************
	 *              EVENTS              *
	 ***********************************/
	// events handling routine
	void processEvents(SignalSlotBase *b) {
		const vector<Qt::MouseButton> acceptedButtons = {
		    {Qt::LeftButton, Qt::RightButton, Qt::MiddleButton}};
		// mouse drag (mouse down)
		for (const auto &b : acceptedButtons)
			if (mousePressedButtons.testFlag(b) && mouseDragMethods.count(b))
				mouseDragMethods[b](this);
		// mouse click
		for (const auto &b : acceptedButtons)
			if (mouseClickedButtons.testFlag(b) && mouseClickMethods.count(b))
				mouseClickMethods[b](this);
		// keyboard press (only once per key press)
		for (const auto &k : keyPress)
			if (keyPressMethods.count(k)) keyPressMethods.at(k)(this);
		// keyboard down (key is down)
		for (const auto &k : keyDown)
			if (keyDownMethods.count(k)) keyDownMethods.at(k)(this);
		// buttons
		for (const auto &bName : b->clickedButtons)
			if (buttons.count(bName)) buttons[bName].clicked(this);

		b->clickedButtons.clear();
	}

	/***************************************************
	 * ** ** ** ** *         PAINT       * ** ** ** ** *
	 **************************************************/

	/**
	 * @brief main paint routine
	 */
	virtual void paint() {
		viewMatrix = camera.getViewMatrix();
		projectionMatrix = camera.getProjectionMatrix((float)viewportSize.width() /
		                                              (float)viewportSize.height());
		// updating scenario
		if (loopStep || worldUpdate) {
			for (auto &p : plugins_preLoop) p(this);  // preLoop Hook
			for (int i = 0; i < nbLoopsPerFrame; ++i) scenario.loop();
			if (!selectedCellStillExists()) selectedCell = nullptr;
			loopStep = false;
		}

		for (auto &p : plugins_preDraw) p(this);           // preDraw hook
		for (auto &s : paintStepsMethods) s.second(this);  // Renderable plugins

		updateStats();
		if (window) {
			window->resetOpenGLState();
			window->update();
		}
	}

	/**
	 * @brief Handles the update of various stats & variables, such as the number of frames
	 * per seconds or the currently selected cell (check if it was'nt destroyed since last
	 * frame)
	 */
	void updateStats() {
		auto t1 = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> fpsDt = t1 - tfps;
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

	/**
	 * @brief  called on redim events
	 *
	 * @param s the new viewport size
	 */
	void setViewportSize(const QSize &s) {
		viewportSize = s;
		screenScaleCoef = 1.0;  // window->devicePixelRatio();
		for (auto &sm : screenManagers) {
			sm->screenChanged(this);
		}
	}

 public:
	/**************************
	 *           SET
	 **************************/

	/**
	 * @brief changes the frame buffer object currently in use
	 *
	 * @param fbo the new FBO
	 */
	void setCurrentFBO(QOpenGLFramebufferObject *fbo) { currentFBO = fbo; }

	/**
	 * @brief set the currently selected cell
	 *
	 * @param c a pointer to the cell to be considered selected
	 */
	void setSelectedCell(Cell *c) { selectedCell = c; }

	/**
	 * @brief pauses calls to the scenario loop
	 */
	void pause() {
		worldUpdate = false;
		loopStep = false;
	}

	/**
	 * @brief restarts calling the scenario loop at each frame
	 */
	void play() { worldUpdate = true; }

	/**
	 * @brief sets the number of times the loop method of the scenario must be called at
	 * each frame. Defaults = 1.
	 *
	 * @param n
	 */
	void setNbLoopsPerFrame(int n) { nbLoopsPerFrame = n; }

	/**************************
	 *           GET
	 **************************/
	/**
	 * @brief
	 *
	 * @return the current frame number
	 */
	int getFrame() { return frame; }

	/**
	 * @brief
	 *
	 * @return a reference to the scenario
	 */
	Scenario &getScenario() { return scenario; }

	/**
	 * @brief
	 *
	 * @return a reference to the current view matrix
	 */
	const QMatrix4x4 &getViewMatrix() { return viewMatrix; }

	/**
	 * @brief
	 *
	 * @return a reference to the current projection matrix
	 */
	const QMatrix4x4 &getProjectionMatrix() { return projectionMatrix; }

	/**
	 * @brief
	 *
	 * @return the time in seconds since the last frame
	 */
	double getTimeSinceLastFrame() { return viewDt; }

	/**
	 * @brief
	 *
	 * @return the current viewport size
	 */
	QSize getViewportSize() { return viewportSize; }

	/**
	 * @brief
	 *
	 * @return a reference to the camera instance
	 */
	Camera &getCamera() { return camera; }

	/**
	 * @brief
	 *
	 * @return a pointer to the currently selected cell. nullptr when none is selected
	 */
	Cell *getSelectedCell() { return selectedCell; }

	/**
	 * @brief
	 *
	 * @return a pointer to the current frame buffer object in use
	 */
	QOpenGLFramebufferObject *getCurrentFBO() { return currentFBO; }

	/**
	 * @brief
	 *
	 * @return the current screen scale coefficient (usually 1, sometimes 2 on retina
	 * displays for ex.)
	 */
	float getScreenScaleCoef() { return screenScaleCoef; }

	/**
	 * @brief is mecacell running in fullscreen mode?
	 *
	 * @return
	 */
	bool isFullscreen() { return fullscreenMode; }

	/**
	 * @brief
	 *
	 * @return the width of the left side menu, in pixels
	 */
	unsigned long getLeftMenuSize() { return leftMenuSize; }

	/**
	 * @brief is the selected cell still in the world? (Could have been destroyed at the
	 * previous frame)
	 *
	 * @return
	 */
	bool selectedCellStillExists() {
		return (std::find(scenario.getWorld().cells.begin(), scenario.getWorld().cells.end(),
		                  selectedCell) != scenario.getWorld().cells.end());
	}

	/**
	 * @brief
	 *
	 * @return a pointer to the top-level menu element
	 */
	MenuElement<R> *getDisplayMenu() { return &displayMenu; }

	/**
	 * @brief adds a paint step that will be called at each frame. Paint steps contains raw
	 * calls to the OpenGL API.
	 *
	 * @param priority is used to determine the calling order of all the paint steps (by
	 * ascending order)
	 * @param f the function (often a lambda) that takes a pointer to the viewer instance as
	 * argument
	 */
	void addPaintStepsMethods(int priority, Rfunc f) {
		paintStepsMethods[priority] = std::move(f);
	}

	/**
	 * @brief removes the paint steps at a given priority
	 *
	 * @param priority
	 */
	void erasePaintStepsMethods(int priority) { paintStepsMethods.erase(priority); }

	/**
	 * @brief
	 *
	 * @return the coordinates of the mouse pointer
	 */
	QPointF getMousePosition() { return mousePosition; }

	/**
	 * @brief
	 *
	 * @return the coordinates of the mouse pointer at previous frame
	 */
	QPointF getPreviousMousePosition() { return mousePrevPosition; }

	/*************************
	 *    UI ADDITIONS
	 *************************/

	/**
	 * @brief registers a plugin class instance. A plugin class is a class or struct that
	 * contains 1 or more hooks methods. The system is identical to the mecacell's core
	 * library plugin & hook system. Available hooks are:
	 * - preLoad: called before the initialization of the viewer. No valid openGL context is
	 * created yet.
	 * - onLoad: called at the end of the init method. Menu is created and a valid openGL
	 * context is initialized. Usefull hook to register interface additions (menu changes,
	 * new buttons...), event processor (KeyDown, MouseClick, etc...)  or paint steps
	 * method.
	 * - preLoop: called just before the scenario's loop method.
	 * - preDraw: called before the first paint step.
	 * All hooks take a pointer to the viewer instance as argument (type Rfunc :
	 * void(Viewer*) )
	 * @tparam P plugin class type (implicit deduction should work just fine)
	 * @param p the plugin class instance.
	 */
	template <typename P> void registerPlugin(P &p) { loadPluginHooks(this, p); }

	/**
	 * @brief registers a method f to call when the Key k is pushed down
	 *
	 * @param k
	 * @param f
	 */
	void addKeyDownMethod(Qt::Key k, Rfunc f) { keyDownMethods[k] = f; }

	/**
	 * @brief registers a method f to call when the Key k is pressed (down + up)
	 *
	 * @param k
	 * @param f
	 */
	void addKeyPressMethod(Qt::Key k, Rfunc f) { keyPressMethods[k] = f; }

	/**
	 * @brief registers a method f to call when the mouse button b is dragged
	 *
	 * @param
	 * @param f
	 */
	void addMouseDragMethod(Qt::MouseButton b, Rfunc f) { mouseDragMethods[b] = f; }

	/**
	 * @brief registers a method f to call when the mouse button b is clicked
	 *
	 * @param b
	 * @param f
	 */
	void addMouseClickMethod(Qt::MouseButton b, Rfunc f) { mouseClickMethods[b] = f; }

	/**
	 * @brief adds a Button instance to the viewer
	 *
	 * @param b the button instance
	 *
	 * @return
	 */
	Button<R> *addButton(Button<R> b) {
		buttons[b.getName()] = b;
		return &buttons[b.getName()];
	}

	/**
	 * @brief adds and constructs a button
	 *
	 * @param name the name opf the button (so it can be accesses later)
	 * @param menu in which menu should this button appear?
	 * @param label the initial label
	 * @param onClicked a function that will be called when the button is clicked.
	 *
	 * @return
	 */
	Button<R> *addButton(std::string name, std::string menu, std::string label,
	                     std::function<void(R *, Button<R> *)> onClicked) {
		Button<R> b(QString::fromStdString(name), QString::fromStdString(menu),
		            QString::fromStdString(label), onClicked);
		buttons[QString::fromStdString(name)] = b;
		return &buttons[b.getName()];
	}

	/**
	 * @brief
	 *
	 * @param name the name of the button
	 *
	 * @return a pointer to the requested button, nullptr if not found
	 */
	Button<R> *getButton(std::string name) {
		if (buttons.count(QString::fromStdString(name)))
			return &buttons[QString::fromStdString(name)];
		return nullptr;
	}

	/**
	 * @brief
	 *
	 * @return a pointer to the underlying QWindow's instance
	 */
	QQuickWindow *getWindow() { return window; }

	/**
	 * @brief
	 *
	 * @return a pointer to the main view
	 */
	QQuickWindow *getMainView() { return view; }

	/**
	 * @brief
	 *
	 * @return a pointer to the underlying QQmlApplicationEngine's instance
	 */
	QQmlApplicationEngine *getEngine() { return engine; }
};
#endif
