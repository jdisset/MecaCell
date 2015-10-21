#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QOpenGLContext>
#include "viewtools.h"
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
}
