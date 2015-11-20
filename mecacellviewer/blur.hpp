#ifndef BLUR_HPP
#define BLUR_HPP
#include "viewtools.h"
#include "screenmanager.hpp"
#include <memory>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFramebufferObjectFormat>

template <typename R> class MenuBlur : public ScreenManager<R> {
 private:
	std::unique_ptr<QOpenGLFramebufferObject> fbo;
	QOpenGLFramebufferObjectFormat format;
	BlurQuad blurTarget;

 public:
	MenuBlur(R* r) : name("menuBlur"), checkable(false) {
		format.setAttachment(QOpenGLFramebufferObject::Depth);
		format.setSamples(0);
		blurTarget.load(":/shaders/dumb.vert", ":/shaders/blur.frag",
		                r->getViewportSize() * r->getScreenCoef());
		screenChanged(r);
	}
	void call(R* r) {
		auto vps = r->getViewportSize();
		auto sc = r->getScreenCoef();
		GL->glViewport(0, 0, vps.width() * sc, vps.height() * sc);  // viewport reset
		blurTarget.draw(
		    fsaaFBO->texture(), 5, vps * sc,
		    QRect(QPoint(0, 0), QSize(r->isFullscreen() ? 0 : r->getLeftMenuSize() * sc,
		                              vps.height() * sc)));
	}
};
#endif
