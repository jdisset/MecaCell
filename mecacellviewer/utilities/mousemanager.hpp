#ifndef MOUSEMANAGER_HPP
#define MOUSEMANAGER_HPP
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <Qt>
#include "cellmanipulations.hpp"

class MouseManager {
 public:
	template <typename R> void onLoad(R* renderer) {
		renderer->addMouseDragMethod(Qt::LeftButton, [](R* r) {
			QVector2D mouseMovement(r->getMousePosition() - r->getPreviousMousePosition());
			r->getCamera().moveAroundTarget(-mouseMovement);
		});
		renderer->addMouseDragMethod(Qt::RightButton, [&](R* r) { dragCell(r); });
		renderer->addMouseClickMethod(Qt::MidButton,
		                       [](R* r) { pickCell(r, r->getMousePosition()); });
	}
};
#endif
