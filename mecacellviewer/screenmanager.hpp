#ifndef SCREENMANAGER_HPP
#define SCREENMANAGER_HPP
#include "paintstep.hpp"
template <typename R> struct ScreenManager : public PaintStep<R> {
	virtual void screenChanged(R* r) = 0;
};
#endif
