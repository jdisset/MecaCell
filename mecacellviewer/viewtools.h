#ifndef VIEWTOOLS_H
#define VIEWTOOLS_H
#include <QVector3D>
#include <QDir>
#include <QOpenGLFunctions>
#include <memory>
#include <math.h>
#include <iostream>
#include <utility>
#include <tuple>

namespace MecacellViewer {
using std::tuple_size;
using std::remove_reference;

enum cellMode { plain, centers };
extern bool culling;
extern QOpenGLFunctions* GL;
extern double scaleFactor;
template <typename V> QVector3D toQV3D(const V& v) {
	return QVector3D(v.x(), v.y(), v.z());
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
}
#endif
