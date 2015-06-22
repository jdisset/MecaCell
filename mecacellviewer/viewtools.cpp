#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QOpenGLContext>
#include "viewtools.h"
QOpenGLFunctions *GL = nullptr;
QString shaderWithHeader(QString filename) {
	QString version = QString::number(QOpenGLContext::currentContext()->format().majorVersion()) +
	                  QString::number(QOpenGLContext::currentContext()->format().minorVersion()) + "0";
	QFile f(filename);
	if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) qDebug() << "unable to open " << filename << endl;
	QTextStream in(&f);
	QString res = QString("#version ") + version + QString(" core\n") + in.readAll();
	return res;
}
void initResources() { Q_INIT_RESOURCE(resourcesLibMecacellViewer); }
