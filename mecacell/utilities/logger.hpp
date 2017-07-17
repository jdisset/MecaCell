#ifndef LOGGER_HPP
#define LOGGER_HPP
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

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

// maybe needed 
//constexpr decltype(WARN::tag) WARN::tag;
//constexpr decltype(ERR::tag) ERR::tag;
//constexpr decltype(INF::tag) INF::tag;
//constexpr decltype(DBG::tag) DBG::tag;
//constexpr decltype(SUC::tag) SUC::tag;
//constexpr decltype(WARN::color) WARN::color;
//constexpr decltype(ERR::color) ERR::color;
//constexpr decltype(INF::color) INF::color;
//constexpr decltype(DBG::color) DBG::color;
//constexpr decltype(SUC::color) SUC::color;

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
#ifndef MECACELL_LOGGER_WARN_DISABLE
	static constexpr const bool enabled = true;
#else
	static constexpr const bool enabled = false;
#endif
	static constexpr const auto color = YELLOW;
	static constexpr const auto tag = "⚠ ";
};
struct ERR {
#ifndef MECACELL_LOGGER_ERR_DISABLE
	static constexpr const bool enabled = true;
#else
	static constexpr const bool enabled = false;
#endif
	static constexpr const auto color = RED;
	static constexpr const auto tag = " ✖ ";
};
struct INF {
#ifndef MECACELL_LOGGER_INF_DISABLE
	static constexpr const bool enabled = true;
#else
	static constexpr const bool enabled = false;
#endif
	static constexpr const auto color = BLUE;
	static constexpr const auto tag = "⟢ ";
};
struct DBG {
#ifndef MECACELL_LOGGER_DBG_DISABLE
	static constexpr const bool enabled = true;
#else
	static constexpr const bool enabled = false;
#endif
	static constexpr const auto color = MAGENTA;
	static constexpr const auto tag = "☵ ";
};
struct SUC {
#ifndef MECACELL_LOGGER_SUC_DISABLE
	static constexpr const bool enabled = true;
#else
	static constexpr const bool enabled = false;
#endif
	static constexpr const auto color = BOLDGREEN;
	static constexpr const auto tag = " ✓ ";
};

template <typename Type, typename... Args> void logger(Args&&... args) {
	if (Type::enabled) {
		time_t rawtime;
		time(&rawtime);
		struct tm* timeinfo = localtime(&rawtime);
		char buffer[80];
		strftime(buffer, 80, "%F %H:%M:%S", timeinfo);

		std::ostringstream os;
		os << BOLDBLACK << "[" << buffer << "]" << RESET << Type::color << " " << Type::tag
		   << BOLDBLACK << " : " << RESET;
		os << sublogger(std::forward<Args>(args)...) << std::endl;
		std::cerr << os.str();
	}
}
}
#endif
