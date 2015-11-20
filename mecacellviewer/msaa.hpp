#ifndef MSAA_HPP
#define MSAA_HPP
#include "viewtools.h"
#include "paintstep.hpp"
#include <memory>
#include <QSize>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFramebufferObjectFormat>

template <typename R> class MSAA : public ScreenManager<R> {
 private:
	std::unique_ptr<QOpenGLFramebufferObject> fbo;
	QOpenGLFramebufferObjectFormat format;

 public:
	MSAA(const QSize& viewportSize, int nbS = 4) : name("msaaStart"), checkable(false) {
		format.setAttachment(QOpenGLFramebufferObject::Depth);
		format.setSamples(nbS);
		fbo = unique_ptr<QOpenGLFramebufferObject>(
		    new QOpenGLFramebufferObject(viewportSize, format));
		fbo->setAttachment(QOpenGLFramebufferObject::Depth);
	}
	void call(R* r) {
		QOpenGLFramebufferObject* current = r->getCurrentFBO();
		if (current) current->release();
		r->setCurrentFBO(fbo.get());
		fbo->bind();
	}
	void screenChanged(R* r) {
		fbo.reset(new QOpenGLFramebufferObject(r->getViewportSize() * r->getScreenScaleCoef(),
		                                       format));
	}
};
#endif
