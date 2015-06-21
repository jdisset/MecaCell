#ifndef CAMERA_HPP
#define CAMERA_HPP
#include <QVector2D>
#include <QVector3D>
#include <QQuaternion>
#include <QSizeF>
#include <iostream>
#include <QDebug>
#include <QMatrix4x4>

class Camera : public QObject {
	Q_OBJECT

public:
	enum ProjectionType { Perspective, Orthographic };
	enum RotateOrder { TiltPanRoll, TiltRollPan, PanTiltRoll, PanRollTilt, RollTiltPan, RollPanTilt };
	enum Mode { fps, centered };

	ProjectionType projectionType = Perspective;
	Mode mode = fps;
	float fieldOfView = 35;
	float nearPlane = 40;
	float farPlane = 30000;
	QSizeF viewSize = QSizeF(2.0f, 2.0f);
	QSizeF minViewSize = QSizeF(0.01f, 0.01f);
	QVector3D position = QVector3D(0, 200, 500);
	QVector3D upVector = QVector3D(0, 1, 0);
	QVector3D target = QVector3D(0, 0, 0);
	QVector3D viewVector = target - position;
	bool adjustForAspectRatio = true;
	float sensitivity = 0.2;
	float speed = 1000;

	Camera() {}

	void update(Camera &c) {
		fieldOfView = c.fieldOfView;
		position = c.position;
		upVector = c.upVector;
		target = c.target;
		viewVector = c.viewVector;
		adjustForAspectRatio = c.adjustForAspectRatio;
		sensitivity = c.sensitivity;
		speed = c.speed;
	}

	QVector3D getOrientation() { return viewVector.normalized(); }

	void setProjectionType(ProjectionType value) { projectionType = value; }

	void setFieldOfView(float angle) { fieldOfView = angle; }

	void setNearPlane(float value) { nearPlane = value; }

	void setFarPlane(float value) { farPlane = value; }

	void setMinViewSize(const QSizeF &size) { minViewSize = size; }

	void setViewSize(const QSizeF &size) {
		QSizeF sz(size);
		if (qAbs(sz.width()) < minViewSize.width()) {
			if (sz.width() >= 0.0f)
				sz.setWidth(minViewSize.width());
			else
				sz.setWidth(-minViewSize.width());
		}
		if (qAbs(sz.height()) < minViewSize.height()) {
			if (sz.height() >= 0.0f)
				sz.setHeight(minViewSize.height());
			else
				sz.setHeight(-minViewSize.height());
		}
		if (viewSize != sz) {
			viewSize = sz;
		}
	}

	QVector3D translation(const QVector3D &v) const {
		// translation is expressed in camera space...
		QVector3D vector(0.0f, 0.0f, 0.0f);
		if (v.x() != 0.0f) vector += QVector3D::normal(viewVector, upVector) * v.x();
		if (v.y() != 0.0f) vector += upVector.normalized() * v.y();
		if (v.z() != 0.0f) vector += viewVector.normalized() * v.z();
		return vector;
	}

