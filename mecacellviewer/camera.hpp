#ifndef CAMERA_HPP
#define CAMERA_HPP
#include <QDebug>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QSizeF>
#include <QVector2D>
#include <QVector3D>
#include <iostream>

class Camera : public QObject {
	Q_OBJECT

 public:
	enum ProjectionType { Perspective, Orthographic };
	enum RotateOrder {
		TiltPanRoll,
		TiltRollPan,
		PanTiltRoll,
		PanRollTilt,
		RollTiltPan,
		RollPanTilt
	};
	enum Mode { fps, centered };
	// in centered mode x is left/right, y is up/down, z is forward/backward

 private:
	ProjectionType projectionType = Perspective;
	Mode mode = fps;
	float fieldOfView = 35;
	float nearPlane = 40;
	float farPlane = 30000;
	QSizeF viewSize = QSizeF(2.0f, 2.0f);
	QSizeF minViewSize = QSizeF(0.01f, 0.01f);
	QVector3D position = QVector3D(0, 1000, 2200);
	QVector3D upVector = QVector3D(0, 1, 0);
	QVector3D target = QVector3D(0, 0, 0);
	QVector3D viewVector = target - position;
	bool adjustForAspectRatio = true;

	float sensitivity = 0.2f;
	float forceIntensity = 15000;
	float rotationSensitivity = 65;
	float friction = 7.0f;
	float mass = 1.5f;
	QVector3D force;
	QVector3D speed;
	QVector3D torque;
	QVector3D angularVelocity;

 public:
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

	void setForce(const QVector3D &value) { force = value; }

	void setMass(float value) { mass = value; }

	void setFriction(float value) { friction = value; }

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

	const QVector3D &getSpeed(void) const { return speed; }

	const QVector3D &getForce(void) const { return force; }

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
			float halfWidth = static_cast<float>(viewSize.width()) / 2.0f;
			float halfHeight = static_cast<float>(viewSize.height()) / 2.0f;
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

	QQuaternion roll(float angle) const {
		return QQuaternion::fromAxisAndAngle(viewVector, angle);
	}
	QQuaternion pan(float angle) const {
		return QQuaternion::fromAxisAndAngle(upVector, angle);
	}

	/*********************************
	 *     MOVEMENT & ORIENTATION
	 *********************************/
	void translate(QVector3D v) {
		position += translation(v);
		viewVector = target - position;
	}

	void moveAroundTarget(const QVector2D &v) { moveAroundTarget(v.x(), v.y()); }
	void moveAroundTarget(float x, float y) {
		if (mode == fps) {
			torque +=
			    rotationSensitivity *
			    (upVector * x + QVector3D::crossProduct(viewVector, upVector).normalized() * y);
		} else if (mode == centered) {
			float distanceFromTarget = (position - target).length();
			force += QVector3D(x, -y, 0) * rotationSensitivity * 0.05f * distanceFromTarget;
		}
	}

	void move(QVector3D v) { translate(v * speed); }
	void move(float x, float y, float z) { move(QVector3D(x, y, z)); }

	void rotate(const QQuaternion &q) {
		upVector = q.rotatedVector(upVector);
		viewVector = q.rotatedVector(viewVector);
		target = position + viewVector;
	}

	void tiltPanRoll(float tiltAngle, float panAngle, float rollAngle) {
		rotate(tilt(tiltAngle) * pan(panAngle) * roll(rollAngle));
	}

	void right(float) {
		if (mode == fps) {
			QVector3D v = QVector3D::crossProduct(viewVector, upVector).normalized();
			force += v * forceIntensity;

		} else if (mode == centered) {
			force += QVector3D(forceIntensity, 0, 0);
		}
	}
	void left(float) {
		if (mode == fps) {
			QVector3D v = QVector3D::crossProduct(upVector, viewVector).normalized();
			force += v * forceIntensity;
		} else if (mode == centered) {
			force += QVector3D(-forceIntensity, 0, 0);
		}
	}
	void backward(float dt) {
		if (dt > 1.0f / 20.0f) dt = 1.0f / 20.0f;
		if (mode == fps) {
			force += -viewVector.normalized() * forceIntensity;
		} else if (mode == centered) {
			force += QVector3D(0, 0, forceIntensity);
		}
	}
	void forward(float dt) {
		if (dt > 1.0f / 20.0f) dt = 1.0f / 20.0f;
		if (mode == fps) {
			force += viewVector.normalized() * forceIntensity;
		} else if (mode == centered) {
			force += QVector3D(0, 0, -forceIntensity);
		}
	}

	void updatePosition(float dt) {
		if (dt > 1.0f / 20.0f) dt = 1.0f / 20.0f;
		if (mode == fps) {
			speed += (force / mass) * dt - speed * friction * dt;
			position += speed * dt;
			force = QVector3D(0, 0, 0);
			angularVelocity += (torque - friction * angularVelocity) * dt;
			QQuaternion q1 = QQuaternion::fromAxisAndAngle(angularVelocity.normalized(),
			                                               dt * angularVelocity.length());
			viewVector = q1.rotatedVector(viewVector);
			upVector = q1.rotatedVector(upVector);
			torque = QVector3D(0, 0, 0);
			target = position + viewVector;
		} else if (mode == centered) {
			speed += (force / mass) * dt - speed * friction * dt;
			float distanceFromTarget = (position - target).length();
			distanceFromTarget += speed.z() * dt;
			position += speed.x() * dt * getRightVector();
			position += speed.y() * dt * getUpVector();
			viewVector = (target - position).normalized();
			position = target - viewVector * distanceFromTarget;
			force = QVector3D(0, 0, 0);
		}
	}
	/**************************
	 *          SET
	 **************************/
	void setUpVector(const QVector3D &vec) { upVector = vec; }
	void setTarget(const QVector3D &vertex) {
		target = vertex;
		viewVector = target - position;
	}
	void setAdjustForAspectRatio(bool value) { adjustForAspectRatio = value; }
	void setPosition(const QVector3D &vertex) {
		position = vertex;
		viewVector = target - position;
	}
	void setMode(Mode m) {
		mode = m;
		speed = QVector3D(0, 0, 0);
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
	QVector3D getRightVector() const {
		return QVector3D::crossProduct(viewVector, upVector).normalized();
	}

	std::string toString() {
		QString dbg;
		QDebug(&dbg) << "Camera";
		QDebug(&dbg) << "\n";
		QDebug(&dbg) << "   projection:"
		             << (getProjectionType() == Perspective ? "Perspective" : "Orthographic");
		QDebug(&dbg) << "-- viewsize:" << getViewSize().width() << "x"
		             << getViewSize().height() << "\n";
		QDebug(&dbg) << "   near-plane:" << getNearPlane()
		             << "-- far-plane:" << getFarPlane();
		QDebug(&dbg) << "-- field-of-view:" << getFieldOfView() << "\n";
		QDebug(&dbg) << "   position:" << getPosition() << "-- target:" << getTarget();
		QDebug(&dbg) << "-- up:" << getUpVector() << "\n";
		return dbg.toStdString();
	}
};
#endif
