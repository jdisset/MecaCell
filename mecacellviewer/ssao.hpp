#ifndef SSAO_HPP
#define SSAO_HPP
#include "viewtools.h"
#include "screenmanager.hpp"
#include <memory>
#include <QSize>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFramebufferObjectFormat>

template <typename R> class MSAA : public ScreenManager<R> {
 private:
	std::unique_ptr<QOpenGLFramebufferObject> fbo;
	QOpenGLFramebufferObjectFormat format;
	GLuint depthTex;
	// depth texture initialisation
	void genDepthTexture(QSize s, GLuint& t) {
		GL->glGenTextures(1, &t);
		GL->glBindTexture(GL_TEXTURE_2D, t);
		GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		GL->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, s.width(), s.height(), 0,
		                 GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
	}

 public:
	MSAA(R* r) : name("ssaoStart"), checkable(false) {
		format.setAttachment(QOpenGLFramebufferObject::Depth);
		format.setSamples(0);
		screenChanged(r);
	}
	void call(R* r) {
		QOpenGLFramebufferObject* current = r->getCurrentFBO();
		if (current) {
			current->release();
			QOpenGLFramebufferObject::blitFramebuffer(
			    fbo.get(), current, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			r->setCurrentFBO(fbo.get());
			fbo->bind();
			ssaoTarget.draw(fbo->texture(), depthTex, camera.getNearPlane(),
			                camera.getFarPlane());
		}
	}
	void screenChanged(R* r) {
		fbo.reset(new QOpenGLFramebufferObject(r->getViewportSize() * r->getScreenScaleCoef(),
		                                       format));
		fbo->setAttachment(QOpenGLFramebufferObject::Depth);
		GL->glDeleteTextures(1, &depthTex);
		genDepthTexture(FSAA_COEF * viewportSize * screenCoef, depthTex);
		fbo->bind();
		GL->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
		                           depthTex, 0);
		fbo->release();
	}
};
#endif
