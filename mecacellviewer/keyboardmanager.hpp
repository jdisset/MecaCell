#ifndef KEYBOARDMANAGER_HPP
#define KEYBOARDMANAGER_HPP
#include <Qt>
#include <QDebug>
struct KeyboardManager {
	template <typename R> void onLoad(R* r) {
		r->addKeyDownMethod(Qt::Key_Q,
		                    [](R* r) { r->getCamera().left(r->getTimeSinceLastFrame()); });
		r->addKeyDownMethod(Qt::Key_D,
		                    [](R* r) { r->getCamera().right(r->getTimeSinceLastFrame()); });
		r->addKeyDownMethod(Qt::Key_Z,
		                    [](R* r) { r->getCamera().forward(r->getTimeSinceLastFrame()); });
		r->addKeyDownMethod(
		    Qt::Key_S, [](R* r) { r->getCamera().backward(r->getTimeSinceLastFrame()); });
	}
};

#endif
