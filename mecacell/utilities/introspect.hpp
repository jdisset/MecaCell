#ifndef MECACELL_INTROSPECT_HPP
#define MECACELL_INTROSPECT_HPP
#include <cxxabi.h>
#include <functional>
#include <iostream>
#include <string>
#include <typeinfo>
#include <utility>

namespace MecaCell {
template <typename... Ts> struct make_void { typedef void type; };
template <typename... Ts> using void_t = typename make_void<Ts...>::type;
}

// displays type name, for debugging purpose
#define DEBUG_TYPE(x)       \
	do {                      \
		typedef void(*T) x;     \
		debug_type<T>(T(), #x); \
	} while (0)

template <typename T> struct debug_type {
	template <typename U> debug_type(void (*)(U), const std::string &p_str) {
		std::string str(p_str.begin() + 1, p_str.end() - 1);
		std::cout << str << " => ";
		char *name = nullptr;
		int status;
		name = abi::__cxa_demangle(typeid(U).name(), 0, 0, &status);
		if (name) {
			std::cout << name << std::endl;
		} else {
			std::cout << typeid(U).name() << std::endl;
		}
		free(name);
	}
};

#define CREATE_METHOD_CHECKS(method)                                                     \
	/* checks if Class C has a method callable using the given signature */                \
	template <typename C, typename F> struct is_##method##_callable {};                    \
	template <typename C, typename Ret, typename... Args>                                  \
	struct is_##method##_callable<C, Ret(Args...)> {                                       \
		template <typename T> static constexpr bool is(...) { return false; }                \
		template <typename T>                                                                \
		static constexpr bool is(                                                            \
		    typename std::is_same<                                                           \
		        Ret, decltype(std::declval<T>().method(std::declval<Args...>()))>::type *) { \
			return true;                                                                       \
		}                                                                                    \
		static constexpr bool value = is<C>(nullptr);                                        \
	};                                                                                     \
	/* checks if Class C has a method with the exact given signature */                    \
	template <typename C, typename F> struct has_##method##_signature {};                  \
	template <typename C, typename Ret, typename... Args>                                  \
	struct has_##method##_signature<C, Ret(Args...)> {                                     \
		using SIG = Ret (C::*)(Args...);                                                     \
		template <typename T, T> struct same;                                                \
		template <typename T>                                                                \
		static constexpr auto has(same<SIG, &T::method> *) -> typename std::true_type::type; \
		template <typename T>                                                                \
		static constexpr auto has(...) -> typename std::false_type::type;                    \
		static constexpr bool value = decltype(has<C>(0))::value;                            \
	}

// helper that explodes a tuple and forward its content to a func
// (+ other args at the begining) ... C++14 only :'(
// template <typename F, typename... OtherArgs, typename... TupleTypes, std::size_t...
// Ind>
// auto callExpand(F &&f, OtherArgs &&... otherArgs, const std::tuple<TupleTypes...>
// &tuple,
// std::index_sequence<Ind...>) {
// return std::forward<F>(f)(std::forward<OtherArgs>(otherArgs)...,
// std::get<Ind>(tuple)...);
//}
// template <typename F, typename... OtherArgs, typename... TupleTypes>
// auto callWithExpandedTuple(F &&f, OtherArgs &&... otherArgs,
// const std::tuple<TupleTypes...> &tuple) {
// return callExpand(std::forward<F>(f), std::forward<OtherArgs>(otherArgs)..., tuple,
// std::index_sequence_for<TupleTypes...>());
//}

#endif
