#ifndef VIEWTOOLS_H
#define VIEWTOOLS_H
#include <QVector3D>
#include <QDir>
#include <QOpenGLFunctions>
#include <memory>
#include <math.h>

extern bool culling;
extern QOpenGLFunctions *GL;
template <typename V> QVector3D toQV3D(const V &v) { return QVector3D(v.x, v.y, v.z); }
QString shaderWithHeader(QString filename);
void initResources();
inline double radToDeg(double x) { return x / M_PI * 180; }
#endif
