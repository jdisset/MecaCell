#ifndef PLUGINS_HPP
#define PLUGINS_HPP
#include "introspect.hpp"
#include <utility>
#include <iostream>
using namespace std;

// checks if a plugin p of type P has a hName methods, and registers it.
#define HOOKCHECK(hName)                                                                \
	template <typename T = R>                                                             \
	static void register_##hName(                                                         \
	    const typename std::enable_if<                                                    \
	        has_##hName##_signatures<P, void(R *), void(const R *)>::value, T *>::type r, \
	    P &p) {                                                                           \
		r->plugins_##hName.push_back([&](R *view) { p.hName(view); });                      \
	}                                                                                     \
	template <typename T = R>                                                             \
	static void register_##hName(                                                         \
	    const typename std::enable_if<                                                    \
	        !has_##hName##_signatures<P, void(R *), void(const R *)>::value, T *>::type,  \
	    P *) {}

CREATE_METHOD_CHECKS(onLoad);
CREATE_METHOD_CHECKS(draw);
CREATE_METHOD_CHECKS(preDraw);
CREATE_METHOD_CHECKS(preLoop);
CREATE_METHOD_CHECKS(postDraw);

template <typename R, typename P> struct HookChecker {
	HOOKCHECK(onLoad)
	HOOKCHECK(preDraw)
	HOOKCHECK(postDraw)

	template <typename T = R>
	static void register_draw(
	    const typename std::enable_if<
	        has_draw_signatures<P, void(R *), void(const R *)>::value, T *>::type r,
	    P &p) {
		r->drawMethods[P::visualObjectName] = std::bind(&P::template draw<R>, &p, r);
	}
	template <typename T = R>
	static void register_draw(
	    const typename std::enable_if<
	        !has_draw_signatures<P, void(R *), void(const R *)>::value, T *>::type,
	    P &) {}
};

#define REGISTERH(hName) HookChecker<R, P>::register_##hName(renderer, p);

template <typename R, typename P> void loadPluginHooks(R *renderer, P &p) {
	REGISTERH(onLoad)
	REGISTERH(preLoop)
	REGISTERH(preDraw)
	REGISTERH(draw)
	REGISTERH(postDraw)
}
#endif
