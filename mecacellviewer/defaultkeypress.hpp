#ifndef DEFAULTKEYPRESS_HPP
#define DEFAULTKEYPRESS_HPP
#include <Qt>
class KeyboardCameraMovements {
	template <typename R> void onLoad(R* r) {
		r->addKeyDownMethod(Qt::Key_Left,
		                    [](R* r) { r->getCamera().left(r->getTimeSinceLastFrame()); });
		r->addKeyDownMethod(Qt::Key_Right,
		                    [](R* r) { r->getCamera().right(r->getTimeSinceLastFrame()); });
		r->addKeyDownMethod(Qt::Key_Down,
		                    [](R* r) { r->getCamera().up(r->getTimeSinceLastFrame()); });
		r->addKeyDownMethod(Qt::Key_Up,
		                    [](R* r) { r->getCamera().down(r->getTimeSinceLastFrame()); });
	}
};

#endif
