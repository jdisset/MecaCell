#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QOpenGLContext>
#include "viewtools.h"
bool culling = false;
QOpenGLFunctions *GL = nullptr;
QString shaderWithHeader(QString filename) {
	int majorV = QOpenGLContext::currentContext()->format().majorVersion();
	int minorV = QOpenGLContext::currentContext()->format().minorVersion();
	QString version;
	if ((majorV == 3 && minorV >= 3) || majorV > 3) {
		version = QString::number(majorV) + QString::number(minorV) + "0";
	} else if (majorV == 3) {
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

	QFile f(filename);
	if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) qDebug() << "unable to open " << filename << endl;
	QTextStream in(&f);
	QString res = QString("#version ") + version + QString(" core\n") + in.readAll();
	return res;
}
void initResources() { Q_INIT_RESOURCE(resourcesLibMecacellViewer); }
