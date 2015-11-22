#ifndef SIGNALSLOTBASE_H
#define SIGNALSLOTBASE_H
#include <QQuickItem>
#include <QQuickWindow>
#include <memory>
#include <unordered_set>
#include <set>
#include <iostream>
namespace MecacellViewer {
class SignalSlotBase;
class SignalSlotRenderer : public QObject {
	Q_OBJECT
	friend class SignalSlotBase;

 protected:
	QQuickWindow *window = nullptr;
	virtual void paint() = 0;

 public:
	explicit SignalSlotRenderer() {}
	virtual void sync(SignalSlotBase *) = 0;
	virtual void initialize() = 0;
	virtual void setViewportSize(const QSize &) = 0;
 public slots:
	void paintSlot() { paint(); };
	void cleanupSlot(){};
};

class SignalSlotBase : public QQuickItem {
	Q_OBJECT
 public:
	SignalSlotBase() {
		connect(this, SIGNAL(windowChanged(QQuickWindow *)), this,
		        SLOT(handleWindowChanged(QQuickWindow *)));
		setAcceptedMouseButtons(Qt::AllButtons);
		setFlags(ItemClipsChildrenToShape | ItemAcceptsInputMethod | ItemHasContents);
	}

	Q_PROPERTY(QVariantMap stats READ getStats WRITE setStats NOTIFY statsChanged)

	QVariantMap stats;
	bool worldUpdate = true;
	bool loopStep = false;
	std::set<Qt::Key> keyDown, keyPress;
	int mouseWheel = 0;
	SignalSlotRenderer *renderer = nullptr;
	bool initialized = false;
	QMouseEvent lastMouseEvent = QMouseEvent(QEvent::None, QPointF(0, 0), Qt::NoButton,
	                                         Qt::NoButton, Qt::NoModifier);
	QFlags<Qt::MouseButtons> mouseClickedButtons, mouseDblClickedButtons;
	std::set<QString> clickedButtons;

	/******************************
	 *            INPUTS
	 *****************************/
	// Mouse
	virtual void mouseMoveEvent(QMouseEvent *event) { lastMouseEvent = *event; }
	virtual void mousePressEvent(QMouseEvent *event) {
		lastMouseEvent = *event;
		mouseClickedButtons |= event->button();
	}
	virtual void mouseReleaseEvent(QMouseEvent *event) { lastMouseEvent = *event; }
	virtual void mouseDoubleClickEvent(QMouseEvent *event) {
		mouseDblClickedButtons |= event->button();
	}
	virtual void wheelEvent(QWheelEvent *) {}
	void buttonClick(QString name) { clickedButtons.insert(name); }

	// Keyboard
	virtual void keyPressEvent(QKeyEvent *event) {
		keyDown.insert(static_cast<Qt::Key>(event->key()));
		keyPress.insert(static_cast<Qt::Key>(event->key()));
	}
	virtual void keyReleaseEvent(QKeyEvent *event) {
		keyPress.erase(static_cast<Qt::Key>(event->key()));
	}

 signals:
	void statsChanged();

 public slots:
	//*************************Qt events ***********************
	virtual void sync() {
		if (!initialized) {
			initialized = true;
			connect(window(), SIGNAL(beforeRendering()), renderer, SLOT(paintSlot()),
			        Qt::DirectConnection);
			connect(window(), SIGNAL(sceneGraphInvalidated()), renderer, SLOT(cleanupSlot()),
			        Qt::DirectConnection);
			renderer->initialize();
		}
		renderer->window = window();
		renderer->setViewportSize(
		    QSize(static_cast<int>(width()), static_cast<int>(height())));
		renderer->sync(this);
		update();
	};

	void init(SignalSlotRenderer *r) { renderer = r; }

	void callUpdate() {
		if (window()) window()->update();
	}

	void step() { loopStep = true; }
	void setWorldUpdate(bool u) { worldUpdate = u; }
	virtual void handleWindowChanged(QQuickWindow *win) {
		if (win) {
			connect(win, SIGNAL(beforeSynchronizing()), this, SLOT(sync()),
			        Qt::DirectConnection);
			win->setClearBeforeRendering(false);
		}
	}

	QVariantMap getStats() { return stats; }
	QVariant getStat(const QString &name) {
		return stats.count(name) ? stats[name] : QVariant();
	}
	void setStats(QVariantMap s) { stats = s; }
};
}
#endif
