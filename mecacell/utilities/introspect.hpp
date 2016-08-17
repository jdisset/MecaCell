#ifndef MECACELL_INTROSPECT_HPP
#define MECACELL_INTROSPECT_HPP
#include <cxxabi.h>
#include <functional>
#include <iostream>
#include <string>
#include <typeinfo>
#include <utility>

template <typename... Ts> struct make_void { typedef void type; };
template <typename... Ts> using void_t = typename make_void<Ts...>::type;
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
		template <typename T>                                                                \
		static constexpr auto has(int) ->                                                    \
		    typename std::is_same<SIG, decltype(static_cast<SIG>(&T::method))>::type;        \
		template <typename T>                                                                \
		static constexpr auto has(...) -> typename std::false_type::type;                    \
                                                                                         \
		static void debug() {                                                                \
			std::cout << " ---------------- signature -------------" << std::endl;             \
			DEBUG_TYPE((Ret(Args...)));                                                        \
			DEBUG_TYPE((SIG));                                                                 \
			DEBUG_TYPE((decltype(&C::method)));                                                \
			std::cout << "myValue = " << value << std::endl;                                   \
		}                                                                                    \
		static constexpr bool value = decltype(has<C>(0))::value;                            \
	};                                                                                     \
	/*checks if Class C has a method with one of the given signatures*/                    \
	template <typename C, typename... Signatures> struct has_##method##_signatures {       \
		static constexpr bool value() { return false; }                                      \
		static void debug() {}                                                               \
	};                                                                                     \
	template <typename C, typename First, typename... Rest>                                \
	struct has_##method##_signatures<C, First, Rest...> {                                  \
		static constexpr bool value() {                                                      \
			return has_##method##_signature<C, First>::value ||                                \
			       has_##method##_signatures<C, Rest...>::value();                             \
		}                                                                                    \
		static void debug() {                                                                \
			std::cerr << "==========================================" << std::endl             \
			          << "signature.value = " << has_##method##_signature<C, First>::value     \
			          << std::endl;                                                            \
			std::cerr << "signature.has = "                                                    \
			          << decltype(                                                             \
			                 has_##method##_signature<C, First>::template has<C>(0))::value    \
			          << std::endl;                                                            \
			std::cerr << "me.value= " << value() << std::endl;                                 \
			has_##method##_signature<C, First>::debug();                                       \
			has_##method##_signatures<C, Rest...>::debug();                                    \
		}                                                                                    \
	};                                                                                     \
	/*checks if Class C has a method with the given name, without signature check*/        \
	/*! doesn't work with overloaded methods or class template!!! */                       \
	template <typename C> struct has_##method##_method {                                   \
		template <typename> static constexpr std::false_type has(...);                       \
		template <typename T>                                                                \
		static constexpr std::true_type has(decltype(&T::method) *stuff = 0);                \
		static constexpr bool value = decltype(has<C>())::value;                             \
	}

// lil' helper that explodes a tuple and forward its content to a func
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
