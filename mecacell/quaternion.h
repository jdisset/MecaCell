#ifndef QUATERNION_H
#define QUATERNION_H
#include <cmath>
#include "vector3D.h"
#include "tools.h"

namespace MecaCell {
struct Quaternion {
   public:
      Vector3D v;
      float_t w;
      Quaternion(const float_t&, const Vector3D& );
      Quaternion(const Vector3D&, const Vector3D&);
      Quaternion(const Quaternion& q):v(q.v),w(q.w){}
      Quaternion(const float_t& x, const float_t& y, const float_t& z, const float_t& ww):v(x,y,z),w(ww){}
      Quaternion():v(0,1,0),w(0){}
      Quaternion operator*(const Quaternion&) const ;
      Vector3D operator*(const Vector3D&) const;
      Vector3D getAxis() const;
      float_t getAngle() const;
      Quaternion normalized() const;
      void normalize();
      Rotation<Vector3D> toAxisAngle();
};
}
#endif

