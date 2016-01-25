#ifndef SCREENCAPTURE_HPP
#define SCREENCAPTURE_HPP
#include "viewtools.h"
#include "blurquad.hpp"
#include "screenmanager.hpp"
#include <memory>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFramebufferObjectFormat>

namespace MecacellViewer {
template <typename R> class MenuScreenCapture : public ScreenManager<R> {
 public:
	QString path = "./";
	MenuScreenCapture(R* r, QString p = "./")
	    : ScreenManager<R>("menuScreenCapture"), path(p) {}
	int cap = 0;

	void call(R* r) {
		if (r->getCurrentFBO()) {
			if (r->getFrame() % 10 == 0) {
				r->getCurrentFBO()->toImage().save(path + QString("capture_") +
				                                   QString::number(cap++) + ".png");
			}
		}
	}

	void screenChanged(R* r) {}
};
}
#endif
