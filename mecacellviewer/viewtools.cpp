#include "viewtools.h"

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QOpenGLContext>
#include <vector>

namespace MecacellViewer {
bool culling = false;
QOpenGLFunctions *GL = nullptr;
QString addShaderHeader(const QString &sh) {
	int majorV = QOpenGLContext::currentContext()->format().majorVersion();
	int minorV = QOpenGLContext::currentContext()->format().minorVersion();
	bool needCore = true;
	QString version;
	if ((majorV == 3 && minorV >= 3) || majorV > 3) {
		version = QString::number(majorV) + QString::number(minorV) + "0";
	} else if (majorV == 3) {
		needCore = false;
		switch (minorV) {
			case 2:
				version = "150";
				break;
			case 1:
				version = "140";
				break;
			default:
				version = "130";
				break;
		}
	} else if (minorV == 1) {
		version = "120";
	} else {
		version = "110";
	}
	QString core = needCore ? " core" : "";
	return QString(QString("#version ") + version + core + QString("\n") + sh);
}

QString shaderWithHeader(QString filename) {
	QFile f(filename);
	if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
		qDebug() << "unable to open " << filename << endl;
	QTextStream in(&f);
	return addShaderHeader(in.readAll());
}

std::vector<QVector3D> getSpherePointsPacking(unsigned int n) {
	// we simulate n mutually repulsive points on a sphere
	std::vector<QVector3D> p;
	double prevminl = 0;
	double avgDelta = 1;
	double prevAvgDelta = 1;
	double dt = 2.0 / sqrt(static_cast<double>(n));
	p.reserve(n);
	double maxD = sqrt((4.0 * M_PI / static_cast<double>(n)) / M_PI);

	// init with golden spiral
	double inc = M_PI * (3.0 - sqrt(5.0));
	double off = 2.0 / (double)n;
	for (unsigned int i = 0; i < n; ++i) {
		double y = i * off - 1.0 + (off * 0.5);
		double r = sqrt(1.0 - y * y);
		double phi = i * inc;
		p.push_back(QVector3D(static_cast<float>(cos(phi) * r), static_cast<float>(y),
		                      static_cast<float>(sin(phi) * r))
		                .normalized());
	}

	// then perfect with a few electrostatic repulsion iterations
	int cpt = 0;
	double prevExactDelta = 0;
	double exactDelta = 0;
	do {
		++cpt;
		auto minlAndNewDt = updateElectrostaticPointsOnSphere(p, dt);
		double minl = minlAndNewDt.first;
		dt = minlAndNewDt.second;
		prevExactDelta = exactDelta;
		exactDelta = minl - prevminl;
		if (exactDelta < 0) {
			dt *= 0.8;
		}
		avgDelta = mix(prevAvgDelta, exactDelta, 0.7);
		prevAvgDelta = avgDelta;
		prevminl = minl;
	} while (cpt < 10 || ((prevExactDelta > 0 || exactDelta > 0) && dt > 0.000001 &&
	                      cpt < 200 && fabs(avgDelta) / dt > 0.001 * maxD));
	return p;
}

std::pair<double, double> updateElectrostaticPointsOnSphere(std::vector<QVector3D> &p,
                                                            double dt) {
	double maxD = sqrt((4.0 * M_PI / static_cast<double>(p.size())) / M_PI) * 0.3;
	double maxF = 0;
	std::vector<QVector3D> f(p.size());
	for (size_t i = 0; i < p.size(); ++i) {
		QVector3D force;
		for (size_t j = 0; j < p.size(); ++j) {
			if (i != j) {
				QVector3D unprojected = p[i] - p[j];
				float sql = unprojected.lengthSquared();
				if (sql != 0) {
					unprojected /= sql;
					force += (unprojected - QVector3D::dotProduct(unprojected, p[i]) * p[i]);
				}
			}
		}
		double fIntensity = force.lengthSquared();
		if (fIntensity > maxF) maxF = fIntensity;
		f[i] = force;
	}
	maxF = sqrt(maxF);
	if (maxF * dt > maxD) {
		dt *= (maxD / (maxF * dt));
	}
	double totalDisplacement = 0;
	// now we update the position of each point;
	for (size_t i = 1; i < p.size(); ++i) {
		QVector3D pprev = p[i];
		p[i] += f[i] * static_cast<float>(dt);
		p[i].normalize();
		totalDisplacement += (pprev - p[i]).length();
	}
	double minminsql = 10000;
	for (size_t i = 0; i < p.size(); ++i) {
		double minsql = 10000;
		for (size_t j = i + 1; j < p.size(); ++j) {
			double sql = (p[i] - p[j]).lengthSquared();
			if (sql < minsql) minsql = sql;
		}
		if (minsql < minminsql) minminsql = minsql;
	}
	return std::make_pair(sqrt(minminsql), dt);
}
}