	QMatrix4x4 getProjectionMatrix(float aspectRatio) const {
		QMatrix4x4 m;
		if (!adjustForAspectRatio) aspectRatio = 1.0f;
		if (projectionType == Perspective && fieldOfView != 0.0f) {
			m.perspective(fieldOfView, aspectRatio, nearPlane, farPlane);
		} else {
			float halfWidth = viewSize.width() / 2.0f;
			float halfHeight = viewSize.height() / 2.0f;
			if (aspectRatio > 1.0f) {
				halfWidth *= aspectRatio;
			} else if (aspectRatio > 0.0f && aspectRatio < 1.0f) {
				halfHeight /= aspectRatio;
			}
			if (projectionType == Perspective) {
				m.frustum(-halfWidth, halfWidth, -halfHeight, halfHeight, nearPlane, farPlane);
			} else {
				m.ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, nearPlane, farPlane);
			}
		}
		return m;
	}

	QMatrix4x4 getViewMatrix() const {
		QMatrix4x4 m;
		m.lookAt(position, target, upVector);
		return m;
	}

	QQuaternion tilt(float angle) const {
		QVector3D side = QVector3D::crossProduct(viewVector, upVector);
		return QQuaternion::fromAxisAndAngle(side, angle);
	}

	QQuaternion roll(float angle) const { return QQuaternion::fromAxisAndAngle(viewVector, angle); }
	QQuaternion pan(float angle) const { return QQuaternion::fromAxisAndAngle(upVector, angle); }

	/*********************************
	 *     MOVEMENT & ORIENTATION
	*********************************/
	void translate(QVector3D v) {
		position += translation(v);
		viewVector = target - position;
	}

	void moveAroundTarget(const QVector2D &v) { moveAroundTarget(v.x(), v.y()); }
	void moveAroundTarget(qreal x, qreal y) {
		QQuaternion q1 = tilt(sensitivity * y);
		if (mode == centered) {
			QVector3D newpos = target + q1.rotatedVector(-viewVector);
			upVector = q1.rotatedVector(upVector);
			setPosition(newpos);
			QQuaternion q2 = pan(sensitivity * x);
			newpos = target + q2.rotatedVector(-viewVector);
			upVector = q2.rotatedVector(upVector);
			setPosition(newpos);
		} else if (mode == fps) {
			upVector = q1.rotatedVector(upVector);
			// plane projection
			upVector = (upVector -
			            QVector3D(1, 0, 0) * QVector3D::dotProduct(upVector, QVector3D(1, 0, 0)) /
			                upVector.lengthSquared())
			               .normalized();
			viewVector = q1.rotatedVector(viewVector);
			QQuaternion q2 = pan(sensitivity * x);
			viewVector = q2.rotatedVector(viewVector);
			target = position + viewVector;
			// newpos = target + q2.rotatedVector(-viewVector);
			// upVector = q2.rotatedVector(upVector);
			// setPosition(newpos);
		}
	}

	void move(QVector3D v) { translate(v * speed); }
	void move(qreal x, qreal y, qreal z) { move(QVector3D(x, y, z)); }

	void rotate(const QQuaternion &q) {
		upVector = q.rotatedVector(upVector);
		viewVector = q.rotatedVector(viewVector);
		target = position + viewVector;
	}

	void tiltPanRoll(float tiltAngle, float panAngle, float rollAngle) {
		rotate(tilt(tiltAngle) * pan(panAngle) * roll(rollAngle));
	}

	void right(float dt) {
		QVector3D v = QVector3D::crossProduct(viewVector, upVector).normalized() * speed * dt;
		position += v;
		target = position + viewVector;
		if (mode == centered) {
			viewVector = (target - position).normalized();
		}
	}
	void left(float dt) {
		QVector3D v = QVector3D::crossProduct(upVector, viewVector).normalized() * speed * dt;
		position += v;
		target = position + viewVector;
		if (mode == centered) {
			viewVector = (target - position).normalized();
		}
	}
	void backward(float dt) {
		if (mode == fps) {
			position += -viewVector.normalized() * speed * dt;
			target = position + viewVector;
		} else if (mode == centered) {
			float amount = 0.1 * (position - target).length();
			translate(-viewVector.normalized() * dt * amount);
		}
	}
	void forward(float dt) {
		if (mode == fps) {
			position += viewVector.normalized() * speed * dt;
			target = position + viewVector;
		} else if (mode == centered) {
			float amount = 0.1 * (position - target).length();
			translate(viewVector.normalized() * dt * amount);
		}
	}
	/**************************
	 *          SET
	**************************/
	void setUpVector(const QVector3D &vector) { upVector = vector; }
	void setTarget(const QVector3D &vertex) {
		target = vertex;
		viewVector = target - position;
	}
	void setAdjustForAspectRatio(bool value) { adjustForAspectRatio = value; }
	void setPosition(const QVector3D &vertex) {
		position = vertex;
		viewVector = target - position;
	}

	/**************************
	 *          GET
	**************************/
	ProjectionType getProjectionType() const { return projectionType; }
	QSizeF getViewSize() const { return viewSize; }
	float getNearPlane() const { return nearPlane; }
	float getFarPlane() const { return farPlane; }
	float getFieldOfView() const { return fieldOfView; }
	QVector3D getPosition() const { return position; }
	QVector3D getTarget() const { return target; }
	QVector3D getUpVector() const { return upVector.normalized(); }
	QVector3D getViewVector() const { return viewVector.normalized(); }
	QVector3D getRightVector() const { return QVector3D::crossProduct(viewVector, upVector).normalized(); }

	std::string toString() {
		QString dbg;
		QDebug(&dbg) << "Camera";
		QDebug(&dbg) << "\n";
		QDebug(&dbg) << "   projection:" << (getProjectionType() == Perspective ? "Perspective" : "Orthographic");
		QDebug(&dbg) << "-- viewsize:" << getViewSize().width() << "x" << getViewSize().height() << "\n";
		QDebug(&dbg) << "   near-plane:" << getNearPlane() << "-- far-plane:" << getFarPlane();
		QDebug(&dbg) << "-- field-of-view:" << getFieldOfView() << "\n";
		QDebug(&dbg) << "   position:" << getPosition() << "-- target:" << getTarget();
		QDebug(&dbg) << "-- up:" << getUpVector() << "\n";
		return dbg.toStdString();
	}
};
#endif
