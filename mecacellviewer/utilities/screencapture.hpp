#ifndef SCREENCAPTURE_MECACELLVIEWERPLUGIN_HPP
#define SCREENCAPTURE_MECACELLVIEWERPLUGIN_HPP
#include <QImage>
/**
 * @brief ScreenCapturePlugin creates a checkable option in the menu that enables screen
 * capture
 */

struct ScreenCapturePlugin {
	void saveImg(int W, int H, const QString &path) {
		std::vector<GLubyte> pixels;
		pixels.resize(3 * W * H);
		GL()->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		GL()->glReadPixels(0, 0, W, H, GL_RGB, GL_UNSIGNED_BYTE, &pixels[0]);
		QImage img(&pixels[0], W, H, QImage::Format_RGB888);
		img.mirrored().save(path + QString("capture_") + QString::number(cap++) + ".jpg");
	}

	QString path = "./capture/";
	int cap = 0;
	int skippedFrame = 0;
	template <typename R> void onLoad(R *renderer) {
		MenuElement<R> *nativeDisplayMenu = renderer->getDisplayMenu();
		MenuElement<R> capture = {"Enable screen capture", true};
		capture.onToggled = [&](R *r, MenuElement<R> *me) {
			if (me->isChecked())
				r->addPaintStepsMethods(1000000000, [&](R *r2) {
					if (r2->getCurrentFBO() && r2->getFrame() % (1 + skippedFrame) == 0) {
						saveImg(r2->getWindow()->width(), r2->getWindow()->height(), path);
					}
				});
			else
				r->erasePaintStepsMethods(1000000000);
		};
		nativeDisplayMenu->add(capture);
	}
};

#endif
