#ifndef VIEWTOOLS_H
#define VIEWTOOLS_H
#include <math.h>
#include <QColor>
#include <QDir>
#include <QOpenGLFunctions>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>
#include <iostream>
#include <memory>
#include <tuple>
#include <utility>

namespace MecacellViewer {
using std::tuple_size;
using std::remove_reference;

enum cellMode { plain, centers };
enum ColorMode { color_normal, color_pressure };
extern bool culling;
extern QOpenGLFunctions* GL;
extern double scaleFactor;
template <typename V> QVector3D toQV3D(const V& v) {
	return QVector3D(v.x(), v.y(), v.z());
}
template <typename M> QMatrix4x4 toQM4x4(const M& m) {
	return QMatrix4x4(m[0][0], m[0][1], m[0][2], m[0][3], m[1][0], m[1][1], m[1][2],
	                  m[1][3], m[2][0], m[2][1], m[2][2], m[2][3], m[3][0], m[3][1],
	                  m[3][2], m[3][3]);
}
QString shaderWithHeader(QString filename);

std::vector<QVector3D> getSpherePointsPacking(unsigned int n);
std::pair<double, double> updateElectrostaticPointsOnSphere(std::vector<QVector3D>& p,
                                                            double dt);

inline double radToDeg(double x) { return x / M_PI * 180.0; }

template <typename T> T mix(T m, T M, double v) { return (1.0 - v) * m + v * M; }

// foreach

template <size_t> struct int_ {};

template <class Tuple, class Func, size_t I> void forEach(Tuple&& t, Func&& f, int_<I>) {
	const size_t Tsize = tuple_size<typename remove_reference<Tuple>::type>::value;
	f(std::get<Tsize - I>(t));
	forEach(std::forward<Tuple>(t), std::forward<Func>(f), int_<I - 1>());
}

template <class Tuple, class Func> void forEach(Tuple&& t, Func&& f, int_<1>) {
	const size_t Tsize = tuple_size<typename remove_reference<Tuple>::type>::value;
	f(std::get<Tsize - 1>(t));
}

template <class Tuple, class Func> void forEach(Tuple&& t, Func&& f) {
	const size_t Tsize = tuple_size<typename remove_reference<Tuple>::type>::value;
	forEach(std::forward<Tuple>(t), std::forward<Func>(f), int_<Tsize>());
}

template <typename COLOR>
QVector4D cellColorToQVector(const COLOR &c) {
        return QVector4D(c[0], c[1], c[2], 1.0);
}
}
#endif
