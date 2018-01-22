#ifndef SIGNALSLOTBASE_H
#define SIGNALSLOTBASE_H
#include <QQuickItem>
#include <QQuickWindow>
#include <iostream>
#include <memory>
#include <set>
#include <unordered_set>
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
	virtual void initialize(QQuickWindow *) = 0;
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

	Q_PROPERTY(QVariantMap guiCtrl READ getGuiCtrl WRITE setGuiCtrl NOTIFY guiCtrlChanged)
	Q_PROPERTY(QVariantMap stats READ getStats WRITE setStats NOTIFY statsChanged)

	QVariantMap stats, guiCtrl;
	bool worldUpdate = false;
	bool loopStep = false;
	std::set<Qt::Key> keyDown, keyPress;
	int mouseWheel = 0;
	QSize viewportSize;
	SignalSlotRenderer *renderer = nullptr;
	bool initialized = false;
	QMouseEvent lastMouseEvent = QMouseEvent(QEvent::None, QPointF(0, 0), Qt::NoButton,
	                                         Qt::NoButton, Qt::NoModifier);
	QFlags<Qt::MouseButton> mouseClickedButtons, mouseDblClickedButtons;
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

	// Keyboard
	virtual void keyPressEvent(QKeyEvent *event) {
		if (!event->isAutoRepeat()) {
			keyPress.insert(static_cast<Qt::Key>(event->key()));
			keyDown.insert(static_cast<Qt::Key>(event->key()));
		}
	}
	virtual void keyReleaseEvent(QKeyEvent *event) {
		if (!event->isAutoRepeat())
			keyDown.erase(static_cast<Qt::Key>(event->key()));
	}

	std::vector<std::pair<QList<QVariant>, bool>> displayMenuToggled;
 signals:
	void statsChanged();
	void guiCtrlChanged();

 public slots:
	void displayMenuElementToggled(QVariant l, bool c) {
		displayMenuToggled.emplace_back(l.toList(), c);
	}
	void buttonClick(QString name) { clickedButtons.insert(name); }
	//*************************Qt events ***********************
	virtual void sync() {
		if (initialized) {
			renderer->window = window();
			renderer->sync(this);
			QSize vs = QSize(static_cast<int>(width()), static_cast<int>(height()));
			if (vs != viewportSize) {
				viewportSize = vs;
				renderer->setViewportSize(vs);
			}
			update();
		}
	};

	void initRenderer() {
		renderer->initialize(window());
		initialized = true;
	}

	void init(SignalSlotRenderer *r) {
		renderer = r;
		connect(window(), SIGNAL(beforeRendering()), renderer, SLOT(paintSlot()),
		        Qt::DirectConnection);
		connect(window(), SIGNAL(sceneGraphInvalidated()), renderer, SLOT(cleanupSlot()),
		        Qt::DirectConnection);
	}

	void callUpdate() {
		if (window()) window()->update();
	}

	void step() { loopStep = true; }
	void setWorldUpdate(bool u) { worldUpdate = u; }
	virtual void handleWindowChanged(QQuickWindow *win) {
		if (win) {
			connect(win, SIGNAL(sceneGraphInitialized()), this, SLOT(initRenderer()),
			        Qt::DirectConnection);
			connect(win, SIGNAL(beforeSynchronizing()), this, SLOT(sync()),
			        Qt::DirectConnection);
			win->setClearBeforeRendering(false);
		}
	}

	QVariantMap getGuiCtrl() { return guiCtrl; }
	QVariant getGuiCtrl(const QString &name) {
		return guiCtrl.count(name) ? guiCtrl[name] : QVariant();
	}
	void setGuiCtrl(QVariantMap s) { guiCtrl = s; }
	QVariantMap getStats() { return stats; }
	QVariant getStat(const QString &name) {
		return stats.count(name) ? stats[name] : QVariant();
	}
	void setStats(QVariantMap s) { stats = s; }
};
}
#endif
