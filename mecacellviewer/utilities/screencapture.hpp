#ifndef SCREENCAPTURE_HPP
#define SCREENCAPTURE_HPP
#include "viewtools.h"
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
	int NBFRAMEPERSCREEN = 10;

	void saveImg(int W, int H) {
		std::vector<GLubyte> pixels;
		pixels.resize(3 * W * H);
		GL()->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		GL()->glReadPixels(0, 0, W, H, GL_RGB, GL_UNSIGNED_BYTE, &pixels[0]);
		QImage img(&pixels[0], W, H, QImage::Format_RGB888);
		img.mirrored().save(path + QString("capture_") + QString::number(cap++) + ".jpg");
	}

	// void saveImg() {
	// r->getCurrentFBO()->toImage().save(path + QString("capture_") +
	// QString::number(cap++) + ".png");
	//}
	
	void call(R* r) {
		if (r->getCurrentFBO()) {
			if (r->getFrame() % NBFRAMEPERSCREEN == 0) {
				auto s = r->getWindow()->renderTargetSize();
				// saveImg(s.width(), s.height());
				saveImg(r->getWindow()->width()*2.0, r->getWindow()->height()*2.0);
			}
		}
	}

	void screenChanged(R* r) {}
};
}
#endif
