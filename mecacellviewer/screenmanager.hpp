#ifndef SCREENMANAGER_HPP
#define SCREENMANAGER_HPP
#include "paintstep.hpp"
#include <string>
template <typename R> struct ScreenManager : public PaintStep<R> {
	ScreenManager() {}
	ScreenManager(const QString& s) : PaintStep<R>(s, "Post processing") {}
	virtual void screenChanged(R*){};
};
#endif
