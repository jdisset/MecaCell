#ifndef SCREENMANAGER_HPP
#define SCREENMANAGER_HPP
#include <string>
#include "../paintstep.hpp"
template <typename R> struct ScreenManager : public PaintStep<R> {
	ScreenManager() {}
	ScreenManager(const QString& s) : PaintStep<R>(s, "Post processing") {}
	virtual void screenChanged(R*){};
};
#endif
