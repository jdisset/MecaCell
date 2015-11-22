#ifndef MOUSEMANAGER_HPP
#define MOUSEMANAGER_HPP
#include "cellmanipulations.hpp"
#include <Qt>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>

class MouseManager {
 public:
	template <typename R> void onLoad(R* r) {
		r->addMouseDragMethod(Qt::LeftButton, [](R* r) {
			QVector2D mouseMovement(r->getMousePosition() - r->getPreviousMousePosition());
			r->getCamera().moveAroundTarget(-mouseMovement);
		});
		r->addMouseDragMethod(Qt::RightButton, [](R* r) { dragCell(r); });
		r->addMouseClickMethod(Qt::MidButton,
		                       [](R* r) { pickCell(r, r->getMousePosition()); });
	}
};
#endif
