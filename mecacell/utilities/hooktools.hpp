#include <array>
#include <vector>
#include "introspect.hpp"

// A lot of stupid macros here but the result is quite pleasant to use.
// I'd like to thank the ViM editor for basically writing this file for me...

// HOOKS ENUM
#define HOOKS_ENUM(...) enum Hooks { __VA_ARGS__, LAST };
// METHOD CHECKS
#define CREATE_MC1(m1) CREATE_METHOD_CHECKS(m1);
#define CREATE_MC2(m1, m2)  \
	CREATE_METHOD_CHECKS(m1); \
	CREATE_METHOD_CHECKS(m2);
#define CREATE_MC3(m1, m2, m3) \
	CREATE_METHOD_CHECKS(m1);    \
	CREATE_METHOD_CHECKS(m2);    \
	CREATE_METHOD_CHECKS(m3);
#define CREATE_MC4(m1, m2, m3, m4) \
	CREATE_METHOD_CHECKS(m1);        \
	CREATE_METHOD_CHECKS(m2);        \
	CREATE_METHOD_CHECKS(m3);        \
	CREATE_METHOD_CHECKS(m4);
#define CREATE_MC5(m1, m2, m3, m4, m5) \
	CREATE_METHOD_CHECKS(m1);            \
	CREATE_METHOD_CHECKS(m2);            \
	CREATE_METHOD_CHECKS(m3);            \
	CREATE_METHOD_CHECKS(m4);            \
	CREATE_METHOD_CHECKS(m5);
#define CREATE_MC6(m1, m2, m3, m4, m5, m6) \
	CREATE_METHOD_CHECKS(m1);                \
	CREATE_METHOD_CHECKS(m2);                \
	CREATE_METHOD_CHECKS(m3);                \
	CREATE_METHOD_CHECKS(m4);                \
	CREATE_METHOD_CHECKS(m5);                \
	CREATE_METHOD_CHECKS(m6);
#define CREATE_MC7(m1, m2, m3, m4, m5, m6, m7) \
	CREATE_METHOD_CHECKS(m1);                    \
	CREATE_METHOD_CHECKS(m2);                    \
	CREATE_METHOD_CHECKS(m3);                    \
	CREATE_METHOD_CHECKS(m4);                    \
	CREATE_METHOD_CHECKS(m5);                    \
	CREATE_METHOD_CHECKS(m6);                    \
	CREATE_METHOD_CHECKS(m7);
#define CREATE_MC8(m1, m2, m3, m4, m5, m6, m7, m8) \
	CREATE_METHOD_CHECKS(m1);                        \
	CREATE_METHOD_CHECKS(m2);                        \
	CREATE_METHOD_CHECKS(m3);                        \
	CREATE_METHOD_CHECKS(m4);                        \
	CREATE_METHOD_CHECKS(m5);                        \
	CREATE_METHOD_CHECKS(m6);                        \
	CREATE_METHOD_CHECKS(m7);                        \
	CREATE_METHOD_CHECKS(m8);
#define CREATE_MC9(m1, m2, m3, m4, m5, m6, m7, m8, m9) \
	CREATE_METHOD_CHECKS(m1);                            \
	CREATE_METHOD_CHECKS(m2);                            \
	CREATE_METHOD_CHECKS(m3);                            \
	CREATE_METHOD_CHECKS(m4);                            \
	CREATE_METHOD_CHECKS(m5);                            \
	CREATE_METHOD_CHECKS(m6);                            \
	CREATE_METHOD_CHECKS(m7);                            \
	CREATE_METHOD_CHECKS(m8);                            \
	CREATE_METHOD_CHECKS(m9);

#define OVERLOADED_MACRO(M, ...) _OVR(M, _COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__)
#define _OVR(macroName, number_of_args) _OVR_EXPAND(macroName, number_of_args)
#define _OVR_EXPAND(macroName, number_of_args) macroName##number_of_args

#define _COUNT_ARGS(...) _ARG_PATTERN_MATCH(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define _ARG_PATTERN_MATCH(_1, _2, _3, _4, _5, _6, _7, _8, _9, N, ...) N

#define HCHK_M(hName)                                                                 \
	template <class P>                                                                  \
	static std::enable_if_t<is_##hName##_callable<P, hook_s>::value> register_##hName(  \
	    H *c, P &p) {                                                                   \
		c->registerHook(Hooks::hName, [&](H *hc) { p.hName(hc); });                       \
	}                                                                                   \
	template <class P>                                                                  \
	static std::enable_if_t<!is_##hName##_callable<P, hook_s>::value> register_##hName( \
	    H *, P &) {}

#define HCHK_M1(h1) HCHK_M(h1)
#define HCHK_M2(h1, h2) \
	HCHK_M(h1)            \
	HCHK_M(h2)
#define HCHK_M3(h1, h2, h3) \
	HCHK_M(h1)                \
	HCHK_M(h2)                \
	HCHK_M(h3)
#define HCHK_M4(h1, h2, h3, h4) \
	HCHK_M(h1)                    \
	HCHK_M(h2)                    \
	HCHK_M(h3)                    \
	HCHK_M(h4)
