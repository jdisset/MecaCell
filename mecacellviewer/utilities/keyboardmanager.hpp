#ifndef KEYBOARDMANAGER_HPP
#define KEYBOARDMANAGER_HPP
#include <QDebug>
#include <Qt>
struct KeyboardManager {
	template <typename R> void onLoad(R* renderer) {
		qDebug() << " KEYBOARD MANAGER LOADED";
		renderer->addKeyDownMethod(Qt::Key_Q,
		                    [](R* r) { r->getCamera().left(r->getTimeSinceLastFrame()); });
		renderer->addKeyDownMethod(Qt::Key_D,
		                    [](R* r) { r->getCamera().right(r->getTimeSinceLastFrame()); });
		renderer->addKeyDownMethod(Qt::Key_Z,
		                    [](R* r) { r->getCamera().forward(r->getTimeSinceLastFrame()); });
		renderer->addKeyDownMethod(
		    Qt::Key_S, [](R* r) { r->getCamera().backward(r->getTimeSinceLastFrame()); });
	}
};

#endif
