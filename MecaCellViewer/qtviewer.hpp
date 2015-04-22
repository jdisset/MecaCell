#ifndef QTVIEWER_H
#define QTVIEWER_H
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

using namespace std;
namespace MecaCellViewer {
template <typename World, typename Cell> class QtViewer {
	function<void(World&)> init;
	function<void(World&)> loop;

 public:
	QtViewer(function<void(World&)> i, function<void(World&)> l) : init(i), loop(l) {
		// for mac osx
		QSurfaceFormat f;
		f.setProfile(QSurfaceFormat::CoreProfile);
		f.setVersion(4, 1);
		QSurfaceFormat::setDefaultFormat(f);
	};

	int exec(int argc, char** argv) {
		QGuiApplication app(argc, argv);
		app.setQuitOnLastWindowClosed(true);
		// init qml app
		qmlRegisterType<SignalSlotBase>("SceneGraphRendering", 1, 0, "Renderer");
		QQuickView view;
		view.setSource(QUrl("qrc:/main.qml"));
		view.setResizeMode(QQuickView::SizeRootObjectToView);
		QObject* root = view.rootObject();
		SignalSlotBase* ssb = root->findChild<SignalSlotBase*>("renderer");
		unique_ptr<SignalSlotRenderer> r =
		    unique_ptr<Renderer<World, Cell>>(new Renderer<World, Cell>(init, loop));
		view.rootContext()->setContextProperty("glview", ssb);
		ssb->init(r);
		view.show();
		return app.exec();
	}
};
}
#endif
