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

// the following are LINUX ONLY terminal color codes.
#ifdef TERMINAL_COLORS
#define RESET "\033[0m"
#define BLACK "\033[30m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"
#define BOLDBLACK "\033[1m\033[30m"
#define BOLDRED "\033[1m\033[31m"
#define BOLDGREEN "\033[1m\033[32m"
#define BOLDYELLOW "\033[1m\033[33m"
#define BOLDBLUE "\033[1m\033[34m"
#define BOLDMAGENTA "\033[1m\033[35m"
#define BOLDCYAN "\033[1m\033[36m"
#define BOLDWHITE "\033[1m\033[37m"
#else
#define RESET ""
#define BLACK ""
#define RED ""
#define GREEN ""
#define YELLOW ""
#define BLUE ""
#define MAGENTA ""
#define CYAN ""
#define WHITE ""
#define BOLDBLACK ""
#define BOLDRED ""
#define BOLDGREEN ""
#define BOLDYELLOW ""
#define BOLDBLUE ""
#define BOLDMAGENTA ""
#define BOLDCYAN ""
#define BOLDWHITE ""
#endif

#define concat(first, second) first second

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
	static constexpr auto tag = concat(concat(YELLOW, " ⚠  [WARNING]"), RESET);
};

struct ERR {
	static constexpr auto tag = concat(concat(RED, " ✖ [ERROR]"), RESET);
};
struct INF {
	static constexpr auto tag = " ⟢ ";
};
struct SUC {
	static constexpr auto tag = concat(concat(BOLDGREEN, " ✓ [SUCCESS]"), RESET);
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

#endif
