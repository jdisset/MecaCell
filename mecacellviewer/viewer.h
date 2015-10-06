#ifndef QTVIEWER_H
#define QTVIEWER_H
#include "viewtools.h"
#include "renderer.hpp"
#include <QQuickView>
#include <QQmlContext>
#include <QGuiApplication>
#include <QApplication>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QQuickItem>
#include <QQmlContext>
#include "slotsignalbase.h"
#include <iostream>
#include <QThread>
#include <Qt>
#include <QWindow>
#include <QQuickWidget>

#define MECACELL_VIEWER
#include "macros.h"

using namespace std;
namespace MecacellViewer {
template <typename Scenario, typename... Plugins> class Viewer {
 public:
	Viewer() {
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
	};

	int exec(int argc, char **argv) {
		QGuiApplication app(argc, argv);
		app.setQuitOnLastWindowClosed(true);
		QQuickView view;
		view.setFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint |
		              Qt::WindowTitleHint | Qt::WindowCloseButtonHint |
		              // Qt::MaximizeUsingFullscreenGeometryHint |
		              Qt::WindowFullscreenButtonHint);
		view.setSurfaceType(QSurface::OpenGLSurface);
		view.setColor(QColor(Qt::transparent));
		view.setClearBeforeRendering(true);
		view.setResizeMode(QQuickView::SizeRootObjectToView);
		qmlRegisterType<SignalSlotBase>("SceneGraphRendering", 1, 0, "Renderer");
		view.setSource(QUrl("qrc:/main.qml"));
		QObject *root = view.rootObject();
		SignalSlotBase *ssb = root->findChild<SignalSlotBase *>("renderer");
		unique_ptr<SignalSlotRenderer> r = unique_ptr<Renderer<Scenario, Plugins...>>(
		    new Renderer<Scenario, Plugins...>(argc, argv));
		view.rootContext()->setContextProperty("glview", ssb);
		ssb->init(r);
		view.show();
		return app.exec();
	}
};
}
#endif
