#ifndef SSAO_HPP
#define SSAO_HPP
#include <QOpenGLFramebufferObject>
#include <QOpenGLFramebufferObjectFormat>
#include <QSize>
#include <memory>
#include "../primitives/renderquad.hpp"
#include "../utilities/viewtools.h"
#include "screenmanager.hpp"

namespace MecacellViewer {
template <typename R> class SSAO : public ScreenManager<R> {
 private:
	std::unique_ptr<QOpenGLFramebufferObject> ssaofbo, renderfbo;
	QOpenGLFramebufferObjectFormat ssaoformat, renderformat;
	GLuint depthTex;
	RenderQuad ssaoTarget;
	RenderQuad dumbTarget;
	// depth texture initialisation
	void genDepthTexture(QSize s) {
		GL()->glDeleteTextures(1, &depthTex);
		GL()->glGenTextures(1, &depthTex);
		GL()->glBindTexture(GL_TEXTURE_2D, depthTex);
		GL()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		GL()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		GL()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		GL()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		GL()->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, s.width(), s.height(), 0,
		                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}

 public:
	SSAO(R* r) : ScreenManager<R>("ssao") {
		this->checkable = true;
		ssaoformat.setAttachment(QOpenGLFramebufferObject::Depth);
		ssaoformat.setSamples(0);
		renderformat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
		renderformat.setSamples(0);
		ssaoTarget.load(":/shaders/dumb.vert", ":/shaders/ssao.frag");
		dumbTarget.load(":/shaders/dumb.vert", ":/shaders/dumb.frag");
		screenChanged(r);
	}
	void callDumb(R* r) {
		QOpenGLFramebufferObject* current = r->getCurrentFBO();
		if (current) {
			current->release();
			r->setCurrentFBO(renderfbo.get());
			renderfbo->bind();
			QOpenGLFramebufferObject::blitFramebuffer(
			    ssaofbo.get(), current, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			dumbTarget.draw(ssaofbo->texture());
		}
	}
	void call(R* r) {
		QOpenGLFramebufferObject* current = r->getCurrentFBO();
		if (current) {
			current->release();
			r->setCurrentFBO(renderfbo.get());
			renderfbo->bind();
			QOpenGLFramebufferObject::blitFramebuffer(
			    ssaofbo.get(), current, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			ssaoTarget.draw(ssaofbo->texture(), depthTex, r->getCamera().getNearPlane(),
			                r->getCamera().getFarPlane());
		}
	}
	void screenChanged(R* r) {
		auto* wdw = r->getWindow();
		if (wdw) {
			if (r->getCurrentFBO()) {
				r->getCurrentFBO()->release();
				r->setCurrentFBO(nullptr);
			}
			auto s = r->getViewportSize() * wdw->devicePixelRatio() * r->getScreenScaleCoef();

			renderfbo.reset(new QOpenGLFramebufferObject(s, renderformat));
			renderfbo->setAttachment(QOpenGLFramebufferObject::NoAttachment);

			ssaofbo.reset(new QOpenGLFramebufferObject(s, ssaoformat));
			ssaofbo->setAttachment(QOpenGLFramebufferObject::Depth);

			genDepthTexture(s);
			ssaofbo->bind();
			GL()->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
			                           depthTex, 0);
			ssaofbo->release();
		}
	}
};
}
#endif
