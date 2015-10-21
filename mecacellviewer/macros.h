#ifndef MVIEWER_MACROS_H
#define MVIEWER_MACROS_H
#include <map>
#include <functional>
#include <QString>

#define MECACELL_VIEWER_ADDITIONS(scenarClass)                                        \
	std::map<                                                                           \
	    QString,                                                                        \
	    std::map<QString, std::function<void(MecacellViewer::Renderer<scenarClass>*)>>> \
	    MCV_buttonMap;                                                                  \
	void MCV_InitializeInterfaceAdditions()

/*****************************************************************************
                    BELOW: member checker
              shamelessly taken from stackoverflow.com
/questions/87372/check-if-a-class-has-a-member-function-of-a-given-signature

******************************************************************************/

// Variadic to force ambiguity of class members.  C++11 and up.
template <typename... Args> struct ambiguate : public Args... {};

// Non-variadic version of the line above.
// template <typename A, typename B> struct ambiguate : public A, public B {};

template <typename A, typename = void> struct got_type : std::false_type {};

template <typename A> struct got_type<A> : std::true_type { typedef A type; };

template <typename T, T> struct sig_check : std::true_type {};

template <typename Alias, typename AmbiguitySeed> struct has_member {
	template <typename C> static char((&f(decltype(&C::value))))[1];
	template <typename C> static char((&f(...)))[2];

	// Make sure the member name is consistently spelled the same.
	static_assert((sizeof(f<AmbiguitySeed>(0)) == 1),
	              "Member name specified in AmbiguitySeed is different from member name "
	              "specified in Alias, or wrong Alias/AmbiguitySeed has been specified.");

	static bool const value = sizeof(f<Alias>(0)) == 2;
};
// Check for any member with given name, whether var, func, class, union, enum.
#define CREATE_MEMBER_CHECK(member)                                             \
                                                                                \
	template <typename T, typename = std::true_type> struct Alias_##member;       \
                                                                                \
	template <typename T>                                                         \
	struct Alias_##member<                                                        \
	    T, std::integral_constant<bool, got_type<decltype(&T::member)>::value>> { \
		static const decltype(&T::member) value;                                    \
	};                                                                            \
                                                                                \
	struct AmbiguitySeed_##member {                                               \
		char member;                                                                \
	};                                                                            \
                                                                                \
	template <typename T> struct has_member_##member {                            \
		static const bool value =                                                   \
		    has_member<Alias_##member<ambiguate<T, AmbiguitySeed_##member>>,        \
		               Alias_##member<AmbiguitySeed_##member>>::value;              \
	}
#endif
