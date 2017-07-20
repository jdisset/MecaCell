#ifndef CELLMANIPULATIONS_HPP
#define CELLMANIPULATIONS_HPP
#include <QPointF>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <Qt>
#include <algorithm>
#include "viewtools.h"

namespace MecacellViewer {
template <typename R> void dragCell(R* r) {
	// drags a cell using a spring
	if (r->getSelectedCell()) {
		auto viewportSize = r->getViewportSize();
		auto& camera = r->getCamera();
		auto selectedCell = r->getSelectedCell();
		QVector2D mouseNDC(
		    2.0 * r->getMousePosition().x() / (float)viewportSize.width() - 1.0,
		    -((2.0 * r->getMousePosition().y()) / (float)viewportSize.height() - 1.0));
		QVector4D rayEye = r->getCamera()
		                       .getProjectionMatrix((float)viewportSize.width() /
		                                            (float)viewportSize.height())
		                       .inverted() *
		                   QVector4D(mouseNDC, -1.0, 1.0);
		rayEye = QVector4D(rayEye.x(), rayEye.y(), -1.0, 0.0);
		QVector4D ray = camera.getViewMatrix().inverted() * rayEye;
		QVector3D l(ray.x(), ray.y(), ray.z());
		QVector3D l0 = camera.getPosition();
		QVector3D p0 = toQV3D(selectedCell->getPosition());
		QVector3D n = camera.getOrientation();
		if (QVector3D::dotProduct(l, n) != 0) {
			float d = (QVector3D::dotProduct((p0 - l0), n)) / QVector3D::dotProduct(l, n);
			QVector3D projectedPos = l0 + d * l;
			decltype(selectedCell->getPosition()) newPos(projectedPos.x(), projectedPos.y(),
			                                             projectedPos.z());
			newPos = newPos;
			selectedCell->getBody().moveTo(newPos);
			selectedCell->getBody().setVelocity(decltype(selectedCell->getPosition())(0, 0, 0));
		}
	}
}

template <typename R> void pickCell(R* r, QPointF screenCoords) {
	auto viewportSize = r->getViewportSize();
	auto& camera = r->getCamera();
	QVector2D mouseNDC(
	    2.0 * screenCoords.x() / (float)viewportSize.width() - 1.0,
	    -((2.0 * (float)screenCoords.y()) / (float)viewportSize.height() - 1.0));
	QVector4D rayEye =
	    camera
	        .getProjectionMatrix((float)viewportSize.width() / (float)viewportSize.height())
	        .inverted() *
	    QVector4D(mouseNDC, -1.0, 1.0);
	rayEye = QVector4D(rayEye.x(), rayEye.y(), -1.0, 0.0);
	QVector4D ray = camera.getViewMatrix().inverted() * rayEye;
	QVector3D vray(ray.x(), ray.y(), ray.z());
	vray.normalize();
	typename R::Cell* res = nullptr;
	double minEyeDist = 1e20;
	for (auto& c : r->getScenario().getWorld().cells) {
		if (c->getVisible()) {
			double sqRad = pow(c->getBoundingBoxRadius(), 2);
			QVector3D EC = toQV3D(c->getPosition()) - camera.getPosition();
			QVector3D EV = vray * QVector3D::dotProduct(EC, vray);
			double eyeDist = EC.lengthSquared();
			double rayDist = eyeDist - EV.lengthSquared();
			if (rayDist <= sqRad && eyeDist < minEyeDist) {
				minEyeDist = eyeDist;
				res = c;
			}
		}
	}
	r->setSelectedCell(res);
}
}  // namespace MecacellViewer
#endif
