#ifndef PLUGINS_HPP
#define PLUGINS_HPP
dq
include <mecacell/utilities/introspect.hpp>
using namespace std;

namespace MecacellViewer {
enum class Hooks {
	beginUpdate,
	preBehaviorUpdate,
	postBehaviorUpdate,
	endUpdate,
	addCell,
	destructor,
	LAST
};
}
template <typename T = R>
static void register_##hName(const typename std::enable_if<
                                 is_##hName##_callable<P, void(R *)>::value, T *>::type r,
                             P &p) {
	r->plugins_##hName.push_back([&](R *view) { p.hName(view); });
}
template <typename T = R>
static void register_##hName(
    const typename std::enable_if<!is_##hName##_callable<P, void(R *)>::value, T *>::type,
    P &) {}

CREATE_METHOD_CHECKS(preLoad);
CREATE_METHOD_CHECKS(onLoad);
CREATE_METHOD_CHECKS(onSync);
CREATE_METHOD_CHECKS(draw);
CREATE_METHOD_CHECKS(preDraw);
CREATE_METHOD_CHECKS(preLoop);
CREATE_METHOD_CHECKS(postDraw);

template <typename R, typename P> struct HookChecker {
	HOOKCHECK(preLoad)
	HOOKCHECK(onLoad)
	HOOKCHECK(onSync)
	HOOKCHECK(preLoop)
	HOOKCHECK(preDraw)
	HOOKCHECK(postDraw)

	template <typename T = R>
	static void register_draw(
	    const typename std::enable_if<
	        has_draw_signatures<P, void(R *), void(const R *)>::value(), T *>::type r,
	    P &p) {
		r->drawMethods[P::visualObjectName] = std::bind(&P::template draw<R>, &p, r);
	}
	template <typename T = R>
	static void register_draw(
	    const typename std::enable_if<
	        !has_draw_signatures<P, void(R *), void(const R *)>::value(), T *>::type,
	    P &) {}
};

#define REGISTERH(hName) HookChecker<R, P>::register_##hName(renderer, p);

template <typename R, typename P> void loadPluginHooks(R *renderer, P &p) {
	REGISTERH(preLoad)
	REGISTERH(onLoad)
	REGISTERH(preLoop)
	REGISTERH(preDraw)
	REGISTERH(draw)
	REGISTERH(postDraw)
}
template <typename R, typename P> void loadSyncHooks(R *renderer, P &p) {
	REGISTERH(onSync)
}
#endif