#define HCHK_M5(h1, h2, h3, h4, h5) \
	HCHK_M(h1)                        \
	HCHK_M(h2)                        \
	HCHK_M(h3)                        \
	HCHK_M(h4)                        \
	HCHK_M(h5)
#define HCHK_M6(h1, h2, h3, h4, h5, h6) \
	HCHK_M(h1)                            \
	HCHK_M(h2)                            \
	HCHK_M(h3)                            \
	HCHK_M(h4)                            \
	HCHK_M(h5)                            \
	HCHK_M(h6)
#define HCHK_M7(h1, h2, h3, h4, h5, h6, h7) \
	HCHK_M(h1)                                \
	HCHK_M(h2)                                \
	HCHK_M(h3)                                \
	HCHK_M(h4)                                \
	HCHK_M(h5)                                \
	HCHK_M(h6)                                \
	HCHK_M(h7)
#define HCHK_M8(h1, h2, h3, h4, h5, h6, h7, h8) \
	HCHK_M(h1)                                    \
	HCHK_M(h2)                                    \
	HCHK_M(h3)                                    \
	HCHK_M(h4)                                    \
	HCHK_M(h5)                                    \
	HCHK_M(h6)                                    \
	HCHK_M(h7)                                    \
	HCHK_M(h8)
#define HCHK_M9(h1, h2, h3, h4, h5, h6, h7, h8, h9) \
	HCHK_M(h1)                                        \
	HCHK_M(h2)                                        \
	HCHK_M(h3)                                        \
	HCHK_M(h4)                                        \
	HCHK_M(h5)                                        \
	HCHK_M(h6)                                        \
	HCHK_M(h7)                                        \
	HCHK_M(h8)                                        \
	HCHK_M(h9)

#define H_REG(hName) hookChecker<decltype(this), hook_s>::register_##hName(this, p)

#define H_REG1(h1) H_REG(h1);
#define H_REG2(h1, h2) \
	H_REG(h1);           \
	H_REG(h2);
#define H_REG3(h1, h2, h3) \
	H_REG(h1);               \
	H_REG(h2);               \
	H_REG(h3);
#define H_REG4(h1, h2, h3, h4) \
	H_REG(h1);                   \
	H_REG(h2);                   \
	H_REG(h3);                   \
	H_REG(h4);
#define H_REG5(h1, h2, h3, h4, h5) \
	H_REG(h1);                       \
	H_REG(h2);                       \
	H_REG(h3);                       \
	H_REG(h4);                       \
	H_REG(h5);
#define H_REG6(h1, h2, h3, h4, h5, h6) \
	H_REG(h1);                           \
	H_REG(h2);                           \
	H_REG(h3);                           \
	H_REG(h4);                           \
	H_REG(h5);                           \
	H_REG(h6);
#define H_REG7(h1, h2, h3, h4, h5, h6, h7) \
	H_REG(h1);                               \
	H_REG(h2);                               \
	H_REG(h3);                               \
	H_REG(h4);                               \
	H_REG(h5);                               \
	H_REG(h6);                               \
	H_REG(h7);
#define H_REG8(h1, h2, h3, h4, h5, h6, h7, h8) \
	H_REG(h1);                                   \
	H_REG(h2);                                   \
	H_REG(h3);                                   \
	H_REG(h4);                                   \
	H_REG(h5);                                   \
	H_REG(h6);                                   \
	H_REG(h7);                                   \
	H_REG(h8);
#define H_REG9(h1, h2, h3, h4, h5, h6, h7, h8, h9) \
	H_REG(h1);                                       \
	H_REG(h2);                                       \
	H_REG(h3);                                       \
	H_REG(h4);                                       \
	H_REG(h5);                                       \
	H_REG(h6);                                       \
	H_REG(h7);                                       \
	H_REG(h8);                                       \
	H_REG(h9);

#define DECLARE_HOOK(...)                                                      \
	HOOKS_ENUM(__VA_ARGS__)                                                      \
	OVERLOADED_MACRO(CREATE_MC, __VA_ARGS__)                                     \
	CREATE_METHOD_CHECKS(onRegister);                                             \
	template <class HookableClass, class hook_s> struct hookChecker {            \
		using H = std::remove_pointer_t<std::remove_reference_t<HookableClass>>;   \
		using hook_t = std::function<hook_s>;                                      \
		template <class P>                                                         \
		static std::enable_if_t<is_onRegister_callable<P, hook_s>::value, hook_t>  \
		    getOnRegister(P &p) {                                                  \
			return [&](H *hc) { p.onRegister(hc); };                                 \
		}                                                                          \
                                                                               \
		template <class P>                                                         \
		static std::enable_if_t<!is_onRegister_callable<P, hook_s>::value, hook_t> \
		    getOnRegister(P &) {                                                   \
			return [&](H *) {};                                                      \
		}                                                                          \
		OVERLOADED_MACRO(HCHK_M, __VA_ARGS__)                                      \
	};                                                                           \
	using hook_t = std::function<hook_s>;                                        \
	std::array<std::vector<hook_t>, Hooks::LAST> hooks;                          \
	template <class P> hook_t registerPlugin(P &p) {                             \
		OVERLOADED_MACRO(H_REG, __VA_ARGS__)                                       \
		return hookChecker<decltype(this), hook_s>::getOnRegister(p);              \
	}
