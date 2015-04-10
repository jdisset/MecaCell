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


using namespace std;
namespace MecaCellViewer {
template <typename World, typename Cell> class QtViewer {
 protected:
	QGuiApplication app;
	QQuickView view;
	World world;

 public:
	QtViewer(int argc, char** argv) : app(argc, argv) {
		// for mac osx
		QSurfaceFormat f;
		f.setProfile(QSurfaceFormat::CoreProfile);
		f.setVersion(4, 1);
		QSurfaceFormat::setDefaultFormat(f);

		// init qml app
		app.setQuitOnLastWindowClosed(true);
		qmlRegisterType<SignalSlotBase>("SceneGraphRendering", 1, 0, "Renderer");
		view.setSource(QUrl("qrc:/main.qml"));
		view.setResizeMode(QQuickView::SizeRootObjectToView);
		QObject* root = view.rootObject();
		SignalSlotBase* ssb = root->findChild<SignalSlotBase*>("renderer");
		unique_ptr<SignalSlotRenderer> r = unique_ptr<Renderer<World, Cell>> (new Renderer<World, Cell>());
		view.rootContext()->setContextProperty("glview", ssb);
		ssb->init(r);
	};

	int exec() {
		view.show();
		return app.exec();
	}
};
}
#endif
