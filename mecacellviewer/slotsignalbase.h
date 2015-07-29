#ifndef SLOTSIGNALBASE_H
#define SLOTSIGNALBASE_H
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QQuickView>
#include <QQuickItem>
#include <QVariant>
#include <QString>
#include <iostream>
#include <map>
#include <QThread>
#include "viewtools.h"
#include <set>

using namespace std;

namespace MecacellViewer {
class SignalSlotBase;

enum colorMode { owncolor, pressure };
class SignalSlotRenderer : public QObject {

	Q_OBJECT

protected:
	QSize viewportSize;
	QQuickWindow *window = nullptr;

public:
	explicit SignalSlotRenderer() {}
	virtual void sync(SignalSlotBase *){};
	virtual void paint(){};
	virtual void initialize(){};
	void setWindow(QQuickWindow *w) { window = w; }
public slots:
	virtual void cleanupSlot() {}
	virtual void setViewportSize(const QSize &s) { viewportSize = s; };
	virtual void paintSlot() { paint(); };
	virtual SignalSlotRenderer *clone() { return new SignalSlotRenderer(); }
};

/***********************************
 *       QML INTERFACE CLASS       *
 ***********************************/
class SignalSlotBase : public QQuickItem {
	Q_OBJECT
public:
	SignalSlotBase() {
		connect(this, SIGNAL(windowChanged(QQuickWindow *)), this, SLOT(handleWindowChanged(QQuickWindow *)));
		setAcceptedMouseButtons(Qt::AllButtons);
		setFlags(ItemClipsChildrenToShape | ItemAcceptsInputMethod | ItemHasContents);
		guiCtrl["tool"] = "move";
	}

	Q_PROPERTY(QVariantMap stats READ getStats WRITE setStats NOTIFY statsChanged)
	Q_PROPERTY(QVariantMap guiCtrl READ getGuiCtrl WRITE setGuiCtrl NOTIFY GuiCtrlChanged)

	QVariantMap stats, guiCtrl;
	bool worldUpdate = true;
	bool loopStep = false;
	set<int> pressedKeys;
	set<int> inputKeys;
	int mouseWheel = 0;
	unique_ptr<SignalSlotRenderer> renderer = nullptr;
	bool initialized = false;
	QMouseEvent lastMouseEvent =
	    QMouseEvent(QEvent::None, QPointF(0, 0), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
	QFlags<Qt::MouseButtons> mouseClickedButtons, mouseDblClickedButtons;

	virtual void mouseMoveEvent(QMouseEvent *event) { lastMouseEvent = *event; }
	virtual void mousePressEvent(QMouseEvent *event) {
		lastMouseEvent = *event;
		mouseClickedButtons |= event->button();
	}

	virtual void mouseReleaseEvent(QMouseEvent *event) { lastMouseEvent = *event; }
	virtual void mouseDoubleClickEvent(QMouseEvent *event) { mouseDblClickedButtons |= event->button(); }
	virtual void wheelEvent(QWheelEvent *) {}

	virtual void keyPressEvent(QKeyEvent *event) {
		pressedKeys.insert(event->key());
		inputKeys.insert(event->key());
	}
	virtual void keyReleaseEvent(QKeyEvent *event) { pressedKeys.erase(event->key()); }

signals:
	void statsChanged();
	void GuiCtrlChanged();

public slots:
	/**************************
	 *      Qt events
	 *************************/
	virtual void sync() {
		if (!initialized) {
			initialized = true;
			renderer.reset(renderer->clone());
			connect(window(), SIGNAL(beforeRendering()), renderer.get(), SLOT(paintSlot()), Qt::DirectConnection);
			connect(window(), SIGNAL(sceneGraphInvalidated()), renderer.get(), SLOT(cleanupSlot()),
			        Qt::DirectConnection);
			renderer->initialize();
		}
		renderer->setWindow(window());
		renderer->setViewportSize(QSize(width(), height()));
		renderer->sync(this);
	};

	void init(unique_ptr<SignalSlotRenderer> &r) { renderer = move(r); }

	void callUpdate() {
		if (window()) window()->update();
	}
	void step() { loopStep = true; }
	void setWorldUpdate(bool u) { worldUpdate = u; }

	virtual void handleWindowChanged(QQuickWindow *win) {
		if (win) {
			connect(win, SIGNAL(beforeSynchronizing()), this, SLOT(sync()), Qt::DirectConnection);
			win->setClearBeforeRendering(false);
		}
	}

	/**************************
	 *      basic stats
	 *************************/
	QVariant getGuiCtrl(QString k) { return guiCtrl.count(k) ? guiCtrl[k] : 0; }
	QVariantMap getGuiCtrl() { return guiCtrl; }
	QVariantMap getStats() { return stats; }
	QVariant getStat(const QString &name) { return stats.count(name) ? stats[name] : QVariant(); }
	void setGuiCtrl(QVariantMap c) { guiCtrl = c; }
	void setStats(QVariantMap s) { stats = s; }
};
}
#endif
