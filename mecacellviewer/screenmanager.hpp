#ifndef SCREENMANAGER_HPP
#define SCREENMANAGER_HPP
#include "paintstep.hpp"
#include <string>
template <typename R> struct ScreenManager : public PaintStep<R> {
	ScreenManager() {}
	ScreenManager(const std::string& s) : PaintStep<R>(s) {}
	virtual void screenChanged(R*){};
};
#endif
