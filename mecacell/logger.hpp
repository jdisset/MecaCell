#ifndef LOGGER_HPP
#define LOGGER_HPP
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>

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

#define LOGGERINSTANCE(var, logger) auto var = logger
#define DEBUG(name) Log(#name, LogLvl::debug, __func__)
#define ERROR(name) Log(#name, LogLvl::error, __func__)
#define WARNING(name) Log(#name, LogLvl::warning, __func__)

// example usage:
// DBG(membrane) << "kikoo" << endl;

enum class LogLvl { error, warning, debug };
struct LogBuf : public std::stringbuf {
	string header;
	string name;
	LogLvl lvl;
	string func;

	LogBuf(const LogBuf& lb)
	    : header(lb.header), name(lb.name), lvl(lb.lvl), func(lb.func) {}
	LogBuf(const string&& n, const LogLvl l, const string&& f)
	    : name(std::move(n)), lvl(l), func(std::move(f)) {
		std::transform(name.begin(), name.end(), name.begin(), ::toupper);
		std::stringstream headerstream;
		string COLOR;
		switch (lvl) {
			case LogLvl::error:
				COLOR = RED;
				break;
			case LogLvl::warning:
				COLOR = MAGENTA;
				break;
			default:
			case LogLvl::debug:
				COLOR = BOLDGREEN;
				break;
		}
		headerstream << COLOR << name << " " << BOLDBLACK << func << COLOR << "┣  " << RESET;
		header = headerstream.str();
	}

	virtual int sync() {
		cerr << header << str();
		str(std::string());
		return 0;
	}
};

struct Log : public std::ostream {
	LogBuf buf;
	Log(Log&& l) : buf(l.buf) {}

	template <typename... Args>
	Log(Args&&... args)
	    : std::ostream(&buf), buf(std::forward<Args>(args)...) {}
};
#endif
