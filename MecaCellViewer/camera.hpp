#ifndef CAMERA_HPP
#define CAMERA_HPP
#include <QVector3D>
#include <QQuaternion>
#include <QSizeF>
#include <iostream>
#include <QDebug>
#include <QMatrix4x4>

enum ProjectionType {Perspective, Orthographic};
enum RotateOrder {TiltPanRoll,TiltRollPan,PanTiltRoll,PanRollTilt,RollTiltPan,RollPanTilt};
class Camera : public QObject
{
   Q_OBJECT

   public:

      ProjectionType projectionType = Perspective;
      float fieldOfView             = 35;
      float nearPlane               = 20;
      float farPlane                = 50000000;
      QSizeF viewSize               = QSizeF(2.0f,2.0f);
      QSizeF minViewSize            = QSizeF(0.01f, 0.01f);
      QVector3D position            = QVector3D(0,200,500);
      QVector3D upVector            = QVector3D(0,1,0);
      QVector3D target              = QVector3D(0,0,0);
      QVector3D viewVector          = target-position;
      bool adjustForAspectRatio     = true;
      float sensitivity             = 0.5;
      float speed                   = 4;

      Camera(){}
      void update(Camera &c){
         fieldOfView = c.fieldOfView;
         position = c.position;
         upVector = c.upVector;
         target = c.target;
         viewVector = c.viewVector;
         adjustForAspectRatio = c.adjustForAspectRatio;
         sensitivity = c.sensitivity;
         speed = c.speed;
      }

      QVector3D getOrientation(){
         return viewVector.normalized();
      }
      void setProjectionType(ProjectionType value){projectionType=value;}

      void setFieldOfView(float angle){ fieldOfView = angle;}

      void setNearPlane(float value){nearPlane = value;}

      void setFarPlane(float value){farPlane = value;}

      void setMinViewSize(const QSizeF& size){minViewSize = size;}

      void setViewSize(const QSizeF& size){
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

      QVector3D translation(const QVector3D& v) const{
         QVector3D vector(0.0f, 0.0f, 0.0f);
         if (v.x() != 0.0f)
            vector += QVector3D::normal(viewVector, upVector) * v.x();
         if (v.y() != 0.0f)
            vector += upVector.normalized() * v.y();
         if (v.z() != 0.0f)
            vector += viewVector.normalized() * v.z();
         return vector;
      }

      QMatrix4x4 getProjectionMatrix(float aspectRatio) const{
         QMatrix4x4 m;
         if (!adjustForAspectRatio)
            aspectRatio = 1.0f;
         if (projectionType == Perspective && fieldOfView != 0.0f) {
            m.perspective(fieldOfView, aspectRatio,
                  nearPlane, farPlane);
         } else {
            float halfWidth = viewSize.width() / 2.0f;
            float halfHeight = viewSize.height() / 2.0f;
            if (aspectRatio > 1.0f) {
               halfWidth *= aspectRatio;
            } else if (aspectRatio > 0.0f && aspectRatio < 1.0f) {
               halfHeight /= aspectRatio;
            }
            if (projectionType == Perspective) {
               m.frustum(-halfWidth, halfWidth, -halfHeight, halfHeight,
                     nearPlane, farPlane);
            } else {
               m.ortho(-halfWidth, halfWidth, -halfHeight, halfHeight,
                     nearPlane, farPlane);
            }
         }
         return m;
      }
      QMatrix4x4 getViewMatrix() const{
         QMatrix4x4 m;
         m.lookAt(position, target, upVector);
         return m;
      }
      QVector3D getPosition() const{return position;}
      QQuaternion tilt(float angle) const{
         QVector3D side = QVector3D::crossProduct(viewVector, upVector);
         return QQuaternion::fromAxisAndAngle(side, angle);
      }

      QQuaternion roll(float angle) const{return QQuaternion::fromAxisAndAngle(viewVector, angle);}
      QQuaternion pan(float angle) const{return QQuaternion::fromAxisAndAngle(upVector, angle);}

      public slots:
         void setPosition(const QVector3D& vertex){
            position = vertex;
            viewVector = target - position;
         }

      void translate(QVector3D v){
         position += translation(v);
         //target += translation(v);
         viewVector = target - position;
      }

      void moveAroundTarget (qreal x, qreal y){
         QQuaternion q1 = tilt(sensitivity*y);
         QQuaternion q2 = pan(sensitivity*x);
         QVector3D newpos = target + q1.rotatedVector(-viewVector);
         upVector = q1.rotatedVector(upVector);
         setPosition(newpos);
         newpos = target + q2.rotatedVector(-viewVector);
         upVector = q2.rotatedVector(upVector);
         setPosition(newpos);
      }
      void move(QVector3D v){
         translate(v*speed);
      }
      void move(qreal x, qreal y, qreal z){
         move(QVector3D(x,y,z));
      }

      void setUpVector(const QVector3D& vector){upVector = vector;}

      void setTarget(const QVector3D& vertex){
         target = vertex;
         viewVector = target - position;
      }


      void setAdjustForAspectRatio(bool value){adjustForAspectRatio = value;}

      void rotate(const QQuaternion& q){
         upVector = q.rotatedVector(upVector);
         viewVector = q.rotatedVector(viewVector);
         target = position + viewVector;
      }


      void tiltPanRoll(float tiltAngle, float panAngle, float rollAngle){
               rotate(tilt(tiltAngle) * pan(panAngle) * roll(rollAngle));
      }

      ProjectionType getProjectionType(){return projectionType;}
      QSizeF getViewSize(){return viewSize;}
      float getNearPlane(){return nearPlane;}
      float getFarPlane(){return farPlane;}
      float getFieldOfView(){return fieldOfView;}
      QVector3D getPosition(){return position;}
      QVector3D getTarget(){return target;}
      QVector3D getUpVector(){return upVector;}
      QVector3D getViewVector(){return viewVector;}

      std::string toString(){
         QString dbg;
         QDebug(&dbg) << "Camera";
         QDebug(&dbg) << "\n";
         QDebug(&dbg) << "   projection:" << (getProjectionType() == Perspective ?
               "Perspective" : "Orthographic" );
         QDebug(&dbg) << "-- viewsize:" << getViewSize().width() << "x" << getViewSize().height() << "\n";
         QDebug(&dbg) << "   near-plane:" << getNearPlane() << "-- far-plane:" << getFarPlane();
         QDebug(&dbg) << "-- field-of-view:" << getFieldOfView() << "\n";
         QDebug(&dbg) << "   position:" << getPosition() << "-- target:" << getTarget();
         QDebug(&dbg) << "-- up:" << getUpVector() << "\n";
         return dbg.toStdString();

      }
};
#endif
