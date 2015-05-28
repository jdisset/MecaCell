#ifndef VIEWTOOLS_H
#define VIEWTOOLS_H
#include <QVector3D>
#include <QOpenGLFunctions>
#include <memory>

extern QOpenGLFunctions* GL ;
template <typename V> QVector3D toQV3D(const V& v) { return QVector3D(v.x, v.y, v.z); }
#endif
