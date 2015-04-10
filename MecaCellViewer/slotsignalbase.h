#ifndef SLOTSIGNALBASE_H
#define SLOTSIGNALBASE_H
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QQuickView>
#include <QQuickItem>
#include <iostream>
#include <map>
using namespace std;

enum mouseButton { none = 0, leftButton = 1, middleButton = 4, rightButton = 2 };

class SignalSlotBase;

class SignalSlotRenderer : public QObject {
	Q_OBJECT
 protected:
	QSize viewportSize;
	QOpenGLFunctions* GL = nullptr;

 public:
	explicit SignalSlotRenderer() {}
	virtual void sync(SignalSlotBase*){};
	virtual void paint(){};
 public slots:
	virtual void setViewportSize(const QSize&){};
	virtual void paintSlot() {
		if (!GL) {
			GL = QOpenGLContext::currentContext()->functions();
			GL->initializeOpenGLFunctions();
		}
		paint();
	};
};

/***********************************
 *       QML INTERFACE CLASS       *
 ***********************************/
class SignalSlotBase : public QQuickItem {
	Q_OBJECT
 public:
	SignalSlotBase()
	    : mouseMovements(vector<QVector2D>(4, QVector2D(0, 0))), mousePressed(vector<bool>(4, false)) {
		connect(this, SIGNAL(windowChanged(QQuickWindow*)), this, SLOT(handleWindowChanged(QQuickWindow*)));
	}
 public slots:

	/**************************
	 *      Qt events
	 *************************/
	virtual void sync() { 
		renderer->sync(this);
 	};

	void init(unique_ptr<SignalSlotRenderer>& r) {
		renderer = move(r);
		connect(window(), SIGNAL(beforeRendering()), renderer.get(), SLOT(paintSlot()), Qt::DirectConnection);
	}

	void callUpdate() {
		if (window()) window()->update();
	}

	virtual void handleWindowChanged(QQuickWindow* win) {
		if (win) {
			connect(win, SIGNAL(beforeSynchronizing()), this, SLOT(sync()), Qt::DirectConnection);
			win->setClearBeforeRendering(false);
		}
	}

	/**************************
	 *      basic stats
	 *************************/
	qreal getFps() { return fps; }
	int getNbCells() { return nbCells; }
	int getNbUpdates() { return nbUpdates; }

	/**************************
	 *      input events
	 *************************/
	void keyDown(Qt::Key k) { pressedKeys[k] = true; }
	void keyUp(Qt::Key k) { pressedKeys[k] = false; }
	void mouseMove(int b, int x, int y) { mouseMovements[b] += QVector2D(x, y); }
	void mouseWheelRotation(int n) { mouseWheel += n; }
	void pressed(int b, bool v) { mousePressed[b] = v; }
	void setMousePos(int x, int y) { mousePosition = QVector2D(x, y); }
	void click(int b, int x, int y) {
		clicked = (mouseButton)b;
		mousePosition = QVector2D(x, y);
	}

 protected:
	int nbCells = 0;
	int nbUpdates = 0;
	int fps = 60;
	int clicked = 0;
	map<Qt::Key, bool> pressedKeys;
	vector<QVector2D> mouseMovements;
	vector<bool> mousePressed;
	int mouseWheel = 0;
	QVector2D mousePosition;
	unique_ptr<SignalSlotRenderer> renderer = nullptr;
};

#endif
