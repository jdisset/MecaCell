#ifndef QTVIEWER_H
#define QTVIEWER_H
#include "viewtools.h"
#include "renderer.hpp"
#include <QtQuick/QQuickView>
#include <QQmlContext>
#include <QGuiApplication>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QQuickItem>
#include "slotsignalbase.h"
#include <iostream>
#include <QThread>

#define MECACELL_VIEWER
#include "macros.h"

using namespace std;
namespace MecacellViewer {
template <typename Scenario> class Viewer {

public:
	Viewer() {
		initResources();
#if __APPLE__
#include "TargetConditionals.h"
#if TARGET_OS_MAC
		// compatibility profile (Qt's default) is not available on mac os...
		// we have to use a core profile
		QSurfaceFormat f;
		f.setProfile(QSurfaceFormat::CoreProfile);
		f.setVersion(3, 3);
		QSurfaceFormat::setDefaultFormat(f);
#endif
#endif
	};

	int exec(int argc, char **argv) {
		QGuiApplication app(argc, argv);
		app.setQuitOnLastWindowClosed(true);
		qmlRegisterType<SignalSlotBase>("SceneGraphRendering", 1, 0, "Renderer");
		QQuickView view;
		view.setSource(QUrl("qrc:/main.qml"));
		view.setResizeMode(QQuickView::SizeRootObjectToView);
		QObject *root = view.rootObject();
		SignalSlotBase *ssb = root->findChild<SignalSlotBase *>("renderer");
		unique_ptr<SignalSlotRenderer> r = unique_ptr<Renderer<Scenario>>(new Renderer<Scenario>(argc, argv));
		view.rootContext()->setContextProperty("glview", ssb);
		ssb->init(r);
		view.show();
		return app.exec();
	}
};
}
#endif
