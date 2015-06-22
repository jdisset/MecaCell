#ifndef VIEWTOOLS_H
#define VIEWTOOLS_H
#include <QVector3D>
#include <QDir>
#include <QOpenGLFunctions>
#include <memory>

extern QOpenGLFunctions *GL;
template <typename V> QVector3D toQV3D(const V &v) { return QVector3D(v.x, v.y, v.z); }
QString shaderWithHeader(QString filename);
void initResources();
#endif
