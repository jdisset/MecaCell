#ifndef VIEWTOOLS_H
#define VIEWTOOLS_H
#include <math.h>
#include <QColor>
#include <QDir>
#include <QOpenGLFunctions>
#include <QVector3D>
#include <QVector4D>
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

template <typename C>
QVector4D cellColorToQVector(const C* c, bool selected,
                             const ColorMode& colormode = color_normal) {
	if (selected) return QVector4D(1.0, 1.0, 1.0, 1.0);
	switch (colormode) {
		/* case color_pressure: {*/
		// QColor col =
		// QColor::fromHsvF(0.8f - 0.8f * (float)c->getNormalizedPressure(), 0.8, 0.70);
		// return QVector4D(col.redF(), col.greenF(), col.blueF(), 1.0);
		// break;
		/*}*/
		default:
			return QVector4D(c->getColor(0), c->getColor(1), c->getColor(2), 1.0);
			break;
	}
}
}
#endif
