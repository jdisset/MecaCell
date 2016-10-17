#ifndef LOGGER_HPP
#define LOGGER_HPP
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

using std::ostream;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

namespace MecaCell {
// the following are LINUX ONLY terminal color codes.
#ifdef MECACELL_TERMINAL_COLORS
const constexpr static char RESET[] = "\033[0m";
const constexpr static char BLACK[] = "\033[30m";
const constexpr static char RED[] = "\033[31m";
const constexpr static char GREEN[] = "\033[32m";
const constexpr static char YELLOW[] = "\033[33m";
const constexpr static char BLUE[] = "\033[34m";
const constexpr static char MAGENTA[] = "\033[35m";
const constexpr static char CYAN[] = "\033[36m";
const constexpr static char WHITE[] = "\033[37m";
const constexpr static char BOLDBLACK[] = "\033[1m\033[30m";
const constexpr static char BOLDRED[] = "\033[1m\033[31m";
const constexpr static char BOLDGREEN[] = "\033[1m\033[32m";
const constexpr static char BOLDYELLOW[] = "\033[1m\033[33m";
const constexpr static char BOLDBLUE[] = "\033[1m\033[34m";
const constexpr static char BOLDMAGENTA[] = "\033[1m\033[35m";
const constexpr static char BOLDCYAN[] = "\033[1m\033[36m";
const constexpr static char BOLDWHITE[] = "\033[1m\033[37m";
#else
const constexpr static char RESET[] = "";
const constexpr static char BLACK[] = "";
const constexpr static char RED[] = "";
const constexpr static char GREEN[] = "";
const constexpr static char YELLOW[] = "";
const constexpr static char BLUE[] = "";
const constexpr static char MAGENTA[] = "";
const constexpr static char CYAN[] = "";
const constexpr static char WHITE[] = "";
const constexpr static char BOLDBLACK[] = "";
const constexpr static char BOLDRED[] = "";
const constexpr static char BOLDGREEN[] = "";
const constexpr static char BOLDYELLOW[] = "";
const constexpr static char BOLDBLUE[] = "";
const constexpr static char BOLDMAGENTA[] = "";
const constexpr static char BOLDCYAN[] = "";
const constexpr static char BOLDWHITE[] = "";
#endif

// code for concatenating constexpr strings
template <unsigned...> struct seq { using type = seq; };
template <unsigned N, unsigned... Is>
struct gen_seq_x : gen_seq_x<N - 1, N - 1, Is...> {};
template <unsigned... Is> struct gen_seq_x<0, Is...> : seq<Is...> {};
template <unsigned N> using gen_seq = typename gen_seq_x<N>::type;
template <size_t S> using size = std::integral_constant<size_t, S>;

template <class T, size_t N> constexpr size<N> length(T const (&)[N]) { return {}; }
template <class T, size_t N> constexpr size<N> length(std::array<T, N> const&) {
	return {};
}

template <class T> using length_t = decltype(length(std::declval<T>()));

constexpr size_t string_size() { return 0; }
template <class... Ts> constexpr size_t string_size(size_t i, Ts... ts) {
	return (i ? i - 1 : 0) + string_size(ts...);
}
template <class... Ts> using string_length = size<string_size(length_t<Ts>{}...)>;

template <class... Ts>
using combined_string = std::array<char, string_length<Ts...>{} + 1>;

template <class Lhs, class Rhs, unsigned... I1, unsigned... I2>
constexpr const combined_string<Lhs, Rhs> concat_impl(Lhs const& lhs, Rhs const& rhs,
                                                      seq<I1...>, seq<I2...>) {
	// the '\0' adds to symmetry:
	return {{lhs[I1]..., rhs[I2]..., '\0'}};
}

template <class Lhs, class Rhs>
constexpr const combined_string<Lhs, Rhs> concat(Lhs const& lhs, Rhs const& rhs) {
	return concat_impl(lhs, rhs, gen_seq<string_length<Lhs>{}>{},
	                   gen_seq<string_length<Rhs>{}>{});
}

template <class T0, class T1, class... Ts>
constexpr const combined_string<T0, T1, Ts...> concat(T0 const& t0, T1 const& t1,
                                                      Ts const&... ts) {
	return concat(t0, concat(t1, ts...));
}

template <class T> constexpr const combined_string<T> concat(T const& t) {
	return concat(t, "");
}
constexpr const combined_string<> concat() { return concat(""); }

// LOGGER
template <typename T> std::string sublogger(T&& t) {
	std::ostringstream os;
	os << t;
	return os.str();
}

template <typename T, typename... Args> std::string sublogger(T&& t, Args&&... args) {
	std::ostringstream os;
	os << t << sublogger(std::forward<Args>(args)...);
	return os.str();
}

struct WARN {
	static constexpr auto tag = concat(YELLOW, " ⚠  [WARNING]", RESET);
};

struct ERR {
	static constexpr auto tag = concat(RED, " ✖ [ERROR]", RESET);
};
struct INF {
	static constexpr auto tag = " ⟢ ";
};
struct DBG {
	static constexpr auto tag = " | ";
};
struct SUC {
	static constexpr auto tag = concat(BOLDGREEN, " ✓ [SUCCESS]", RESET);
};

template <typename Type, typename... Args> void logger(Args&&... args) {
	std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::string tstr(ctime(&t));
	tstr = tstr.substr(0, tstr.size() - 1);
	std::ostringstream os;
	os << BOLDBLACK << tstr << RESET << "| " << Type::tag << "𝌅 ";
	os << sublogger(std::forward<Args>(args)...) << std::endl;
	std::cerr << os.str();
}
}
#endif
