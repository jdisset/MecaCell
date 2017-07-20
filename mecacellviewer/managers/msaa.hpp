#ifndef MSAA_HPP
#define MSAA_HPP
#include <QOpenGLFramebufferObject>
#include <QOpenGLFramebufferObjectFormat>
#include <QSize>
#include <memory>
#include "../paintstep.hpp"
#include "../utilities/viewtools.h"

namespace MecacellViewer {
template <typename R> class MSAA : public ScreenManager<R> {
 private:
	std::unique_ptr<QOpenGLFramebufferObject> fbo;
	QOpenGLFramebufferObjectFormat format;

 public:
	MSAA(R* r, int nbS = 4) : ScreenManager<R>("msaaStart") {
		this->checkable = false;
		format.setAttachment(QOpenGLFramebufferObject::Depth);
		format.setSamples(nbS);
		auto* wdw = r->getWindow();
		if (wdw) {
			auto s = r->getViewportSize() * wdw->devicePixelRatio() * r->getScreenScaleCoef();
			fbo.reset(new QOpenGLFramebufferObject(s, format));
			fbo->setAttachment(QOpenGLFramebufferObject::Depth);
		}
	}
	void call(R* r) {
		QOpenGLFramebufferObject* current = r->getCurrentFBO();
		if (current) current->release();
		r->setCurrentFBO(fbo.get());
		fbo->bind();
		auto* wdw = r->getWindow();
		auto s = r->getViewportSize() * wdw->devicePixelRatio() * r->getScreenScaleCoef();
		GL()->glViewport(0, 0, s.width(), s.height());
		GL()->glDepthMask(true);
		GL()->glClearColor(1.0, 1.0, 1.0, 1.0);
		GL()->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		GL()->glEnable(GL_DEPTH_TEST);
	}
	void screenChanged(R* r) {
		auto* wdw = r->getWindow();
		if (wdw) {
			if (r->getCurrentFBO() == fbo.get() && fbo.get()) {
				fbo->release();
				r->setCurrentFBO(nullptr);
			}
			auto s = r->getViewportSize() * wdw->devicePixelRatio() * r->getScreenScaleCoef();
			fbo.reset(new QOpenGLFramebufferObject(s, format));
			fbo->setAttachment(QOpenGLFramebufferObject::Depth);
		}
	}
};
}
#endif
