#ifndef PLUGINS_HPP
#define PLUGINS_HPP
#include <utility>
#include "macros.h"
#include <iostream>
using namespace std;
#define HOOKCHECK(hName)                                                                \
	template <typename T = R>                                                             \
	static void register_##hName(                                                         \
	    const typename std::enable_if<has_member_##hName<P>::value, T *>::type r, P &p) { \
		r->plugins_##hName.push_back(std::bind(&P::template hName<T>, &p, r));              \
	}                                                                                     \
	template <typename T = R>                                                             \
	static void register_##hName(                                                         \
	    const typename std::enable_if<!has_member_##hName<P>::value, T *>::type, P &) {}

CREATE_MEMBER_CHECK(onLoad);
CREATE_MEMBER_CHECK(onDraw);
CREATE_MEMBER_CHECK(preDraw);
CREATE_MEMBER_CHECK(preLoop);
CREATE_MEMBER_CHECK(postDraw);

template <typename R, typename P> struct HookChecker {
	HOOKCHECK(onLoad)
	HOOKCHECK(onDraw)
	HOOKCHECK(preLoop)
	HOOKCHECK(preDraw)
	HOOKCHECK(postDraw)
};

#define REGISTERH(hName) HookChecker<R, P>::register_##hName(r, p);

template <typename R> struct PluginLoader {
	R *r;
	template <typename P> void operator()(P &p) {
		REGISTERH(onLoad)
		REGISTERH(onDraw)
		REGISTERH(preLoop)
		REGISTERH(preDraw)
		REGISTERH(postDraw)
	}
};
#endif
