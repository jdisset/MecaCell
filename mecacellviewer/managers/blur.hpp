#ifndef BLUR_HPP
#define BLUR_HPP
#include <QOpenGLFramebufferObject>
#include <QOpenGLFramebufferObjectFormat>
#include <memory>
#include "../managers/screenmanager.hpp"
#include "../renderables/blurquad.hpp"
#include "../utilities/viewtools.h"

namespace MecacellViewer {
template <typename R> class MenuBlur : public ScreenManager<R> {
 private:
	BlurQuad blurTarget;

 public:
	MenuBlur(R* r) : ScreenManager<R>("menuBlur") {
		this->checkable = false;
		auto* wdw = r->getWindow();
		auto vps = r->getViewportSize() * wdw->devicePixelRatio();
		qDebug() << "vps = " << vps;
		blurTarget.load(":/shaders/dumb.vert", ":/shaders/blur.frag", vps);
	}

	void call(R* r) {
		auto* wdw = r->getWindow();
		auto vps = r->getViewportSize() * wdw->devicePixelRatio();
		if (r->getCurrentFBO()) {
			GL()->glViewport(0, 0, vps.width(), vps.height());  // viewport reset
			blurTarget.draw(
			    r->getCurrentFBO()->texture(), 5, vps,
			    QRect(QPoint(0, 0),
			          QSize(r->isFullscreen() ? 0 :
			                                    r->getLeftMenuSize() * wdw->devicePixelRatio(),
			                vps.height())));
		}
	}

	void screenChanged(R* ) {}
};
}
#endif
