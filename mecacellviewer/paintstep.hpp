#ifndef PAINTSTEP_HPP
#define PAINTSTEP_HPP
#include <string>
template <typename RendererType> struct PaintStep {
	std::string name = "generic name";
	std::string category = "Visual elements";
	bool checkable = true;
	virtual void call(RendererType* ) = 0;
};
#endif
