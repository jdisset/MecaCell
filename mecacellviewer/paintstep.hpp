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
	virtual void call(RendererType*){};
};

template <typename RenderType> struct PaintStepMethod {
	QString name = "generic name";
	QString category = "Visual elements";
	std::function<void(RenderType*)> callMethod;
	bool checkable = true;
	QString subgroup;
	PaintStepMethod(QString n, QString c, decltype(callMethod) cm, QString s = "",
	                bool ch = true)
	    : name(n), category(c), callMethod(cm), checkable(ch), subgroup(s){};
	QString getHash() { return category + name + subgroup; }
};

#endif
