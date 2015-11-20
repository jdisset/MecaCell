#ifndef MECACELL_INTROSPECT_HPP
#define MECACELL_INTROSPECT_HPP
#include <iostream>
#include <string>
#include <functional>
#include <typeinfo>
#include <cxxabi.h>

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

#define CREATE_METHOD_CHECKS(method)                                                    \
	/*checks if Class C has a method with the given signature */                          \
	template <typename C, typename F> struct has_##method##_signature {};                 \
	template <typename C, typename Ret, typename... Args>                                 \
	struct has_##method##_signature<C, Ret(Args...)> {                                    \
		using COUILLE = Ret (C::*)(Args...);                                                \
		template <typename T> static constexpr bool has(...) { return false; }              \
		template <typename T>                                                               \
		static constexpr bool has(decltype(static_cast<COUILLE>(&T::method)) *) {           \
			return std::is_same<COUILLE, decltype(static_cast<COUILLE>(&T::method))>::value;  \
		}                                                                                   \
		static constexpr bool value = has<C>(nullptr);                                      \
	};                                                                                    \
	/*checks if Class C has a method with one of the given signatures*/                   \
	template <typename C, typename... Signatures>                                         \
	struct has_##method##_signatures : std::false_type {};                                \
	template <typename C, typename First, typename... Rest>                               \
	struct has_##method##_signatures<C, First, Rest...> {                                 \
		static constexpr bool value = has_##method##_signature<C, First>::value ||          \
		                              has_##method##_signatures<C, Rest...>::value;         \
	};                                                                                    \
	/*checks if Class C has a method with the given name, without signature check*/       \
	/*! doesn't work with overloaded methods!!! */                                        \
	template <typename C> struct has_##method##_method {                                  \
		template <typename> static constexpr std::false_type has(...);                      \
		template <typename T>                                                               \
		static constexpr std::true_type has(decltype(&T::method) *stuff = 0);               \
		using type = decltype(has<C>());                                                    \
		static constexpr bool value = type::value;                                          \
	};                                                                                    \
	/*makes sure that if Class C has a certain method, it has one of the given            \
	 * signatures*/                                                                       \
	/*! doesn't work for overloaded methods*/                                             \
	template <typename T, typename... Signatures>                                         \
	static constexpr bool require_##method##_signatures(                                  \
	    typename std::enable_if<has_##method##_method<T>::value, T *>::type = nullptr) {  \
		static_assert(has_##method##_signatures<T, Signatures...>::value,                   \
		              "Wrong signature for method ##method");                               \
		return true;                                                                        \
	}                                                                                     \
	template <typename T, typename... Signatures>                                         \
	static constexpr bool require_##method##_signatures(                                  \
	    typename std::enable_if<!has_##method##_method<T>::value, T *>::type = nullptr) { \
		return false;                                                                       \
	}
#endif
