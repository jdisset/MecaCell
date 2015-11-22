#ifndef PAINTSTEP_HPP
#define PAINTSTEP_HPP
#include <QString>
template <typename RendererType> struct PaintStep {
	QString name = "generic name";
	QString category = "Visual elements";
	bool checkable = true;
	PaintStep() {}
	PaintStep(const QString& n, const QString& c = "Visual elements")
	    : name(n), category(c) {}
	virtual void call(RendererType*) = 0;
};
#endif
