#ifndef HOOKS_UTILITIES_HPP
#define HOOKS_UTILITIES_HPP
#include "introspect.hpp"
namespace MecaCell {
enum class Hooks {
	beginUpdate,
	preBehaviorUpdate,
	postBehaviorUpdate,
	endUpdate,
	addCell,
	destructor,
	LAST
};

// checks if a plugin p of type P has a hName methods, and registers it.
#define HOOKCHECK(hName)                                                         \
	template <typename T = R>                                                      \
	static void register_##hName(                                                  \
	    const typename std::enable_if<is_##hName##_callable<P, void(R *)>::value,  \
	                                  T *>::type r,                                \
	    P &p) {                                                                    \
		r->registerHook(Hooks::hName, [&](R *w) { p.hName(w); });                    \
	}                                                                              \
	template <typename T = R>                                                      \
	static void register_##hName(                                                  \
	    const typename std::enable_if<!is_##hName##_callable<P, void(R *)>::value, \
	                                  T *>::type,                                  \
	    P &) {}

CREATE_METHOD_CHECKS(beginUpdate);
CREATE_METHOD_CHECKS(preBehaviorUpdate);
CREATE_METHOD_CHECKS(postBehaviorUpdate);
CREATE_METHOD_CHECKS(endUpdate);
CREATE_METHOD_CHECKS(addCell);
CREATE_METHOD_CHECKS(destructor);

template <typename R, typename P> struct HookChecker {
	HOOKCHECK(beginUpdate)
	HOOKCHECK(preBehaviorUpdate)
	HOOKCHECK(postBehaviorUpdate)
	HOOKCHECK(endUpdate)
	HOOKCHECK(addCell)
	HOOKCHECK(destructor)
};

#define REGISTERH(hName) HookChecker<R, P>::register_##hName(renderer, p);

template <typename R, typename P> void loadPluginHooks(R *renderer, P &p) {
	REGISTERH(beginUpdate);
	REGISTERH(preBehaviorUpdate);
	REGISTERH(postBehaviorUpdate);
	REGISTERH(endUpdate);
	REGISTERH(addCell);
	REGISTERH(destructor);
}
}
#endif
