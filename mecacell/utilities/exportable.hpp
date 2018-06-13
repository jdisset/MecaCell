#ifndef EXPORTABLE_HPP
#define EXPORTABLE_HPP
#include <functional>
#include <iostream>
#include <string>
#include "json.hpp"

// get number of arguments with __NARG__
#define __NARG__(...) __NARG_I_(__VA_ARGS__, __RSEQ_N())
#define __NARG_I_(...) __ARG_N(__VA_ARGS__)
#define __ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
                _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30,  \
                _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44,  \
                _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,  \
                _59, _60, _61, _62, _63, N, ...)                                       \
	N
#define __RSEQ_N()                                                                    \
	63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, \
	    42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, \
	    22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

// general definition for any function name
#define _VFUNC_(name, n) name##n
#define _VFUNC(name, n) _VFUNC_(name, n)
#define VFUNC(func, ...) _VFUNC(func, __NARG__(__VA_ARGS__))(__VA_ARGS__)

#define MECACELL_STR_FIRST(k, v) #k
#define MECACELL_FIRST(k, v) k
#define MECACELL_SECOND(k, v) v
#define KV(p) (p, p)

#define MECACELL_K(...) MECACELL_FIRST __VA_ARGS__
#define MECACELL_SK(...) MECACELL_STR_FIRST __VA_ARGS__
#define MECACELL_V(...) MECACELL_SECOND __VA_ARGS__

#define EXPORTABLE_INHERIT(...) VFUNC(EXPORTABLE_INHERIT, __VA_ARGS__)
#define EXPORTABLE(...) VFUNC(EXPORTABLE, __VA_ARGS__)

#define EXPORTABLE_INHERIT3(S, Base, p0)                                            \
	struct Exportable : ExportableType<Base>::type {                                  \
		using BType = ExportableType<Base>::type;                                       \
		using json = nlohmann::json;                                                    \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);      \
		Exportable() {}                                                                 \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }           \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                  \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                     \
		}                                                                               \
		void from_json(const json& o) {                                                 \
			if (o.count(MECACELL_SK(p0)))                                                 \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();        \
		}                                                                               \
		json to_json() const {                                                          \
			auto b = BType::to_json();                                                    \
			auto r = to_jsonL(*this);                                                     \
			if (!b.empty()) r.update(b);                                                  \
			return r;                                                                     \
		}                                                                               \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {     \
			return s.saveJ(MECACELL_SK(p0), s.MECACELL_K(p0));                            \
		};                                                                              \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); }; \
	};                                                                                \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE2(S, v0) EXPORTABLE_INHERIT3(S, BaseExportable, v0)

#define EXPORTABLE_INHERIT4(S, Base, p0, p1)                                        \
	struct Exportable : ExportableType<Base>::type {                                  \
		using BType = ExportableType<Base>::type;                                       \
		using json = nlohmann::json;                                                    \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);      \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);      \
		Exportable() {}                                                                 \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }           \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                  \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                     \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                     \
		}                                                                               \
		void from_json(const json& o) {                                                 \
			if (o.count(MECACELL_SK(p0)))                                                 \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();        \
			if (o.count(MECACELL_SK(p1)))                                                 \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();        \
		}                                                                               \
		json to_json() const {                                                          \
			auto b = BType::to_json();                                                    \
			auto r = to_jsonL(*this);                                                     \
			if (!b.empty()) r.update(b);                                                  \
			return r;                                                                     \
		}                                                                               \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {     \
			return s.saveJ(MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1),            \
			               s.MECACELL_K(p1));                                             \
		};                                                                              \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); }; \
	};                                                                                \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE3(S, v0, v1) EXPORTABLE_INHERIT4(S, BaseExportable, v0, v1)

#define EXPORTABLE_INHERIT5(S, Base, p0, p1, p2)                                    \
	struct Exportable : ExportableType<Base>::type {                                  \
		using BType = ExportableType<Base>::type;                                       \
		using json = nlohmann::json;                                                    \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);      \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);      \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);      \
		Exportable() {}                                                                 \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }           \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                  \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                     \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                     \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                     \
		}                                                                               \
		void from_json(const json& o) {                                                 \
			if (o.count(MECACELL_SK(p0)))                                                 \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();        \
			if (o.count(MECACELL_SK(p1)))                                                 \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();        \
			if (o.count(MECACELL_SK(p2)))                                                 \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();        \
		}                                                                               \
		json to_json() const {                                                          \
			auto b = BType::to_json();                                                    \
			auto r = to_jsonL(*this);                                                     \
			if (!b.empty()) r.update(b);                                                  \
			return r;                                                                     \
		}                                                                               \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {     \
			return s.saveJ(MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1),            \
			               s.MECACELL_K(p1), MECACELL_SK(p2), s.MECACELL_K(p2));          \
		};                                                                              \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); }; \
	};                                                                                \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE4(S, v0, v1, v2) EXPORTABLE_INHERIT5(S, BaseExportable, v0, v1, v2)

#define EXPORTABLE_INHERIT6(S, Base, p0, p1, p2, p3)                                \
	struct Exportable : ExportableType<Base>::type {                                  \
		using BType = ExportableType<Base>::type;                                       \
		using json = nlohmann::json;                                                    \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);      \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);      \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);      \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);      \
		Exportable() {}                                                                 \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }           \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                  \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                     \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                     \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                     \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                     \
		}                                                                               \
		void from_json(const json& o) {                                                 \
			if (o.count(MECACELL_SK(p0)))                                                 \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();        \
			if (o.count(MECACELL_SK(p1)))                                                 \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();        \
			if (o.count(MECACELL_SK(p2)))                                                 \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();        \
			if (o.count(MECACELL_SK(p3)))                                                 \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();        \
		}                                                                               \
		json to_json() const {                                                          \
			auto b = BType::to_json();                                                    \
			auto r = to_jsonL(*this);                                                     \
			if (!b.empty()) r.update(b);                                                  \
			return r;                                                                     \
		}                                                                               \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {     \
			return s.saveJ(MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1),            \
			               s.MECACELL_K(p1), MECACELL_SK(p2), s.MECACELL_K(p2),           \
			               MECACELL_SK(p3), s.MECACELL_K(p3));                            \
		};                                                                              \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); }; \
	};                                                                                \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE5(S, v0, v1, v2, v3) \
	EXPORTABLE_INHERIT6(S, BaseExportable, v0, v1, v2, v3)

#define EXPORTABLE_INHERIT7(S, Base, p0, p1, p2, p3, p4)                            \
	struct Exportable : ExportableType<Base>::type {                                  \
		using BType = ExportableType<Base>::type;                                       \
		using json = nlohmann::json;                                                    \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);      \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);      \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);      \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);      \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);      \
		Exportable() {}                                                                 \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }           \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                  \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                     \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                     \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                     \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                     \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                     \
		}                                                                               \
		void from_json(const json& o) {                                                 \
			if (o.count(MECACELL_SK(p0)))                                                 \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();        \
			if (o.count(MECACELL_SK(p1)))                                                 \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();        \
			if (o.count(MECACELL_SK(p2)))                                                 \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();        \
			if (o.count(MECACELL_SK(p3)))                                                 \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();        \
			if (o.count(MECACELL_SK(p4)))                                                 \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();        \
		}                                                                               \
		json to_json() const {                                                          \
			auto b = BType::to_json();                                                    \
			auto r = to_jsonL(*this);                                                     \
			if (!b.empty()) r.update(b);                                                  \
			return r;                                                                     \
		}                                                                               \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {     \
			return s.saveJ(MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1),            \
			               s.MECACELL_K(p1), MECACELL_SK(p2), s.MECACELL_K(p2),           \
			               MECACELL_SK(p3), s.MECACELL_K(p3), MECACELL_SK(p4),            \
			               s.MECACELL_K(p4));                                             \
		};                                                                              \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); }; \
	};                                                                                \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE6(S, v0, v1, v2, v3, v4) \
	EXPORTABLE_INHERIT7(S, BaseExportable, v0, v1, v2, v3, v4)

#define EXPORTABLE_INHERIT8(S, Base, p0, p1, p2, p3, p4, p5)                        \
	struct Exportable : ExportableType<Base>::type {                                  \
		using BType = ExportableType<Base>::type;                                       \
		using json = nlohmann::json;                                                    \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);      \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);      \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);      \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);      \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);      \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);      \
		Exportable() {}                                                                 \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }           \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                  \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                     \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                     \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                     \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                     \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                     \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                     \
		}                                                                               \
		void from_json(const json& o) {                                                 \
			if (o.count(MECACELL_SK(p0)))                                                 \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();        \
			if (o.count(MECACELL_SK(p1)))                                                 \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();        \
			if (o.count(MECACELL_SK(p2)))                                                 \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();        \
			if (o.count(MECACELL_SK(p3)))                                                 \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();        \
			if (o.count(MECACELL_SK(p4)))                                                 \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();        \
			if (o.count(MECACELL_SK(p5)))                                                 \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();        \
		}                                                                               \
		json to_json() const {                                                          \
			auto b = BType::to_json();                                                    \
			auto r = to_jsonL(*this);                                                     \
			if (!b.empty()) r.update(b);                                                  \
			return r;                                                                     \
		}                                                                               \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {     \
			return s.saveJ(MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1),            \
			               s.MECACELL_K(p1), MECACELL_SK(p2), s.MECACELL_K(p2),           \
			               MECACELL_SK(p3), s.MECACELL_K(p3), MECACELL_SK(p4),            \
			               s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5));          \
		};                                                                              \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); }; \
	};                                                                                \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE7(S, v0, v1, v2, v3, v4, v5) \
	EXPORTABLE_INHERIT8(S, BaseExportable, v0, v1, v2, v3, v4, v5)

#define EXPORTABLE_INHERIT9(S, Base, p0, p1, p2, p3, p4, p5, p6)                    \
	struct Exportable : ExportableType<Base>::type {                                  \
		using BType = ExportableType<Base>::type;                                       \
		using json = nlohmann::json;                                                    \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);      \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);      \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);      \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);      \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);      \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);      \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);      \
		Exportable() {}                                                                 \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }           \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                  \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                     \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                     \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                     \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                     \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                     \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                     \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                     \
		}                                                                               \
		void from_json(const json& o) {                                                 \
			if (o.count(MECACELL_SK(p0)))                                                 \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();        \
			if (o.count(MECACELL_SK(p1)))                                                 \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();        \
			if (o.count(MECACELL_SK(p2)))                                                 \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();        \
			if (o.count(MECACELL_SK(p3)))                                                 \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();        \
			if (o.count(MECACELL_SK(p4)))                                                 \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();        \
			if (o.count(MECACELL_SK(p5)))                                                 \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();        \
			if (o.count(MECACELL_SK(p6)))                                                 \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();        \
		}                                                                               \
		json to_json() const {                                                          \
			auto b = BType::to_json();                                                    \
			auto r = to_jsonL(*this);                                                     \
			if (!b.empty()) r.update(b);                                                  \
			return r;                                                                     \
		}                                                                               \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {     \
			return s.saveJ(MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1),            \
			               s.MECACELL_K(p1), MECACELL_SK(p2), s.MECACELL_K(p2),           \
			               MECACELL_SK(p3), s.MECACELL_K(p3), MECACELL_SK(p4),            \
			               s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),           \
			               MECACELL_SK(p6), s.MECACELL_K(p6));                            \
		};                                                                              \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); }; \
	};                                                                                \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE8(S, v0, v1, v2, v3, v4, v5, v6) \
	EXPORTABLE_INHERIT9(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6)

#define EXPORTABLE_INHERIT10(S, Base, p0, p1, p2, p3, p4, p5, p6, p7)               \
	struct Exportable : ExportableType<Base>::type {                                  \
		using BType = ExportableType<Base>::type;                                       \
		using json = nlohmann::json;                                                    \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);      \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);      \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);      \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);      \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);      \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);      \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);      \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);      \
		Exportable() {}                                                                 \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }           \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                  \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                     \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                     \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                     \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                     \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                     \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                     \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                     \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                     \
		}                                                                               \
		void from_json(const json& o) {                                                 \
			if (o.count(MECACELL_SK(p0)))                                                 \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();        \
			if (o.count(MECACELL_SK(p1)))                                                 \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();        \
			if (o.count(MECACELL_SK(p2)))                                                 \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();        \
			if (o.count(MECACELL_SK(p3)))                                                 \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();        \
			if (o.count(MECACELL_SK(p4)))                                                 \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();        \
			if (o.count(MECACELL_SK(p5)))                                                 \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();        \
			if (o.count(MECACELL_SK(p6)))                                                 \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();        \
			if (o.count(MECACELL_SK(p7)))                                                 \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();        \
		}                                                                               \
		json to_json() const {                                                          \
			auto b = BType::to_json();                                                    \
			auto r = to_jsonL(*this);                                                     \
			if (!b.empty()) r.update(b);                                                  \
			return r;                                                                     \
		}                                                                               \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {     \
			return s.saveJ(                                                               \
			    MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1), s.MECACELL_K(p1),     \
			    MECACELL_SK(p2), s.MECACELL_K(p2), MECACELL_SK(p3), s.MECACELL_K(p3),     \
			    MECACELL_SK(p4), s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),     \
			    MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7), s.MECACELL_K(p7));    \
		};                                                                              \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); }; \
	};                                                                                \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE9(S, v0, v1, v2, v3, v4, v5, v6, v7) \
	EXPORTABLE_INHERIT10(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7)

#define EXPORTABLE_INHERIT11(S, Base, p0, p1, p2, p3, p4, p5, p6, p7, p8)           \
	struct Exportable : ExportableType<Base>::type {                                  \
		using BType = ExportableType<Base>::type;                                       \
		using json = nlohmann::json;                                                    \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);      \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);      \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);      \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);      \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);      \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);      \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);      \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);      \
		typename ExportableType<decltype(S::MECACELL_V(p8))>::type MECACELL_K(p8);      \
		Exportable() {}                                                                 \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }           \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                  \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                     \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                     \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                     \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                     \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                     \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                     \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                     \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                     \
			ExportableConstruct(MECACELL_K(p8), s.MECACELL_V(p8), s);                     \
		}                                                                               \
		void from_json(const json& o) {                                                 \
			if (o.count(MECACELL_SK(p0)))                                                 \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();        \
			if (o.count(MECACELL_SK(p1)))                                                 \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();        \
			if (o.count(MECACELL_SK(p2)))                                                 \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();        \
			if (o.count(MECACELL_SK(p3)))                                                 \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();        \
			if (o.count(MECACELL_SK(p4)))                                                 \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();        \
			if (o.count(MECACELL_SK(p5)))                                                 \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();        \
			if (o.count(MECACELL_SK(p6)))                                                 \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();        \
			if (o.count(MECACELL_SK(p7)))                                                 \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();        \
			if (o.count(MECACELL_SK(p8)))                                                 \
				MECACELL_K(p8) = o[MECACELL_SK(p8)].get<decltype(MECACELL_K(p8))>();        \
		}                                                                               \
		json to_json() const {                                                          \
			auto b = BType::to_json();                                                    \
			auto r = to_jsonL(*this);                                                     \
			if (!b.empty()) r.update(b);                                                  \
			return r;                                                                     \
		}                                                                               \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {     \
			return s.saveJ(MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1),            \
			               s.MECACELL_K(p1), MECACELL_SK(p2), s.MECACELL_K(p2),           \
			               MECACELL_SK(p3), s.MECACELL_K(p3), MECACELL_SK(p4),            \
			               s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),           \
			               MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7),            \
			               s.MECACELL_K(p7), MECACELL_SK(p8), s.MECACELL_K(p8));          \
		};                                                                              \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); }; \
	};                                                                                \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE10(S, v0, v1, v2, v3, v4, v5, v6, v7, v8) \
	EXPORTABLE_INHERIT11(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7, v8)

#define EXPORTABLE_INHERIT12(S, Base, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9)       \
	struct Exportable : ExportableType<Base>::type {                                  \
		using BType = ExportableType<Base>::type;                                       \
		using json = nlohmann::json;                                                    \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);      \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);      \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);      \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);      \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);      \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);      \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);      \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);      \
		typename ExportableType<decltype(S::MECACELL_V(p8))>::type MECACELL_K(p8);      \
		typename ExportableType<decltype(S::MECACELL_V(p9))>::type MECACELL_K(p9);      \
		Exportable() {}                                                                 \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }           \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                  \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                     \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                     \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                     \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                     \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                     \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                     \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                     \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                     \
			ExportableConstruct(MECACELL_K(p8), s.MECACELL_V(p8), s);                     \
			ExportableConstruct(MECACELL_K(p9), s.MECACELL_V(p9), s);                     \
		}                                                                               \
		void from_json(const json& o) {                                                 \
			if (o.count(MECACELL_SK(p0)))                                                 \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();        \
			if (o.count(MECACELL_SK(p1)))                                                 \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();        \
			if (o.count(MECACELL_SK(p2)))                                                 \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();        \
			if (o.count(MECACELL_SK(p3)))                                                 \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();        \
			if (o.count(MECACELL_SK(p4)))                                                 \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();        \
			if (o.count(MECACELL_SK(p5)))                                                 \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();        \
			if (o.count(MECACELL_SK(p6)))                                                 \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();        \
			if (o.count(MECACELL_SK(p7)))                                                 \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();        \
			if (o.count(MECACELL_SK(p8)))                                                 \
				MECACELL_K(p8) = o[MECACELL_SK(p8)].get<decltype(MECACELL_K(p8))>();        \
			if (o.count(MECACELL_SK(p9)))                                                 \
				MECACELL_K(p9) = o[MECACELL_SK(p9)].get<decltype(MECACELL_K(p9))>();        \
		}                                                                               \
		json to_json() const {                                                          \
			auto b = BType::to_json();                                                    \
			auto r = to_jsonL(*this);                                                     \
			if (!b.empty()) r.update(b);                                                  \
			return r;                                                                     \
		}                                                                               \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {     \
			return s.saveJ(                                                               \
			    MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1), s.MECACELL_K(p1),     \
			    MECACELL_SK(p2), s.MECACELL_K(p2), MECACELL_SK(p3), s.MECACELL_K(p3),     \
			    MECACELL_SK(p4), s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),     \
			    MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7), s.MECACELL_K(p7),     \
			    MECACELL_SK(p8), s.MECACELL_K(p8), MECACELL_SK(p9), s.MECACELL_K(p9));    \
		};                                                                              \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); }; \
	};                                                                                \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE11(S, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9) \
	EXPORTABLE_INHERIT12(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9)

#define EXPORTABLE_INHERIT13(S, Base, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)  \
	struct Exportable : ExportableType<Base>::type {                                  \
		using BType = ExportableType<Base>::type;                                       \
		using json = nlohmann::json;                                                    \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);      \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);      \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);      \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);      \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);      \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);      \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);      \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);      \
		typename ExportableType<decltype(S::MECACELL_V(p8))>::type MECACELL_K(p8);      \
		typename ExportableType<decltype(S::MECACELL_V(p9))>::type MECACELL_K(p9);      \
		typename ExportableType<decltype(S::MECACELL_V(p10))>::type MECACELL_K(p10);    \
		Exportable() {}                                                                 \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }           \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                  \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                     \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                     \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                     \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                     \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                     \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                     \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                     \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                     \
			ExportableConstruct(MECACELL_K(p8), s.MECACELL_V(p8), s);                     \
			ExportableConstruct(MECACELL_K(p9), s.MECACELL_V(p9), s);                     \
			ExportableConstruct(MECACELL_K(p10), s.MECACELL_V(p10), s);                   \
		}                                                                               \
		void from_json(const json& o) {                                                 \
			if (o.count(MECACELL_SK(p0)))                                                 \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();        \
			if (o.count(MECACELL_SK(p1)))                                                 \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();        \
			if (o.count(MECACELL_SK(p2)))                                                 \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();        \
			if (o.count(MECACELL_SK(p3)))                                                 \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();        \
			if (o.count(MECACELL_SK(p4)))                                                 \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();        \
			if (o.count(MECACELL_SK(p5)))                                                 \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();        \
			if (o.count(MECACELL_SK(p6)))                                                 \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();        \
			if (o.count(MECACELL_SK(p7)))                                                 \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();        \
			if (o.count(MECACELL_SK(p8)))                                                 \
				MECACELL_K(p8) = o[MECACELL_SK(p8)].get<decltype(MECACELL_K(p8))>();        \
			if (o.count(MECACELL_SK(p9)))                                                 \
				MECACELL_K(p9) = o[MECACELL_SK(p9)].get<decltype(MECACELL_K(p9))>();        \
			if (o.count(MECACELL_SK(p10)))                                                \
				MECACELL_K(p10) = o[MECACELL_SK(p10)].get<decltype(MECACELL_K(p10))>();     \
		}                                                                               \
		json to_json() const {                                                          \
			auto b = BType::to_json();                                                    \
			auto r = to_jsonL(*this);                                                     \
			if (!b.empty()) r.update(b);                                                  \
			return r;                                                                     \
		}                                                                               \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {     \
			return s.saveJ(                                                               \
			    MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1), s.MECACELL_K(p1),     \
			    MECACELL_SK(p2), s.MECACELL_K(p2), MECACELL_SK(p3), s.MECACELL_K(p3),     \
			    MECACELL_SK(p4), s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),     \
			    MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7), s.MECACELL_K(p7),     \
			    MECACELL_SK(p8), s.MECACELL_K(p8), MECACELL_SK(p9), s.MECACELL_K(p9),     \
			    MECACELL_SK(p10), s.MECACELL_K(p10));                                     \
		};                                                                              \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); }; \
	};                                                                                \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE12(S, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10) \
	EXPORTABLE_INHERIT13(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10)

#define EXPORTABLE_INHERIT14(S, Base, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11) \
	struct Exportable : ExportableType<Base>::type {                                      \
		using BType = ExportableType<Base>::type;                                           \
		using json = nlohmann::json;                                                        \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);          \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);          \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);          \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);          \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);          \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);          \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);          \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);          \
		typename ExportableType<decltype(S::MECACELL_V(p8))>::type MECACELL_K(p8);          \
		typename ExportableType<decltype(S::MECACELL_V(p9))>::type MECACELL_K(p9);          \
		typename ExportableType<decltype(S::MECACELL_V(p10))>::type MECACELL_K(p10);        \
		typename ExportableType<decltype(S::MECACELL_V(p11))>::type MECACELL_K(p11);        \
		Exportable() {}                                                                     \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }               \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                      \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                         \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                         \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                         \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                         \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                         \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                         \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                         \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                         \
			ExportableConstruct(MECACELL_K(p8), s.MECACELL_V(p8), s);                         \
			ExportableConstruct(MECACELL_K(p9), s.MECACELL_V(p9), s);                         \
			ExportableConstruct(MECACELL_K(p10), s.MECACELL_V(p10), s);                       \
			ExportableConstruct(MECACELL_K(p11), s.MECACELL_V(p11), s);                       \
		}                                                                                   \
		void from_json(const json& o) {                                                     \
			if (o.count(MECACELL_SK(p0)))                                                     \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();            \
			if (o.count(MECACELL_SK(p1)))                                                     \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();            \
			if (o.count(MECACELL_SK(p2)))                                                     \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();            \
			if (o.count(MECACELL_SK(p3)))                                                     \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();            \
			if (o.count(MECACELL_SK(p4)))                                                     \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();            \
			if (o.count(MECACELL_SK(p5)))                                                     \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();            \
			if (o.count(MECACELL_SK(p6)))                                                     \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();            \
			if (o.count(MECACELL_SK(p7)))                                                     \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();            \
			if (o.count(MECACELL_SK(p8)))                                                     \
				MECACELL_K(p8) = o[MECACELL_SK(p8)].get<decltype(MECACELL_K(p8))>();            \
			if (o.count(MECACELL_SK(p9)))                                                     \
				MECACELL_K(p9) = o[MECACELL_SK(p9)].get<decltype(MECACELL_K(p9))>();            \
			if (o.count(MECACELL_SK(p10)))                                                    \
				MECACELL_K(p10) = o[MECACELL_SK(p10)].get<decltype(MECACELL_K(p10))>();         \
			if (o.count(MECACELL_SK(p11)))                                                    \
				MECACELL_K(p11) = o[MECACELL_SK(p11)].get<decltype(MECACELL_K(p11))>();         \
		}                                                                                   \
		json to_json() const {                                                              \
			auto b = BType::to_json();                                                        \
			auto r = to_jsonL(*this);                                                         \
			if (!b.empty()) r.update(b);                                                      \
			return r;                                                                         \
		}                                                                                   \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {         \
			return s.saveJ(                                                                   \
			    MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1), s.MECACELL_K(p1),         \
			    MECACELL_SK(p2), s.MECACELL_K(p2), MECACELL_SK(p3), s.MECACELL_K(p3),         \
			    MECACELL_SK(p4), s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),         \
			    MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7), s.MECACELL_K(p7),         \
			    MECACELL_SK(p8), s.MECACELL_K(p8), MECACELL_SK(p9), s.MECACELL_K(p9),         \
			    MECACELL_SK(p10), s.MECACELL_K(p10), MECACELL_SK(p11), s.MECACELL_K(p11));    \
		};                                                                                  \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); };     \
	};                                                                                    \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE13(S, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11)              \
	EXPORTABLE_INHERIT14(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, \
	                     v11)

#define EXPORTABLE_INHERIT15(S, Base, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, \
                             p12)                                                       \
	struct Exportable : ExportableType<Base>::type {                                      \
		using BType = ExportableType<Base>::type;                                           \
		using json = nlohmann::json;                                                        \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);          \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);          \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);          \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);          \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);          \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);          \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);          \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);          \
		typename ExportableType<decltype(S::MECACELL_V(p8))>::type MECACELL_K(p8);          \
		typename ExportableType<decltype(S::MECACELL_V(p9))>::type MECACELL_K(p9);          \
		typename ExportableType<decltype(S::MECACELL_V(p10))>::type MECACELL_K(p10);        \
		typename ExportableType<decltype(S::MECACELL_V(p11))>::type MECACELL_K(p11);        \
		typename ExportableType<decltype(S::MECACELL_V(p12))>::type MECACELL_K(p12);        \
		Exportable() {}                                                                     \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }               \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                      \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                         \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                         \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                         \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                         \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                         \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                         \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                         \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                         \
			ExportableConstruct(MECACELL_K(p8), s.MECACELL_V(p8), s);                         \
			ExportableConstruct(MECACELL_K(p9), s.MECACELL_V(p9), s);                         \
			ExportableConstruct(MECACELL_K(p10), s.MECACELL_V(p10), s);                       \
			ExportableConstruct(MECACELL_K(p11), s.MECACELL_V(p11), s);                       \
			ExportableConstruct(MECACELL_K(p12), s.MECACELL_V(p12), s);                       \
		}                                                                                   \
		void from_json(const json& o) {                                                     \
			if (o.count(MECACELL_SK(p0)))                                                     \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();            \
			if (o.count(MECACELL_SK(p1)))                                                     \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();            \
			if (o.count(MECACELL_SK(p2)))                                                     \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();            \
			if (o.count(MECACELL_SK(p3)))                                                     \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();            \
			if (o.count(MECACELL_SK(p4)))                                                     \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();            \
			if (o.count(MECACELL_SK(p5)))                                                     \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();            \
			if (o.count(MECACELL_SK(p6)))                                                     \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();            \
			if (o.count(MECACELL_SK(p7)))                                                     \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();            \
			if (o.count(MECACELL_SK(p8)))                                                     \
				MECACELL_K(p8) = o[MECACELL_SK(p8)].get<decltype(MECACELL_K(p8))>();            \
			if (o.count(MECACELL_SK(p9)))                                                     \
				MECACELL_K(p9) = o[MECACELL_SK(p9)].get<decltype(MECACELL_K(p9))>();            \
			if (o.count(MECACELL_SK(p10)))                                                    \
				MECACELL_K(p10) = o[MECACELL_SK(p10)].get<decltype(MECACELL_K(p10))>();         \
			if (o.count(MECACELL_SK(p11)))                                                    \
				MECACELL_K(p11) = o[MECACELL_SK(p11)].get<decltype(MECACELL_K(p11))>();         \
			if (o.count(MECACELL_SK(p12)))                                                    \
				MECACELL_K(p12) = o[MECACELL_SK(p12)].get<decltype(MECACELL_K(p12))>();         \
		}                                                                                   \
		json to_json() const {                                                              \
			auto b = BType::to_json();                                                        \
			auto r = to_jsonL(*this);                                                         \
			if (!b.empty()) r.update(b);                                                      \
			return r;                                                                         \
		}                                                                                   \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {         \
			return s.saveJ(                                                                   \
			    MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1), s.MECACELL_K(p1),         \
			    MECACELL_SK(p2), s.MECACELL_K(p2), MECACELL_SK(p3), s.MECACELL_K(p3),         \
			    MECACELL_SK(p4), s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),         \
			    MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7), s.MECACELL_K(p7),         \
			    MECACELL_SK(p8), s.MECACELL_K(p8), MECACELL_SK(p9), s.MECACELL_K(p9),         \
			    MECACELL_SK(p10), s.MECACELL_K(p10), MECACELL_SK(p11), s.MECACELL_K(p11),     \
			    MECACELL_SK(p12), s.MECACELL_K(p12));                                         \
		};                                                                                  \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); };     \
	};                                                                                    \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE14(S, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12)         \
	EXPORTABLE_INHERIT15(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, \
	                     v11, v12)

#define EXPORTABLE_INHERIT16(S, Base, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, \
                             p12, p13)                                                  \
	struct Exportable : ExportableType<Base>::type {                                      \
		using BType = ExportableType<Base>::type;                                           \
		using json = nlohmann::json;                                                        \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);          \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);          \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);          \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);          \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);          \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);          \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);          \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);          \
		typename ExportableType<decltype(S::MECACELL_V(p8))>::type MECACELL_K(p8);          \
		typename ExportableType<decltype(S::MECACELL_V(p9))>::type MECACELL_K(p9);          \
		typename ExportableType<decltype(S::MECACELL_V(p10))>::type MECACELL_K(p10);        \
		typename ExportableType<decltype(S::MECACELL_V(p11))>::type MECACELL_K(p11);        \
		typename ExportableType<decltype(S::MECACELL_V(p12))>::type MECACELL_K(p12);        \
		typename ExportableType<decltype(S::MECACELL_V(p13))>::type MECACELL_K(p13);        \
		Exportable() {}                                                                     \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }               \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                      \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                         \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                         \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                         \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                         \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                         \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                         \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                         \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                         \
			ExportableConstruct(MECACELL_K(p8), s.MECACELL_V(p8), s);                         \
			ExportableConstruct(MECACELL_K(p9), s.MECACELL_V(p9), s);                         \
			ExportableConstruct(MECACELL_K(p10), s.MECACELL_V(p10), s);                       \
			ExportableConstruct(MECACELL_K(p11), s.MECACELL_V(p11), s);                       \
			ExportableConstruct(MECACELL_K(p12), s.MECACELL_V(p12), s);                       \
			ExportableConstruct(MECACELL_K(p13), s.MECACELL_V(p13), s);                       \
		}                                                                                   \
		void from_json(const json& o) {                                                     \
			if (o.count(MECACELL_SK(p0)))                                                     \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();            \
			if (o.count(MECACELL_SK(p1)))                                                     \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();            \
			if (o.count(MECACELL_SK(p2)))                                                     \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();            \
			if (o.count(MECACELL_SK(p3)))                                                     \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();            \
			if (o.count(MECACELL_SK(p4)))                                                     \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();            \
			if (o.count(MECACELL_SK(p5)))                                                     \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();            \
			if (o.count(MECACELL_SK(p6)))                                                     \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();            \
			if (o.count(MECACELL_SK(p7)))                                                     \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();            \
			if (o.count(MECACELL_SK(p8)))                                                     \
				MECACELL_K(p8) = o[MECACELL_SK(p8)].get<decltype(MECACELL_K(p8))>();            \
			if (o.count(MECACELL_SK(p9)))                                                     \
				MECACELL_K(p9) = o[MECACELL_SK(p9)].get<decltype(MECACELL_K(p9))>();            \
			if (o.count(MECACELL_SK(p10)))                                                    \
				MECACELL_K(p10) = o[MECACELL_SK(p10)].get<decltype(MECACELL_K(p10))>();         \
			if (o.count(MECACELL_SK(p11)))                                                    \
				MECACELL_K(p11) = o[MECACELL_SK(p11)].get<decltype(MECACELL_K(p11))>();         \
			if (o.count(MECACELL_SK(p12)))                                                    \
				MECACELL_K(p12) = o[MECACELL_SK(p12)].get<decltype(MECACELL_K(p12))>();         \
			if (o.count(MECACELL_SK(p13)))                                                    \
				MECACELL_K(p13) = o[MECACELL_SK(p13)].get<decltype(MECACELL_K(p13))>();         \
		}                                                                                   \
		json to_json() const {                                                              \
			auto b = BType::to_json();                                                        \
			auto r = to_jsonL(*this);                                                         \
			if (!b.empty()) r.update(b);                                                      \
			return r;                                                                         \
		}                                                                                   \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {         \
			return s.saveJ(                                                                   \
			    MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1), s.MECACELL_K(p1),         \
			    MECACELL_SK(p2), s.MECACELL_K(p2), MECACELL_SK(p3), s.MECACELL_K(p3),         \
			    MECACELL_SK(p4), s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),         \
			    MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7), s.MECACELL_K(p7),         \
			    MECACELL_SK(p8), s.MECACELL_K(p8), MECACELL_SK(p9), s.MECACELL_K(p9),         \
			    MECACELL_SK(p10), s.MECACELL_K(p10), MECACELL_SK(p11), s.MECACELL_K(p11),     \
			    MECACELL_SK(p12), s.MECACELL_K(p12), MECACELL_SK(p13), s.MECACELL_K(p13));    \
		};                                                                                  \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); };     \
	};                                                                                    \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE15(S, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13)    \
	EXPORTABLE_INHERIT16(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, \
	                     v11, v12, v13)

#define EXPORTABLE_INHERIT17(S, Base, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, \
                             p12, p13, p14)                                             \
	struct Exportable : ExportableType<Base>::type {                                      \
		using BType = ExportableType<Base>::type;                                           \
		using json = nlohmann::json;                                                        \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);          \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);          \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);          \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);          \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);          \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);          \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);          \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);          \
		typename ExportableType<decltype(S::MECACELL_V(p8))>::type MECACELL_K(p8);          \
		typename ExportableType<decltype(S::MECACELL_V(p9))>::type MECACELL_K(p9);          \
		typename ExportableType<decltype(S::MECACELL_V(p10))>::type MECACELL_K(p10);        \
		typename ExportableType<decltype(S::MECACELL_V(p11))>::type MECACELL_K(p11);        \
		typename ExportableType<decltype(S::MECACELL_V(p12))>::type MECACELL_K(p12);        \
		typename ExportableType<decltype(S::MECACELL_V(p13))>::type MECACELL_K(p13);        \
		typename ExportableType<decltype(S::MECACELL_V(p14))>::type MECACELL_K(p14);        \
		Exportable() {}                                                                     \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }               \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                      \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                         \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                         \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                         \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                         \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                         \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                         \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                         \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                         \
			ExportableConstruct(MECACELL_K(p8), s.MECACELL_V(p8), s);                         \
			ExportableConstruct(MECACELL_K(p9), s.MECACELL_V(p9), s);                         \
			ExportableConstruct(MECACELL_K(p10), s.MECACELL_V(p10), s);                       \
			ExportableConstruct(MECACELL_K(p11), s.MECACELL_V(p11), s);                       \
			ExportableConstruct(MECACELL_K(p12), s.MECACELL_V(p12), s);                       \
			ExportableConstruct(MECACELL_K(p13), s.MECACELL_V(p13), s);                       \
			ExportableConstruct(MECACELL_K(p14), s.MECACELL_V(p14), s);                       \
		}                                                                                   \
		void from_json(const json& o) {                                                     \
			if (o.count(MECACELL_SK(p0)))                                                     \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();            \
			if (o.count(MECACELL_SK(p1)))                                                     \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();            \
			if (o.count(MECACELL_SK(p2)))                                                     \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();            \
			if (o.count(MECACELL_SK(p3)))                                                     \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();            \
			if (o.count(MECACELL_SK(p4)))                                                     \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();            \
			if (o.count(MECACELL_SK(p5)))                                                     \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();            \
			if (o.count(MECACELL_SK(p6)))                                                     \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();            \
			if (o.count(MECACELL_SK(p7)))                                                     \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();            \
			if (o.count(MECACELL_SK(p8)))                                                     \
				MECACELL_K(p8) = o[MECACELL_SK(p8)].get<decltype(MECACELL_K(p8))>();            \
			if (o.count(MECACELL_SK(p9)))                                                     \
				MECACELL_K(p9) = o[MECACELL_SK(p9)].get<decltype(MECACELL_K(p9))>();            \
			if (o.count(MECACELL_SK(p10)))                                                    \
				MECACELL_K(p10) = o[MECACELL_SK(p10)].get<decltype(MECACELL_K(p10))>();         \
			if (o.count(MECACELL_SK(p11)))                                                    \
				MECACELL_K(p11) = o[MECACELL_SK(p11)].get<decltype(MECACELL_K(p11))>();         \
			if (o.count(MECACELL_SK(p12)))                                                    \
				MECACELL_K(p12) = o[MECACELL_SK(p12)].get<decltype(MECACELL_K(p12))>();         \
			if (o.count(MECACELL_SK(p13)))                                                    \
				MECACELL_K(p13) = o[MECACELL_SK(p13)].get<decltype(MECACELL_K(p13))>();         \
			if (o.count(MECACELL_SK(p14)))                                                    \
				MECACELL_K(p14) = o[MECACELL_SK(p14)].get<decltype(MECACELL_K(p14))>();         \
		}                                                                                   \
		json to_json() const {                                                              \
			auto b = BType::to_json();                                                        \
			auto r = to_jsonL(*this);                                                         \
			if (!b.empty()) r.update(b);                                                      \
			return r;                                                                         \
		}                                                                                   \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {         \
			return s.saveJ(                                                                   \
			    MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1), s.MECACELL_K(p1),         \
			    MECACELL_SK(p2), s.MECACELL_K(p2), MECACELL_SK(p3), s.MECACELL_K(p3),         \
			    MECACELL_SK(p4), s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),         \
			    MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7), s.MECACELL_K(p7),         \
			    MECACELL_SK(p8), s.MECACELL_K(p8), MECACELL_SK(p9), s.MECACELL_K(p9),         \
			    MECACELL_SK(p10), s.MECACELL_K(p10), MECACELL_SK(p11), s.MECACELL_K(p11),     \
			    MECACELL_SK(p12), s.MECACELL_K(p12), MECACELL_SK(p13), s.MECACELL_K(p13),     \
			    MECACELL_SK(p14), s.MECACELL_K(p14));                                         \
		};                                                                                  \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); };     \
	};                                                                                    \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE16(S, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14) \
	EXPORTABLE_INHERIT17(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,   \
	                     v11, v12, v13, v14)

#define EXPORTABLE_INHERIT18(S, Base, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, \
                             p12, p13, p14, p15)                                        \
	struct Exportable : ExportableType<Base>::type {                                      \
		using BType = ExportableType<Base>::type;                                           \
		using json = nlohmann::json;                                                        \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);          \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);          \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);          \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);          \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);          \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);          \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);          \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);          \
		typename ExportableType<decltype(S::MECACELL_V(p8))>::type MECACELL_K(p8);          \
		typename ExportableType<decltype(S::MECACELL_V(p9))>::type MECACELL_K(p9);          \
		typename ExportableType<decltype(S::MECACELL_V(p10))>::type MECACELL_K(p10);        \
		typename ExportableType<decltype(S::MECACELL_V(p11))>::type MECACELL_K(p11);        \
		typename ExportableType<decltype(S::MECACELL_V(p12))>::type MECACELL_K(p12);        \
		typename ExportableType<decltype(S::MECACELL_V(p13))>::type MECACELL_K(p13);        \
		typename ExportableType<decltype(S::MECACELL_V(p14))>::type MECACELL_K(p14);        \
		typename ExportableType<decltype(S::MECACELL_V(p15))>::type MECACELL_K(p15);        \
		Exportable() {}                                                                     \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }               \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                      \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                         \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                         \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                         \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                         \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                         \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                         \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                         \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                         \
			ExportableConstruct(MECACELL_K(p8), s.MECACELL_V(p8), s);                         \
			ExportableConstruct(MECACELL_K(p9), s.MECACELL_V(p9), s);                         \
			ExportableConstruct(MECACELL_K(p10), s.MECACELL_V(p10), s);                       \
			ExportableConstruct(MECACELL_K(p11), s.MECACELL_V(p11), s);                       \
			ExportableConstruct(MECACELL_K(p12), s.MECACELL_V(p12), s);                       \
			ExportableConstruct(MECACELL_K(p13), s.MECACELL_V(p13), s);                       \
			ExportableConstruct(MECACELL_K(p14), s.MECACELL_V(p14), s);                       \
			ExportableConstruct(MECACELL_K(p15), s.MECACELL_V(p15), s);                       \
		}                                                                                   \
		void from_json(const json& o) {                                                     \
			if (o.count(MECACELL_SK(p0)))                                                     \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();            \
			if (o.count(MECACELL_SK(p1)))                                                     \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();            \
			if (o.count(MECACELL_SK(p2)))                                                     \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();            \
			if (o.count(MECACELL_SK(p3)))                                                     \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();            \
			if (o.count(MECACELL_SK(p4)))                                                     \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();            \
			if (o.count(MECACELL_SK(p5)))                                                     \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();            \
			if (o.count(MECACELL_SK(p6)))                                                     \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();            \
			if (o.count(MECACELL_SK(p7)))                                                     \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();            \
			if (o.count(MECACELL_SK(p8)))                                                     \
				MECACELL_K(p8) = o[MECACELL_SK(p8)].get<decltype(MECACELL_K(p8))>();            \
			if (o.count(MECACELL_SK(p9)))                                                     \
				MECACELL_K(p9) = o[MECACELL_SK(p9)].get<decltype(MECACELL_K(p9))>();            \
			if (o.count(MECACELL_SK(p10)))                                                    \
				MECACELL_K(p10) = o[MECACELL_SK(p10)].get<decltype(MECACELL_K(p10))>();         \
			if (o.count(MECACELL_SK(p11)))                                                    \
				MECACELL_K(p11) = o[MECACELL_SK(p11)].get<decltype(MECACELL_K(p11))>();         \
			if (o.count(MECACELL_SK(p12)))                                                    \
				MECACELL_K(p12) = o[MECACELL_SK(p12)].get<decltype(MECACELL_K(p12))>();         \
			if (o.count(MECACELL_SK(p13)))                                                    \
				MECACELL_K(p13) = o[MECACELL_SK(p13)].get<decltype(MECACELL_K(p13))>();         \
			if (o.count(MECACELL_SK(p14)))                                                    \
				MECACELL_K(p14) = o[MECACELL_SK(p14)].get<decltype(MECACELL_K(p14))>();         \
			if (o.count(MECACELL_SK(p15)))                                                    \
				MECACELL_K(p15) = o[MECACELL_SK(p15)].get<decltype(MECACELL_K(p15))>();         \
		}                                                                                   \
		json to_json() const {                                                              \
			auto b = BType::to_json();                                                        \
			auto r = to_jsonL(*this);                                                         \
			if (!b.empty()) r.update(b);                                                      \
			return r;                                                                         \
		}                                                                                   \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {         \
			return s.saveJ(                                                                   \
			    MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1), s.MECACELL_K(p1),         \
			    MECACELL_SK(p2), s.MECACELL_K(p2), MECACELL_SK(p3), s.MECACELL_K(p3),         \
			    MECACELL_SK(p4), s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),         \
			    MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7), s.MECACELL_K(p7),         \
			    MECACELL_SK(p8), s.MECACELL_K(p8), MECACELL_SK(p9), s.MECACELL_K(p9),         \
			    MECACELL_SK(p10), s.MECACELL_K(p10), MECACELL_SK(p11), s.MECACELL_K(p11),     \
			    MECACELL_SK(p12), s.MECACELL_K(p12), MECACELL_SK(p13), s.MECACELL_K(p13),     \
			    MECACELL_SK(p14), s.MECACELL_K(p14), MECACELL_SK(p15), s.MECACELL_K(p15));    \
		};                                                                                  \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); };     \
	};                                                                                    \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE17(S, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, \
                     v15)                                                                \
	EXPORTABLE_INHERIT18(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,   \
	                     v11, v12, v13, v14, v15)

#define EXPORTABLE_INHERIT19(S, Base, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, \
                             p12, p13, p14, p15, p16)                                   \
	struct Exportable : ExportableType<Base>::type {                                      \
		using BType = ExportableType<Base>::type;                                           \
		using json = nlohmann::json;                                                        \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);          \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);          \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);          \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);          \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);          \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);          \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);          \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);          \
		typename ExportableType<decltype(S::MECACELL_V(p8))>::type MECACELL_K(p8);          \
		typename ExportableType<decltype(S::MECACELL_V(p9))>::type MECACELL_K(p9);          \
		typename ExportableType<decltype(S::MECACELL_V(p10))>::type MECACELL_K(p10);        \
		typename ExportableType<decltype(S::MECACELL_V(p11))>::type MECACELL_K(p11);        \
		typename ExportableType<decltype(S::MECACELL_V(p12))>::type MECACELL_K(p12);        \
		typename ExportableType<decltype(S::MECACELL_V(p13))>::type MECACELL_K(p13);        \
		typename ExportableType<decltype(S::MECACELL_V(p14))>::type MECACELL_K(p14);        \
		typename ExportableType<decltype(S::MECACELL_V(p15))>::type MECACELL_K(p15);        \
		typename ExportableType<decltype(S::MECACELL_V(p16))>::type MECACELL_K(p16);        \
		Exportable() {}                                                                     \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }               \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                      \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                         \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                         \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                         \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                         \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                         \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                         \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                         \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                         \
			ExportableConstruct(MECACELL_K(p8), s.MECACELL_V(p8), s);                         \
			ExportableConstruct(MECACELL_K(p9), s.MECACELL_V(p9), s);                         \
			ExportableConstruct(MECACELL_K(p10), s.MECACELL_V(p10), s);                       \
			ExportableConstruct(MECACELL_K(p11), s.MECACELL_V(p11), s);                       \
			ExportableConstruct(MECACELL_K(p12), s.MECACELL_V(p12), s);                       \
			ExportableConstruct(MECACELL_K(p13), s.MECACELL_V(p13), s);                       \
			ExportableConstruct(MECACELL_K(p14), s.MECACELL_V(p14), s);                       \
			ExportableConstruct(MECACELL_K(p15), s.MECACELL_V(p15), s);                       \
			ExportableConstruct(MECACELL_K(p16), s.MECACELL_V(p16), s);                       \
		}                                                                                   \
		void from_json(const json& o) {                                                     \
			if (o.count(MECACELL_SK(p0)))                                                     \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();            \
			if (o.count(MECACELL_SK(p1)))                                                     \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();            \
			if (o.count(MECACELL_SK(p2)))                                                     \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();            \
			if (o.count(MECACELL_SK(p3)))                                                     \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();            \
			if (o.count(MECACELL_SK(p4)))                                                     \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();            \
			if (o.count(MECACELL_SK(p5)))                                                     \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();            \
			if (o.count(MECACELL_SK(p6)))                                                     \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();            \
			if (o.count(MECACELL_SK(p7)))                                                     \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();            \
			if (o.count(MECACELL_SK(p8)))                                                     \
				MECACELL_K(p8) = o[MECACELL_SK(p8)].get<decltype(MECACELL_K(p8))>();            \
			if (o.count(MECACELL_SK(p9)))                                                     \
				MECACELL_K(p9) = o[MECACELL_SK(p9)].get<decltype(MECACELL_K(p9))>();            \
			if (o.count(MECACELL_SK(p10)))                                                    \
				MECACELL_K(p10) = o[MECACELL_SK(p10)].get<decltype(MECACELL_K(p10))>();         \
			if (o.count(MECACELL_SK(p11)))                                                    \
				MECACELL_K(p11) = o[MECACELL_SK(p11)].get<decltype(MECACELL_K(p11))>();         \
			if (o.count(MECACELL_SK(p12)))                                                    \
				MECACELL_K(p12) = o[MECACELL_SK(p12)].get<decltype(MECACELL_K(p12))>();         \
			if (o.count(MECACELL_SK(p13)))                                                    \
				MECACELL_K(p13) = o[MECACELL_SK(p13)].get<decltype(MECACELL_K(p13))>();         \
			if (o.count(MECACELL_SK(p14)))                                                    \
				MECACELL_K(p14) = o[MECACELL_SK(p14)].get<decltype(MECACELL_K(p14))>();         \
			if (o.count(MECACELL_SK(p15)))                                                    \
				MECACELL_K(p15) = o[MECACELL_SK(p15)].get<decltype(MECACELL_K(p15))>();         \
			if (o.count(MECACELL_SK(p16)))                                                    \
				MECACELL_K(p16) = o[MECACELL_SK(p16)].get<decltype(MECACELL_K(p16))>();         \
		}                                                                                   \
		json to_json() const {                                                              \
			auto b = BType::to_json();                                                        \
			auto r = to_jsonL(*this);                                                         \
			if (!b.empty()) r.update(b);                                                      \
			return r;                                                                         \
		}                                                                                   \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {         \
			return s.saveJ(                                                                   \
			    MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1), s.MECACELL_K(p1),         \
			    MECACELL_SK(p2), s.MECACELL_K(p2), MECACELL_SK(p3), s.MECACELL_K(p3),         \
			    MECACELL_SK(p4), s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),         \
			    MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7), s.MECACELL_K(p7),         \
			    MECACELL_SK(p8), s.MECACELL_K(p8), MECACELL_SK(p9), s.MECACELL_K(p9),         \
			    MECACELL_SK(p10), s.MECACELL_K(p10), MECACELL_SK(p11), s.MECACELL_K(p11),     \
			    MECACELL_SK(p12), s.MECACELL_K(p12), MECACELL_SK(p13), s.MECACELL_K(p13),     \
			    MECACELL_SK(p14), s.MECACELL_K(p14), MECACELL_SK(p15), s.MECACELL_K(p15),     \
			    MECACELL_SK(p16), s.MECACELL_K(p16));                                         \
		};                                                                                  \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); };     \
	};                                                                                    \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE18(S, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, \
                     v15, v16)                                                           \
	EXPORTABLE_INHERIT19(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,   \
	                     v11, v12, v13, v14, v15, v16)

#define EXPORTABLE_INHERIT20(S, Base, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, \
                             p12, p13, p14, p15, p16, p17)                              \
	struct Exportable : ExportableType<Base>::type {                                      \
		using BType = ExportableType<Base>::type;                                           \
		using json = nlohmann::json;                                                        \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);          \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);          \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);          \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);          \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);          \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);          \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);          \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);          \
		typename ExportableType<decltype(S::MECACELL_V(p8))>::type MECACELL_K(p8);          \
		typename ExportableType<decltype(S::MECACELL_V(p9))>::type MECACELL_K(p9);          \
		typename ExportableType<decltype(S::MECACELL_V(p10))>::type MECACELL_K(p10);        \
		typename ExportableType<decltype(S::MECACELL_V(p11))>::type MECACELL_K(p11);        \
		typename ExportableType<decltype(S::MECACELL_V(p12))>::type MECACELL_K(p12);        \
		typename ExportableType<decltype(S::MECACELL_V(p13))>::type MECACELL_K(p13);        \
		typename ExportableType<decltype(S::MECACELL_V(p14))>::type MECACELL_K(p14);        \
		typename ExportableType<decltype(S::MECACELL_V(p15))>::type MECACELL_K(p15);        \
		typename ExportableType<decltype(S::MECACELL_V(p16))>::type MECACELL_K(p16);        \
		typename ExportableType<decltype(S::MECACELL_V(p17))>::type MECACELL_K(p17);        \
		Exportable() {}                                                                     \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }               \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                      \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                         \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                         \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                         \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                         \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                         \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                         \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                         \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                         \
			ExportableConstruct(MECACELL_K(p8), s.MECACELL_V(p8), s);                         \
			ExportableConstruct(MECACELL_K(p9), s.MECACELL_V(p9), s);                         \
			ExportableConstruct(MECACELL_K(p10), s.MECACELL_V(p10), s);                       \
			ExportableConstruct(MECACELL_K(p11), s.MECACELL_V(p11), s);                       \
			ExportableConstruct(MECACELL_K(p12), s.MECACELL_V(p12), s);                       \
			ExportableConstruct(MECACELL_K(p13), s.MECACELL_V(p13), s);                       \
			ExportableConstruct(MECACELL_K(p14), s.MECACELL_V(p14), s);                       \
			ExportableConstruct(MECACELL_K(p15), s.MECACELL_V(p15), s);                       \
			ExportableConstruct(MECACELL_K(p16), s.MECACELL_V(p16), s);                       \
			ExportableConstruct(MECACELL_K(p17), s.MECACELL_V(p17), s);                       \
		}                                                                                   \
		void from_json(const json& o) {                                                     \
			if (o.count(MECACELL_SK(p0)))                                                     \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();            \
			if (o.count(MECACELL_SK(p1)))                                                     \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();            \
			if (o.count(MECACELL_SK(p2)))                                                     \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();            \
			if (o.count(MECACELL_SK(p3)))                                                     \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();            \
			if (o.count(MECACELL_SK(p4)))                                                     \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();            \
			if (o.count(MECACELL_SK(p5)))                                                     \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();            \
			if (o.count(MECACELL_SK(p6)))                                                     \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();            \
			if (o.count(MECACELL_SK(p7)))                                                     \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();            \
			if (o.count(MECACELL_SK(p8)))                                                     \
				MECACELL_K(p8) = o[MECACELL_SK(p8)].get<decltype(MECACELL_K(p8))>();            \
			if (o.count(MECACELL_SK(p9)))                                                     \
				MECACELL_K(p9) = o[MECACELL_SK(p9)].get<decltype(MECACELL_K(p9))>();            \
			if (o.count(MECACELL_SK(p10)))                                                    \
				MECACELL_K(p10) = o[MECACELL_SK(p10)].get<decltype(MECACELL_K(p10))>();         \
			if (o.count(MECACELL_SK(p11)))                                                    \
				MECACELL_K(p11) = o[MECACELL_SK(p11)].get<decltype(MECACELL_K(p11))>();         \
			if (o.count(MECACELL_SK(p12)))                                                    \
				MECACELL_K(p12) = o[MECACELL_SK(p12)].get<decltype(MECACELL_K(p12))>();         \
			if (o.count(MECACELL_SK(p13)))                                                    \
				MECACELL_K(p13) = o[MECACELL_SK(p13)].get<decltype(MECACELL_K(p13))>();         \
			if (o.count(MECACELL_SK(p14)))                                                    \
				MECACELL_K(p14) = o[MECACELL_SK(p14)].get<decltype(MECACELL_K(p14))>();         \
			if (o.count(MECACELL_SK(p15)))                                                    \
				MECACELL_K(p15) = o[MECACELL_SK(p15)].get<decltype(MECACELL_K(p15))>();         \
			if (o.count(MECACELL_SK(p16)))                                                    \
				MECACELL_K(p16) = o[MECACELL_SK(p16)].get<decltype(MECACELL_K(p16))>();         \
			if (o.count(MECACELL_SK(p17)))                                                    \
				MECACELL_K(p17) = o[MECACELL_SK(p17)].get<decltype(MECACELL_K(p17))>();         \
		}                                                                                   \
		json to_json() const {                                                              \
			auto b = BType::to_json();                                                        \
			auto r = to_jsonL(*this);                                                         \
			if (!b.empty()) r.update(b);                                                      \
			return r;                                                                         \
		}                                                                                   \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {         \
			return s.saveJ(                                                                   \
			    MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1), s.MECACELL_K(p1),         \
			    MECACELL_SK(p2), s.MECACELL_K(p2), MECACELL_SK(p3), s.MECACELL_K(p3),         \
			    MECACELL_SK(p4), s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),         \
			    MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7), s.MECACELL_K(p7),         \
			    MECACELL_SK(p8), s.MECACELL_K(p8), MECACELL_SK(p9), s.MECACELL_K(p9),         \
			    MECACELL_SK(p10), s.MECACELL_K(p10), MECACELL_SK(p11), s.MECACELL_K(p11),     \
			    MECACELL_SK(p12), s.MECACELL_K(p12), MECACELL_SK(p13), s.MECACELL_K(p13),     \
			    MECACELL_SK(p14), s.MECACELL_K(p14), MECACELL_SK(p15), s.MECACELL_K(p15),     \
			    MECACELL_SK(p16), s.MECACELL_K(p16), MECACELL_SK(p17), s.MECACELL_K(p17));    \
		};                                                                                  \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); };     \
	};                                                                                    \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE19(S, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, \
                     v15, v16, v17)                                                      \
	EXPORTABLE_INHERIT20(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,   \
	                     v11, v12, v13, v14, v15, v16, v17)

#define EXPORTABLE_INHERIT21(S, Base, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, \
                             p12, p13, p14, p15, p16, p17, p18)                         \
	struct Exportable : ExportableType<Base>::type {                                      \
		using BType = ExportableType<Base>::type;                                           \
		using json = nlohmann::json;                                                        \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);          \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);          \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);          \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);          \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);          \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);          \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);          \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);          \
		typename ExportableType<decltype(S::MECACELL_V(p8))>::type MECACELL_K(p8);          \
		typename ExportableType<decltype(S::MECACELL_V(p9))>::type MECACELL_K(p9);          \
		typename ExportableType<decltype(S::MECACELL_V(p10))>::type MECACELL_K(p10);        \
		typename ExportableType<decltype(S::MECACELL_V(p11))>::type MECACELL_K(p11);        \
		typename ExportableType<decltype(S::MECACELL_V(p12))>::type MECACELL_K(p12);        \
		typename ExportableType<decltype(S::MECACELL_V(p13))>::type MECACELL_K(p13);        \
		typename ExportableType<decltype(S::MECACELL_V(p14))>::type MECACELL_K(p14);        \
		typename ExportableType<decltype(S::MECACELL_V(p15))>::type MECACELL_K(p15);        \
		typename ExportableType<decltype(S::MECACELL_V(p16))>::type MECACELL_K(p16);        \
		typename ExportableType<decltype(S::MECACELL_V(p17))>::type MECACELL_K(p17);        \
		typename ExportableType<decltype(S::MECACELL_V(p18))>::type MECACELL_K(p18);        \
		Exportable() {}                                                                     \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }               \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                      \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                         \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                         \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                         \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                         \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                         \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                         \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                         \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                         \
			ExportableConstruct(MECACELL_K(p8), s.MECACELL_V(p8), s);                         \
			ExportableConstruct(MECACELL_K(p9), s.MECACELL_V(p9), s);                         \
			ExportableConstruct(MECACELL_K(p10), s.MECACELL_V(p10), s);                       \
			ExportableConstruct(MECACELL_K(p11), s.MECACELL_V(p11), s);                       \
			ExportableConstruct(MECACELL_K(p12), s.MECACELL_V(p12), s);                       \
			ExportableConstruct(MECACELL_K(p13), s.MECACELL_V(p13), s);                       \
			ExportableConstruct(MECACELL_K(p14), s.MECACELL_V(p14), s);                       \
			ExportableConstruct(MECACELL_K(p15), s.MECACELL_V(p15), s);                       \
			ExportableConstruct(MECACELL_K(p16), s.MECACELL_V(p16), s);                       \
			ExportableConstruct(MECACELL_K(p17), s.MECACELL_V(p17), s);                       \
			ExportableConstruct(MECACELL_K(p18), s.MECACELL_V(p18), s);                       \
		}                                                                                   \
		void from_json(const json& o) {                                                     \
			if (o.count(MECACELL_SK(p0)))                                                     \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();            \
			if (o.count(MECACELL_SK(p1)))                                                     \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();            \
			if (o.count(MECACELL_SK(p2)))                                                     \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();            \
			if (o.count(MECACELL_SK(p3)))                                                     \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();            \
			if (o.count(MECACELL_SK(p4)))                                                     \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();            \
			if (o.count(MECACELL_SK(p5)))                                                     \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();            \
			if (o.count(MECACELL_SK(p6)))                                                     \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();            \
			if (o.count(MECACELL_SK(p7)))                                                     \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();            \
			if (o.count(MECACELL_SK(p8)))                                                     \
				MECACELL_K(p8) = o[MECACELL_SK(p8)].get<decltype(MECACELL_K(p8))>();            \
			if (o.count(MECACELL_SK(p9)))                                                     \
				MECACELL_K(p9) = o[MECACELL_SK(p9)].get<decltype(MECACELL_K(p9))>();            \
			if (o.count(MECACELL_SK(p10)))                                                    \
				MECACELL_K(p10) = o[MECACELL_SK(p10)].get<decltype(MECACELL_K(p10))>();         \
			if (o.count(MECACELL_SK(p11)))                                                    \
				MECACELL_K(p11) = o[MECACELL_SK(p11)].get<decltype(MECACELL_K(p11))>();         \
			if (o.count(MECACELL_SK(p12)))                                                    \
				MECACELL_K(p12) = o[MECACELL_SK(p12)].get<decltype(MECACELL_K(p12))>();         \
			if (o.count(MECACELL_SK(p13)))                                                    \
				MECACELL_K(p13) = o[MECACELL_SK(p13)].get<decltype(MECACELL_K(p13))>();         \
			if (o.count(MECACELL_SK(p14)))                                                    \
				MECACELL_K(p14) = o[MECACELL_SK(p14)].get<decltype(MECACELL_K(p14))>();         \
			if (o.count(MECACELL_SK(p15)))                                                    \
				MECACELL_K(p15) = o[MECACELL_SK(p15)].get<decltype(MECACELL_K(p15))>();         \
			if (o.count(MECACELL_SK(p16)))                                                    \
				MECACELL_K(p16) = o[MECACELL_SK(p16)].get<decltype(MECACELL_K(p16))>();         \
			if (o.count(MECACELL_SK(p17)))                                                    \
				MECACELL_K(p17) = o[MECACELL_SK(p17)].get<decltype(MECACELL_K(p17))>();         \
			if (o.count(MECACELL_SK(p18)))                                                    \
				MECACELL_K(p18) = o[MECACELL_SK(p18)].get<decltype(MECACELL_K(p18))>();         \
		}                                                                                   \
		json to_json() const {                                                              \
			auto b = BType::to_json();                                                        \
			auto r = to_jsonL(*this);                                                         \
			if (!b.empty()) r.update(b);                                                      \
			return r;                                                                         \
		}                                                                                   \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {         \
			return s.saveJ(                                                                   \
			    MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1), s.MECACELL_K(p1),         \
			    MECACELL_SK(p2), s.MECACELL_K(p2), MECACELL_SK(p3), s.MECACELL_K(p3),         \
			    MECACELL_SK(p4), s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),         \
			    MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7), s.MECACELL_K(p7),         \
			    MECACELL_SK(p8), s.MECACELL_K(p8), MECACELL_SK(p9), s.MECACELL_K(p9),         \
			    MECACELL_SK(p10), s.MECACELL_K(p10), MECACELL_SK(p11), s.MECACELL_K(p11),     \
			    MECACELL_SK(p12), s.MECACELL_K(p12), MECACELL_SK(p13), s.MECACELL_K(p13),     \
			    MECACELL_SK(p14), s.MECACELL_K(p14), MECACELL_SK(p15), s.MECACELL_K(p15),     \
			    MECACELL_SK(p16), s.MECACELL_K(p16), MECACELL_SK(p17), s.MECACELL_K(p17),     \
			    MECACELL_SK(p18), s.MECACELL_K(p18));                                         \
		};                                                                                  \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); };     \
	};                                                                                    \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE20(S, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, \
                     v15, v16, v17, v18)                                                 \
	EXPORTABLE_INHERIT21(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,   \
	                     v11, v12, v13, v14, v15, v16, v17, v18)

#define EXPORTABLE_INHERIT22(S, Base, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, \
                             p12, p13, p14, p15, p16, p17, p18, p19)                    \
	struct Exportable : ExportableType<Base>::type {                                      \
		using BType = ExportableType<Base>::type;                                           \
		using json = nlohmann::json;                                                        \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);          \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);          \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);          \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);          \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);          \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);          \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);          \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);          \
		typename ExportableType<decltype(S::MECACELL_V(p8))>::type MECACELL_K(p8);          \
		typename ExportableType<decltype(S::MECACELL_V(p9))>::type MECACELL_K(p9);          \
		typename ExportableType<decltype(S::MECACELL_V(p10))>::type MECACELL_K(p10);        \
		typename ExportableType<decltype(S::MECACELL_V(p11))>::type MECACELL_K(p11);        \
		typename ExportableType<decltype(S::MECACELL_V(p12))>::type MECACELL_K(p12);        \
		typename ExportableType<decltype(S::MECACELL_V(p13))>::type MECACELL_K(p13);        \
		typename ExportableType<decltype(S::MECACELL_V(p14))>::type MECACELL_K(p14);        \
		typename ExportableType<decltype(S::MECACELL_V(p15))>::type MECACELL_K(p15);        \
		typename ExportableType<decltype(S::MECACELL_V(p16))>::type MECACELL_K(p16);        \
		typename ExportableType<decltype(S::MECACELL_V(p17))>::type MECACELL_K(p17);        \
		typename ExportableType<decltype(S::MECACELL_V(p18))>::type MECACELL_K(p18);        \
		typename ExportableType<decltype(S::MECACELL_V(p19))>::type MECACELL_K(p19);        \
		Exportable() {}                                                                     \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }               \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                      \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                         \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                         \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                         \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                         \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                         \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                         \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                         \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                         \
			ExportableConstruct(MECACELL_K(p8), s.MECACELL_V(p8), s);                         \
			ExportableConstruct(MECACELL_K(p9), s.MECACELL_V(p9), s);                         \
			ExportableConstruct(MECACELL_K(p10), s.MECACELL_V(p10), s);                       \
			ExportableConstruct(MECACELL_K(p11), s.MECACELL_V(p11), s);                       \
			ExportableConstruct(MECACELL_K(p12), s.MECACELL_V(p12), s);                       \
			ExportableConstruct(MECACELL_K(p13), s.MECACELL_V(p13), s);                       \
			ExportableConstruct(MECACELL_K(p14), s.MECACELL_V(p14), s);                       \
			ExportableConstruct(MECACELL_K(p15), s.MECACELL_V(p15), s);                       \
			ExportableConstruct(MECACELL_K(p16), s.MECACELL_V(p16), s);                       \
			ExportableConstruct(MECACELL_K(p17), s.MECACELL_V(p17), s);                       \
			ExportableConstruct(MECACELL_K(p18), s.MECACELL_V(p18), s);                       \
			ExportableConstruct(MECACELL_K(p19), s.MECACELL_V(p19), s);                       \
		}                                                                                   \
		void from_json(const json& o) {                                                     \
			if (o.count(MECACELL_SK(p0)))                                                     \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();            \
			if (o.count(MECACELL_SK(p1)))                                                     \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();            \
			if (o.count(MECACELL_SK(p2)))                                                     \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();            \
			if (o.count(MECACELL_SK(p3)))                                                     \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();            \
			if (o.count(MECACELL_SK(p4)))                                                     \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();            \
			if (o.count(MECACELL_SK(p5)))                                                     \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();            \
			if (o.count(MECACELL_SK(p6)))                                                     \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();            \
			if (o.count(MECACELL_SK(p7)))                                                     \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();            \
			if (o.count(MECACELL_SK(p8)))                                                     \
				MECACELL_K(p8) = o[MECACELL_SK(p8)].get<decltype(MECACELL_K(p8))>();            \
			if (o.count(MECACELL_SK(p9)))                                                     \
				MECACELL_K(p9) = o[MECACELL_SK(p9)].get<decltype(MECACELL_K(p9))>();            \
			if (o.count(MECACELL_SK(p10)))                                                    \
				MECACELL_K(p10) = o[MECACELL_SK(p10)].get<decltype(MECACELL_K(p10))>();         \
			if (o.count(MECACELL_SK(p11)))                                                    \
				MECACELL_K(p11) = o[MECACELL_SK(p11)].get<decltype(MECACELL_K(p11))>();         \
			if (o.count(MECACELL_SK(p12)))                                                    \
				MECACELL_K(p12) = o[MECACELL_SK(p12)].get<decltype(MECACELL_K(p12))>();         \
			if (o.count(MECACELL_SK(p13)))                                                    \
				MECACELL_K(p13) = o[MECACELL_SK(p13)].get<decltype(MECACELL_K(p13))>();         \
			if (o.count(MECACELL_SK(p14)))                                                    \
				MECACELL_K(p14) = o[MECACELL_SK(p14)].get<decltype(MECACELL_K(p14))>();         \
			if (o.count(MECACELL_SK(p15)))                                                    \
				MECACELL_K(p15) = o[MECACELL_SK(p15)].get<decltype(MECACELL_K(p15))>();         \
			if (o.count(MECACELL_SK(p16)))                                                    \
				MECACELL_K(p16) = o[MECACELL_SK(p16)].get<decltype(MECACELL_K(p16))>();         \
			if (o.count(MECACELL_SK(p17)))                                                    \
				MECACELL_K(p17) = o[MECACELL_SK(p17)].get<decltype(MECACELL_K(p17))>();         \
			if (o.count(MECACELL_SK(p18)))                                                    \
				MECACELL_K(p18) = o[MECACELL_SK(p18)].get<decltype(MECACELL_K(p18))>();         \
			if (o.count(MECACELL_SK(p19)))                                                    \
				MECACELL_K(p19) = o[MECACELL_SK(p19)].get<decltype(MECACELL_K(p19))>();         \
		}                                                                                   \
		json to_json() const {                                                              \
			auto b = BType::to_json();                                                        \
			auto r = to_jsonL(*this);                                                         \
			if (!b.empty()) r.update(b);                                                      \
			return r;                                                                         \
		}                                                                                   \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {         \
			return s.saveJ(                                                                   \
			    MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1), s.MECACELL_K(p1),         \
			    MECACELL_SK(p2), s.MECACELL_K(p2), MECACELL_SK(p3), s.MECACELL_K(p3),         \
			    MECACELL_SK(p4), s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),         \
			    MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7), s.MECACELL_K(p7),         \
			    MECACELL_SK(p8), s.MECACELL_K(p8), MECACELL_SK(p9), s.MECACELL_K(p9),         \
			    MECACELL_SK(p10), s.MECACELL_K(p10), MECACELL_SK(p11), s.MECACELL_K(p11),     \
			    MECACELL_SK(p12), s.MECACELL_K(p12), MECACELL_SK(p13), s.MECACELL_K(p13),     \
			    MECACELL_SK(p14), s.MECACELL_K(p14), MECACELL_SK(p15), s.MECACELL_K(p15),     \
			    MECACELL_SK(p16), s.MECACELL_K(p16), MECACELL_SK(p17), s.MECACELL_K(p17),     \
			    MECACELL_SK(p18), s.MECACELL_K(p18), MECACELL_SK(p19), s.MECACELL_K(p19));    \
		};                                                                                  \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); };     \
	};                                                                                    \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE21(S, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, \
                     v15, v16, v17, v18, v19)                                            \
	EXPORTABLE_INHERIT22(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,   \
	                     v11, v12, v13, v14, v15, v16, v17, v18, v19)

#define EXPORTABLE_INHERIT23(S, Base, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, \
                             p12, p13, p14, p15, p16, p17, p18, p19, p20)               \
	struct Exportable : ExportableType<Base>::type {                                      \
		using BType = ExportableType<Base>::type;                                           \
		using json = nlohmann::json;                                                        \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);          \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);          \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);          \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);          \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);          \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);          \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);          \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);          \
		typename ExportableType<decltype(S::MECACELL_V(p8))>::type MECACELL_K(p8);          \
		typename ExportableType<decltype(S::MECACELL_V(p9))>::type MECACELL_K(p9);          \
		typename ExportableType<decltype(S::MECACELL_V(p10))>::type MECACELL_K(p10);        \
		typename ExportableType<decltype(S::MECACELL_V(p11))>::type MECACELL_K(p11);        \
		typename ExportableType<decltype(S::MECACELL_V(p12))>::type MECACELL_K(p12);        \
		typename ExportableType<decltype(S::MECACELL_V(p13))>::type MECACELL_K(p13);        \
		typename ExportableType<decltype(S::MECACELL_V(p14))>::type MECACELL_K(p14);        \
		typename ExportableType<decltype(S::MECACELL_V(p15))>::type MECACELL_K(p15);        \
		typename ExportableType<decltype(S::MECACELL_V(p16))>::type MECACELL_K(p16);        \
		typename ExportableType<decltype(S::MECACELL_V(p17))>::type MECACELL_K(p17);        \
		typename ExportableType<decltype(S::MECACELL_V(p18))>::type MECACELL_K(p18);        \
		typename ExportableType<decltype(S::MECACELL_V(p19))>::type MECACELL_K(p19);        \
		typename ExportableType<decltype(S::MECACELL_V(p20))>::type MECACELL_K(p20);        \
		Exportable() {}                                                                     \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }               \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                      \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                         \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                         \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                         \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                         \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                         \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                         \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                         \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                         \
			ExportableConstruct(MECACELL_K(p8), s.MECACELL_V(p8), s);                         \
			ExportableConstruct(MECACELL_K(p9), s.MECACELL_V(p9), s);                         \
			ExportableConstruct(MECACELL_K(p10), s.MECACELL_V(p10), s);                       \
			ExportableConstruct(MECACELL_K(p11), s.MECACELL_V(p11), s);                       \
			ExportableConstruct(MECACELL_K(p12), s.MECACELL_V(p12), s);                       \
			ExportableConstruct(MECACELL_K(p13), s.MECACELL_V(p13), s);                       \
			ExportableConstruct(MECACELL_K(p14), s.MECACELL_V(p14), s);                       \
			ExportableConstruct(MECACELL_K(p15), s.MECACELL_V(p15), s);                       \
			ExportableConstruct(MECACELL_K(p16), s.MECACELL_V(p16), s);                       \
			ExportableConstruct(MECACELL_K(p17), s.MECACELL_V(p17), s);                       \
			ExportableConstruct(MECACELL_K(p18), s.MECACELL_V(p18), s);                       \
			ExportableConstruct(MECACELL_K(p19), s.MECACELL_V(p19), s);                       \
			ExportableConstruct(MECACELL_K(p20), s.MECACELL_V(p20), s);                       \
		}                                                                                   \
		void from_json(const json& o) {                                                     \
			if (o.count(MECACELL_SK(p0)))                                                     \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();            \
			if (o.count(MECACELL_SK(p1)))                                                     \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();            \
			if (o.count(MECACELL_SK(p2)))                                                     \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();            \
			if (o.count(MECACELL_SK(p3)))                                                     \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();            \
			if (o.count(MECACELL_SK(p4)))                                                     \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();            \
			if (o.count(MECACELL_SK(p5)))                                                     \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();            \
			if (o.count(MECACELL_SK(p6)))                                                     \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();            \
			if (o.count(MECACELL_SK(p7)))                                                     \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();            \
			if (o.count(MECACELL_SK(p8)))                                                     \
				MECACELL_K(p8) = o[MECACELL_SK(p8)].get<decltype(MECACELL_K(p8))>();            \
			if (o.count(MECACELL_SK(p9)))                                                     \
				MECACELL_K(p9) = o[MECACELL_SK(p9)].get<decltype(MECACELL_K(p9))>();            \
			if (o.count(MECACELL_SK(p10)))                                                    \
				MECACELL_K(p10) = o[MECACELL_SK(p10)].get<decltype(MECACELL_K(p10))>();         \
			if (o.count(MECACELL_SK(p11)))                                                    \
				MECACELL_K(p11) = o[MECACELL_SK(p11)].get<decltype(MECACELL_K(p11))>();         \
			if (o.count(MECACELL_SK(p12)))                                                    \
				MECACELL_K(p12) = o[MECACELL_SK(p12)].get<decltype(MECACELL_K(p12))>();         \
			if (o.count(MECACELL_SK(p13)))                                                    \
				MECACELL_K(p13) = o[MECACELL_SK(p13)].get<decltype(MECACELL_K(p13))>();         \
			if (o.count(MECACELL_SK(p14)))                                                    \
				MECACELL_K(p14) = o[MECACELL_SK(p14)].get<decltype(MECACELL_K(p14))>();         \
			if (o.count(MECACELL_SK(p15)))                                                    \
				MECACELL_K(p15) = o[MECACELL_SK(p15)].get<decltype(MECACELL_K(p15))>();         \
			if (o.count(MECACELL_SK(p16)))                                                    \
				MECACELL_K(p16) = o[MECACELL_SK(p16)].get<decltype(MECACELL_K(p16))>();         \
			if (o.count(MECACELL_SK(p17)))                                                    \
				MECACELL_K(p17) = o[MECACELL_SK(p17)].get<decltype(MECACELL_K(p17))>();         \
			if (o.count(MECACELL_SK(p18)))                                                    \
				MECACELL_K(p18) = o[MECACELL_SK(p18)].get<decltype(MECACELL_K(p18))>();         \
			if (o.count(MECACELL_SK(p19)))                                                    \
				MECACELL_K(p19) = o[MECACELL_SK(p19)].get<decltype(MECACELL_K(p19))>();         \
			if (o.count(MECACELL_SK(p20)))                                                    \
				MECACELL_K(p20) = o[MECACELL_SK(p20)].get<decltype(MECACELL_K(p20))>();         \
		}                                                                                   \
		json to_json() const {                                                              \
			auto b = BType::to_json();                                                        \
			auto r = to_jsonL(*this);                                                         \
			if (!b.empty()) r.update(b);                                                      \
			return r;                                                                         \
		}                                                                                   \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {         \
			return s.saveJ(                                                                   \
			    MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1), s.MECACELL_K(p1),         \
			    MECACELL_SK(p2), s.MECACELL_K(p2), MECACELL_SK(p3), s.MECACELL_K(p3),         \
			    MECACELL_SK(p4), s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),         \
			    MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7), s.MECACELL_K(p7),         \
			    MECACELL_SK(p8), s.MECACELL_K(p8), MECACELL_SK(p9), s.MECACELL_K(p9),         \
			    MECACELL_SK(p10), s.MECACELL_K(p10), MECACELL_SK(p11), s.MECACELL_K(p11),     \
			    MECACELL_SK(p12), s.MECACELL_K(p12), MECACELL_SK(p13), s.MECACELL_K(p13),     \
			    MECACELL_SK(p14), s.MECACELL_K(p14), MECACELL_SK(p15), s.MECACELL_K(p15),     \
			    MECACELL_SK(p16), s.MECACELL_K(p16), MECACELL_SK(p17), s.MECACELL_K(p17),     \
			    MECACELL_SK(p18), s.MECACELL_K(p18), MECACELL_SK(p19), s.MECACELL_K(p19),     \
			    MECACELL_SK(p20), s.MECACELL_K(p20));                                         \
		};                                                                                  \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); };     \
	};                                                                                    \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE22(S, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, \
                     v15, v16, v17, v18, v19, v20)                                       \
	EXPORTABLE_INHERIT23(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,   \
	                     v11, v12, v13, v14, v15, v16, v17, v18, v19, v20)

#define EXPORTABLE_INHERIT24(S, Base, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, \
                             p12, p13, p14, p15, p16, p17, p18, p19, p20, p21)          \
	struct Exportable : ExportableType<Base>::type {                                      \
		using BType = ExportableType<Base>::type;                                           \
		using json = nlohmann::json;                                                        \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);          \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);          \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);          \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);          \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);          \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);          \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);          \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);          \
		typename ExportableType<decltype(S::MECACELL_V(p8))>::type MECACELL_K(p8);          \
		typename ExportableType<decltype(S::MECACELL_V(p9))>::type MECACELL_K(p9);          \
		typename ExportableType<decltype(S::MECACELL_V(p10))>::type MECACELL_K(p10);        \
		typename ExportableType<decltype(S::MECACELL_V(p11))>::type MECACELL_K(p11);        \
		typename ExportableType<decltype(S::MECACELL_V(p12))>::type MECACELL_K(p12);        \
		typename ExportableType<decltype(S::MECACELL_V(p13))>::type MECACELL_K(p13);        \
		typename ExportableType<decltype(S::MECACELL_V(p14))>::type MECACELL_K(p14);        \
		typename ExportableType<decltype(S::MECACELL_V(p15))>::type MECACELL_K(p15);        \
		typename ExportableType<decltype(S::MECACELL_V(p16))>::type MECACELL_K(p16);        \
		typename ExportableType<decltype(S::MECACELL_V(p17))>::type MECACELL_K(p17);        \
		typename ExportableType<decltype(S::MECACELL_V(p18))>::type MECACELL_K(p18);        \
		typename ExportableType<decltype(S::MECACELL_V(p19))>::type MECACELL_K(p19);        \
		typename ExportableType<decltype(S::MECACELL_V(p20))>::type MECACELL_K(p20);        \
		typename ExportableType<decltype(S::MECACELL_V(p21))>::type MECACELL_K(p21);        \
		Exportable() {}                                                                     \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }               \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                      \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                         \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                         \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                         \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                         \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                         \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                         \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                         \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                         \
			ExportableConstruct(MECACELL_K(p8), s.MECACELL_V(p8), s);                         \
			ExportableConstruct(MECACELL_K(p9), s.MECACELL_V(p9), s);                         \
			ExportableConstruct(MECACELL_K(p10), s.MECACELL_V(p10), s);                       \
			ExportableConstruct(MECACELL_K(p11), s.MECACELL_V(p11), s);                       \
			ExportableConstruct(MECACELL_K(p12), s.MECACELL_V(p12), s);                       \
			ExportableConstruct(MECACELL_K(p13), s.MECACELL_V(p13), s);                       \
			ExportableConstruct(MECACELL_K(p14), s.MECACELL_V(p14), s);                       \
			ExportableConstruct(MECACELL_K(p15), s.MECACELL_V(p15), s);                       \
			ExportableConstruct(MECACELL_K(p16), s.MECACELL_V(p16), s);                       \
			ExportableConstruct(MECACELL_K(p17), s.MECACELL_V(p17), s);                       \
			ExportableConstruct(MECACELL_K(p18), s.MECACELL_V(p18), s);                       \
			ExportableConstruct(MECACELL_K(p19), s.MECACELL_V(p19), s);                       \
			ExportableConstruct(MECACELL_K(p20), s.MECACELL_V(p20), s);                       \
			ExportableConstruct(MECACELL_K(p21), s.MECACELL_V(p21), s);                       \
		}                                                                                   \
		void from_json(const json& o) {                                                     \
			if (o.count(MECACELL_SK(p0)))                                                     \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();            \
			if (o.count(MECACELL_SK(p1)))                                                     \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();            \
			if (o.count(MECACELL_SK(p2)))                                                     \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();            \
			if (o.count(MECACELL_SK(p3)))                                                     \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();            \
			if (o.count(MECACELL_SK(p4)))                                                     \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();            \
			if (o.count(MECACELL_SK(p5)))                                                     \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();            \
			if (o.count(MECACELL_SK(p6)))                                                     \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();            \
			if (o.count(MECACELL_SK(p7)))                                                     \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();            \
			if (o.count(MECACELL_SK(p8)))                                                     \
				MECACELL_K(p8) = o[MECACELL_SK(p8)].get<decltype(MECACELL_K(p8))>();            \
			if (o.count(MECACELL_SK(p9)))                                                     \
				MECACELL_K(p9) = o[MECACELL_SK(p9)].get<decltype(MECACELL_K(p9))>();            \
			if (o.count(MECACELL_SK(p10)))                                                    \
				MECACELL_K(p10) = o[MECACELL_SK(p10)].get<decltype(MECACELL_K(p10))>();         \
			if (o.count(MECACELL_SK(p11)))                                                    \
				MECACELL_K(p11) = o[MECACELL_SK(p11)].get<decltype(MECACELL_K(p11))>();         \
			if (o.count(MECACELL_SK(p12)))                                                    \
				MECACELL_K(p12) = o[MECACELL_SK(p12)].get<decltype(MECACELL_K(p12))>();         \
			if (o.count(MECACELL_SK(p13)))                                                    \
				MECACELL_K(p13) = o[MECACELL_SK(p13)].get<decltype(MECACELL_K(p13))>();         \
			if (o.count(MECACELL_SK(p14)))                                                    \
				MECACELL_K(p14) = o[MECACELL_SK(p14)].get<decltype(MECACELL_K(p14))>();         \
			if (o.count(MECACELL_SK(p15)))                                                    \
				MECACELL_K(p15) = o[MECACELL_SK(p15)].get<decltype(MECACELL_K(p15))>();         \
			if (o.count(MECACELL_SK(p16)))                                                    \
				MECACELL_K(p16) = o[MECACELL_SK(p16)].get<decltype(MECACELL_K(p16))>();         \
			if (o.count(MECACELL_SK(p17)))                                                    \
				MECACELL_K(p17) = o[MECACELL_SK(p17)].get<decltype(MECACELL_K(p17))>();         \
			if (o.count(MECACELL_SK(p18)))                                                    \
				MECACELL_K(p18) = o[MECACELL_SK(p18)].get<decltype(MECACELL_K(p18))>();         \
			if (o.count(MECACELL_SK(p19)))                                                    \
				MECACELL_K(p19) = o[MECACELL_SK(p19)].get<decltype(MECACELL_K(p19))>();         \
			if (o.count(MECACELL_SK(p20)))                                                    \
				MECACELL_K(p20) = o[MECACELL_SK(p20)].get<decltype(MECACELL_K(p20))>();         \
			if (o.count(MECACELL_SK(p21)))                                                    \
				MECACELL_K(p21) = o[MECACELL_SK(p21)].get<decltype(MECACELL_K(p21))>();         \
		}                                                                                   \
		json to_json() const {                                                              \
			auto b = BType::to_json();                                                        \
			auto r = to_jsonL(*this);                                                         \
			if (!b.empty()) r.update(b);                                                      \
			return r;                                                                         \
		}                                                                                   \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {         \
			return s.saveJ(                                                                   \
			    MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1), s.MECACELL_K(p1),         \
			    MECACELL_SK(p2), s.MECACELL_K(p2), MECACELL_SK(p3), s.MECACELL_K(p3),         \
			    MECACELL_SK(p4), s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),         \
			    MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7), s.MECACELL_K(p7),         \
			    MECACELL_SK(p8), s.MECACELL_K(p8), MECACELL_SK(p9), s.MECACELL_K(p9),         \
			    MECACELL_SK(p10), s.MECACELL_K(p10), MECACELL_SK(p11), s.MECACELL_K(p11),     \
			    MECACELL_SK(p12), s.MECACELL_K(p12), MECACELL_SK(p13), s.MECACELL_K(p13),     \
			    MECACELL_SK(p14), s.MECACELL_K(p14), MECACELL_SK(p15), s.MECACELL_K(p15),     \
			    MECACELL_SK(p16), s.MECACELL_K(p16), MECACELL_SK(p17), s.MECACELL_K(p17),     \
			    MECACELL_SK(p18), s.MECACELL_K(p18), MECACELL_SK(p19), s.MECACELL_K(p19),     \
			    MECACELL_SK(p20), s.MECACELL_K(p20), MECACELL_SK(p21), s.MECACELL_K(p21));    \
		};                                                                                  \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); };     \
	};                                                                                    \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE23(S, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, \
                     v15, v16, v17, v18, v19, v20, v21)                                  \
	EXPORTABLE_INHERIT24(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,   \
	                     v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21)

#define EXPORTABLE_INHERIT25(S, Base, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, \
                             p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22)     \
	struct Exportable : ExportableType<Base>::type {                                      \
		using BType = ExportableType<Base>::type;                                           \
		using json = nlohmann::json;                                                        \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);          \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);          \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);          \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);          \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);          \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);          \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);          \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);          \
		typename ExportableType<decltype(S::MECACELL_V(p8))>::type MECACELL_K(p8);          \
		typename ExportableType<decltype(S::MECACELL_V(p9))>::type MECACELL_K(p9);          \
		typename ExportableType<decltype(S::MECACELL_V(p10))>::type MECACELL_K(p10);        \
		typename ExportableType<decltype(S::MECACELL_V(p11))>::type MECACELL_K(p11);        \
		typename ExportableType<decltype(S::MECACELL_V(p12))>::type MECACELL_K(p12);        \
		typename ExportableType<decltype(S::MECACELL_V(p13))>::type MECACELL_K(p13);        \
		typename ExportableType<decltype(S::MECACELL_V(p14))>::type MECACELL_K(p14);        \
		typename ExportableType<decltype(S::MECACELL_V(p15))>::type MECACELL_K(p15);        \
		typename ExportableType<decltype(S::MECACELL_V(p16))>::type MECACELL_K(p16);        \
		typename ExportableType<decltype(S::MECACELL_V(p17))>::type MECACELL_K(p17);        \
		typename ExportableType<decltype(S::MECACELL_V(p18))>::type MECACELL_K(p18);        \
		typename ExportableType<decltype(S::MECACELL_V(p19))>::type MECACELL_K(p19);        \
		typename ExportableType<decltype(S::MECACELL_V(p20))>::type MECACELL_K(p20);        \
		typename ExportableType<decltype(S::MECACELL_V(p21))>::type MECACELL_K(p21);        \
		typename ExportableType<decltype(S::MECACELL_V(p22))>::type MECACELL_K(p22);        \
		Exportable() {}                                                                     \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }               \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                      \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                         \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                         \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                         \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                         \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                         \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                         \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                         \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                         \
			ExportableConstruct(MECACELL_K(p8), s.MECACELL_V(p8), s);                         \
			ExportableConstruct(MECACELL_K(p9), s.MECACELL_V(p9), s);                         \
			ExportableConstruct(MECACELL_K(p10), s.MECACELL_V(p10), s);                       \
			ExportableConstruct(MECACELL_K(p11), s.MECACELL_V(p11), s);                       \
			ExportableConstruct(MECACELL_K(p12), s.MECACELL_V(p12), s);                       \
			ExportableConstruct(MECACELL_K(p13), s.MECACELL_V(p13), s);                       \
			ExportableConstruct(MECACELL_K(p14), s.MECACELL_V(p14), s);                       \
			ExportableConstruct(MECACELL_K(p15), s.MECACELL_V(p15), s);                       \
			ExportableConstruct(MECACELL_K(p16), s.MECACELL_V(p16), s);                       \
			ExportableConstruct(MECACELL_K(p17), s.MECACELL_V(p17), s);                       \
			ExportableConstruct(MECACELL_K(p18), s.MECACELL_V(p18), s);                       \
			ExportableConstruct(MECACELL_K(p19), s.MECACELL_V(p19), s);                       \
			ExportableConstruct(MECACELL_K(p20), s.MECACELL_V(p20), s);                       \
			ExportableConstruct(MECACELL_K(p21), s.MECACELL_V(p21), s);                       \
			ExportableConstruct(MECACELL_K(p22), s.MECACELL_V(p22), s);                       \
		}                                                                                   \
		void from_json(const json& o) {                                                     \
			if (o.count(MECACELL_SK(p0)))                                                     \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();            \
			if (o.count(MECACELL_SK(p1)))                                                     \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();            \
			if (o.count(MECACELL_SK(p2)))                                                     \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();            \
			if (o.count(MECACELL_SK(p3)))                                                     \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();            \
			if (o.count(MECACELL_SK(p4)))                                                     \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();            \
			if (o.count(MECACELL_SK(p5)))                                                     \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();            \
			if (o.count(MECACELL_SK(p6)))                                                     \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();            \
			if (o.count(MECACELL_SK(p7)))                                                     \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();            \
			if (o.count(MECACELL_SK(p8)))                                                     \
				MECACELL_K(p8) = o[MECACELL_SK(p8)].get<decltype(MECACELL_K(p8))>();            \
			if (o.count(MECACELL_SK(p9)))                                                     \
				MECACELL_K(p9) = o[MECACELL_SK(p9)].get<decltype(MECACELL_K(p9))>();            \
			if (o.count(MECACELL_SK(p10)))                                                    \
				MECACELL_K(p10) = o[MECACELL_SK(p10)].get<decltype(MECACELL_K(p10))>();         \
			if (o.count(MECACELL_SK(p11)))                                                    \
				MECACELL_K(p11) = o[MECACELL_SK(p11)].get<decltype(MECACELL_K(p11))>();         \
			if (o.count(MECACELL_SK(p12)))                                                    \
				MECACELL_K(p12) = o[MECACELL_SK(p12)].get<decltype(MECACELL_K(p12))>();         \
			if (o.count(MECACELL_SK(p13)))                                                    \
				MECACELL_K(p13) = o[MECACELL_SK(p13)].get<decltype(MECACELL_K(p13))>();         \
			if (o.count(MECACELL_SK(p14)))                                                    \
				MECACELL_K(p14) = o[MECACELL_SK(p14)].get<decltype(MECACELL_K(p14))>();         \
			if (o.count(MECACELL_SK(p15)))                                                    \
				MECACELL_K(p15) = o[MECACELL_SK(p15)].get<decltype(MECACELL_K(p15))>();         \
			if (o.count(MECACELL_SK(p16)))                                                    \
				MECACELL_K(p16) = o[MECACELL_SK(p16)].get<decltype(MECACELL_K(p16))>();         \
			if (o.count(MECACELL_SK(p17)))                                                    \
				MECACELL_K(p17) = o[MECACELL_SK(p17)].get<decltype(MECACELL_K(p17))>();         \
			if (o.count(MECACELL_SK(p18)))                                                    \
				MECACELL_K(p18) = o[MECACELL_SK(p18)].get<decltype(MECACELL_K(p18))>();         \
			if (o.count(MECACELL_SK(p19)))                                                    \
				MECACELL_K(p19) = o[MECACELL_SK(p19)].get<decltype(MECACELL_K(p19))>();         \
			if (o.count(MECACELL_SK(p20)))                                                    \
				MECACELL_K(p20) = o[MECACELL_SK(p20)].get<decltype(MECACELL_K(p20))>();         \
			if (o.count(MECACELL_SK(p21)))                                                    \
				MECACELL_K(p21) = o[MECACELL_SK(p21)].get<decltype(MECACELL_K(p21))>();         \
			if (o.count(MECACELL_SK(p22)))                                                    \
				MECACELL_K(p22) = o[MECACELL_SK(p22)].get<decltype(MECACELL_K(p22))>();         \
		}                                                                                   \
		json to_json() const {                                                              \
			auto b = BType::to_json();                                                        \
			auto r = to_jsonL(*this);                                                         \
			if (!b.empty()) r.update(b);                                                      \
			return r;                                                                         \
		}                                                                                   \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {         \
			return s.saveJ(                                                                   \
			    MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1), s.MECACELL_K(p1),         \
			    MECACELL_SK(p2), s.MECACELL_K(p2), MECACELL_SK(p3), s.MECACELL_K(p3),         \
			    MECACELL_SK(p4), s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),         \
			    MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7), s.MECACELL_K(p7),         \
			    MECACELL_SK(p8), s.MECACELL_K(p8), MECACELL_SK(p9), s.MECACELL_K(p9),         \
			    MECACELL_SK(p10), s.MECACELL_K(p10), MECACELL_SK(p11), s.MECACELL_K(p11),     \
			    MECACELL_SK(p12), s.MECACELL_K(p12), MECACELL_SK(p13), s.MECACELL_K(p13),     \
			    MECACELL_SK(p14), s.MECACELL_K(p14), MECACELL_SK(p15), s.MECACELL_K(p15),     \
			    MECACELL_SK(p16), s.MECACELL_K(p16), MECACELL_SK(p17), s.MECACELL_K(p17),     \
			    MECACELL_SK(p18), s.MECACELL_K(p18), MECACELL_SK(p19), s.MECACELL_K(p19),     \
			    MECACELL_SK(p20), s.MECACELL_K(p20), MECACELL_SK(p21), s.MECACELL_K(p21),     \
			    MECACELL_SK(p22), s.MECACELL_K(p22));                                         \
		};                                                                                  \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); };     \
	};                                                                                    \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE24(S, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, \
                     v15, v16, v17, v18, v19, v20, v21, v22)                             \
	EXPORTABLE_INHERIT25(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,   \
	                     v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22)

#define EXPORTABLE_INHERIT26(S, Base, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11,  \
                             p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23) \
	struct Exportable : ExportableType<Base>::type {                                       \
		using BType = ExportableType<Base>::type;                                            \
		using json = nlohmann::json;                                                         \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);           \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);           \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);           \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);           \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);           \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);           \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);           \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);           \
		typename ExportableType<decltype(S::MECACELL_V(p8))>::type MECACELL_K(p8);           \
		typename ExportableType<decltype(S::MECACELL_V(p9))>::type MECACELL_K(p9);           \
		typename ExportableType<decltype(S::MECACELL_V(p10))>::type MECACELL_K(p10);         \
		typename ExportableType<decltype(S::MECACELL_V(p11))>::type MECACELL_K(p11);         \
		typename ExportableType<decltype(S::MECACELL_V(p12))>::type MECACELL_K(p12);         \
		typename ExportableType<decltype(S::MECACELL_V(p13))>::type MECACELL_K(p13);         \
		typename ExportableType<decltype(S::MECACELL_V(p14))>::type MECACELL_K(p14);         \
		typename ExportableType<decltype(S::MECACELL_V(p15))>::type MECACELL_K(p15);         \
		typename ExportableType<decltype(S::MECACELL_V(p16))>::type MECACELL_K(p16);         \
		typename ExportableType<decltype(S::MECACELL_V(p17))>::type MECACELL_K(p17);         \
		typename ExportableType<decltype(S::MECACELL_V(p18))>::type MECACELL_K(p18);         \
		typename ExportableType<decltype(S::MECACELL_V(p19))>::type MECACELL_K(p19);         \
		typename ExportableType<decltype(S::MECACELL_V(p20))>::type MECACELL_K(p20);         \
		typename ExportableType<decltype(S::MECACELL_V(p21))>::type MECACELL_K(p21);         \
		typename ExportableType<decltype(S::MECACELL_V(p22))>::type MECACELL_K(p22);         \
		typename ExportableType<decltype(S::MECACELL_V(p23))>::type MECACELL_K(p23);         \
		Exportable() {}                                                                      \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }                \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                       \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                          \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                          \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                          \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                          \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                          \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                          \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                          \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                          \
			ExportableConstruct(MECACELL_K(p8), s.MECACELL_V(p8), s);                          \
			ExportableConstruct(MECACELL_K(p9), s.MECACELL_V(p9), s);                          \
			ExportableConstruct(MECACELL_K(p10), s.MECACELL_V(p10), s);                        \
			ExportableConstruct(MECACELL_K(p11), s.MECACELL_V(p11), s);                        \
			ExportableConstruct(MECACELL_K(p12), s.MECACELL_V(p12), s);                        \
			ExportableConstruct(MECACELL_K(p13), s.MECACELL_V(p13), s);                        \
			ExportableConstruct(MECACELL_K(p14), s.MECACELL_V(p14), s);                        \
			ExportableConstruct(MECACELL_K(p15), s.MECACELL_V(p15), s);                        \
			ExportableConstruct(MECACELL_K(p16), s.MECACELL_V(p16), s);                        \
			ExportableConstruct(MECACELL_K(p17), s.MECACELL_V(p17), s);                        \
			ExportableConstruct(MECACELL_K(p18), s.MECACELL_V(p18), s);                        \
			ExportableConstruct(MECACELL_K(p19), s.MECACELL_V(p19), s);                        \
			ExportableConstruct(MECACELL_K(p20), s.MECACELL_V(p20), s);                        \
			ExportableConstruct(MECACELL_K(p21), s.MECACELL_V(p21), s);                        \
			ExportableConstruct(MECACELL_K(p22), s.MECACELL_V(p22), s);                        \
			ExportableConstruct(MECACELL_K(p23), s.MECACELL_V(p23), s);                        \
		}                                                                                    \
		void from_json(const json& o) {                                                      \
			if (o.count(MECACELL_SK(p0)))                                                      \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();             \
			if (o.count(MECACELL_SK(p1)))                                                      \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();             \
			if (o.count(MECACELL_SK(p2)))                                                      \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();             \
			if (o.count(MECACELL_SK(p3)))                                                      \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();             \
			if (o.count(MECACELL_SK(p4)))                                                      \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();             \
			if (o.count(MECACELL_SK(p5)))                                                      \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();             \
			if (o.count(MECACELL_SK(p6)))                                                      \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();             \
			if (o.count(MECACELL_SK(p7)))                                                      \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();             \
			if (o.count(MECACELL_SK(p8)))                                                      \
				MECACELL_K(p8) = o[MECACELL_SK(p8)].get<decltype(MECACELL_K(p8))>();             \
			if (o.count(MECACELL_SK(p9)))                                                      \
				MECACELL_K(p9) = o[MECACELL_SK(p9)].get<decltype(MECACELL_K(p9))>();             \
			if (o.count(MECACELL_SK(p10)))                                                     \
				MECACELL_K(p10) = o[MECACELL_SK(p10)].get<decltype(MECACELL_K(p10))>();          \
			if (o.count(MECACELL_SK(p11)))                                                     \
				MECACELL_K(p11) = o[MECACELL_SK(p11)].get<decltype(MECACELL_K(p11))>();          \
			if (o.count(MECACELL_SK(p12)))                                                     \
				MECACELL_K(p12) = o[MECACELL_SK(p12)].get<decltype(MECACELL_K(p12))>();          \
			if (o.count(MECACELL_SK(p13)))                                                     \
				MECACELL_K(p13) = o[MECACELL_SK(p13)].get<decltype(MECACELL_K(p13))>();          \
			if (o.count(MECACELL_SK(p14)))                                                     \
				MECACELL_K(p14) = o[MECACELL_SK(p14)].get<decltype(MECACELL_K(p14))>();          \
			if (o.count(MECACELL_SK(p15)))                                                     \
				MECACELL_K(p15) = o[MECACELL_SK(p15)].get<decltype(MECACELL_K(p15))>();          \
			if (o.count(MECACELL_SK(p16)))                                                     \
				MECACELL_K(p16) = o[MECACELL_SK(p16)].get<decltype(MECACELL_K(p16))>();          \
			if (o.count(MECACELL_SK(p17)))                                                     \
				MECACELL_K(p17) = o[MECACELL_SK(p17)].get<decltype(MECACELL_K(p17))>();          \
			if (o.count(MECACELL_SK(p18)))                                                     \
				MECACELL_K(p18) = o[MECACELL_SK(p18)].get<decltype(MECACELL_K(p18))>();          \
			if (o.count(MECACELL_SK(p19)))                                                     \
				MECACELL_K(p19) = o[MECACELL_SK(p19)].get<decltype(MECACELL_K(p19))>();          \
			if (o.count(MECACELL_SK(p20)))                                                     \
				MECACELL_K(p20) = o[MECACELL_SK(p20)].get<decltype(MECACELL_K(p20))>();          \
			if (o.count(MECACELL_SK(p21)))                                                     \
				MECACELL_K(p21) = o[MECACELL_SK(p21)].get<decltype(MECACELL_K(p21))>();          \
			if (o.count(MECACELL_SK(p22)))                                                     \
				MECACELL_K(p22) = o[MECACELL_SK(p22)].get<decltype(MECACELL_K(p22))>();          \
			if (o.count(MECACELL_SK(p23)))                                                     \
				MECACELL_K(p23) = o[MECACELL_SK(p23)].get<decltype(MECACELL_K(p23))>();          \
		}                                                                                    \
		json to_json() const {                                                               \
			auto b = BType::to_json();                                                         \
			auto r = to_jsonL(*this);                                                          \
			if (!b.empty()) r.update(b);                                                       \
			return r;                                                                          \
		}                                                                                    \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {          \
			return s.saveJ(                                                                    \
			    MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1), s.MECACELL_K(p1),          \
			    MECACELL_SK(p2), s.MECACELL_K(p2), MECACELL_SK(p3), s.MECACELL_K(p3),          \
			    MECACELL_SK(p4), s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),          \
			    MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7), s.MECACELL_K(p7),          \
			    MECACELL_SK(p8), s.MECACELL_K(p8), MECACELL_SK(p9), s.MECACELL_K(p9),          \
			    MECACELL_SK(p10), s.MECACELL_K(p10), MECACELL_SK(p11), s.MECACELL_K(p11),      \
			    MECACELL_SK(p12), s.MECACELL_K(p12), MECACELL_SK(p13), s.MECACELL_K(p13),      \
			    MECACELL_SK(p14), s.MECACELL_K(p14), MECACELL_SK(p15), s.MECACELL_K(p15),      \
			    MECACELL_SK(p16), s.MECACELL_K(p16), MECACELL_SK(p17), s.MECACELL_K(p17),      \
			    MECACELL_SK(p18), s.MECACELL_K(p18), MECACELL_SK(p19), s.MECACELL_K(p19),      \
			    MECACELL_SK(p20), s.MECACELL_K(p20), MECACELL_SK(p21), s.MECACELL_K(p21),      \
			    MECACELL_SK(p22), s.MECACELL_K(p22), MECACELL_SK(p23), s.MECACELL_K(p23));     \
		};                                                                                   \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); };      \
	};                                                                                     \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE25(S, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, \
                     v15, v16, v17, v18, v19, v20, v21, v22, v23)                        \
	EXPORTABLE_INHERIT26(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,   \
	                     v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23)

#define EXPORTABLE_INHERIT27(S, Base, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11,  \
                             p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, \
                             p24)                                                        \
	struct Exportable : ExportableType<Base>::type {                                       \
		using BType = ExportableType<Base>::type;                                            \
		using json = nlohmann::json;                                                         \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);           \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);           \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);           \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);           \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);           \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);           \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);           \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);           \
		typename ExportableType<decltype(S::MECACELL_V(p8))>::type MECACELL_K(p8);           \
		typename ExportableType<decltype(S::MECACELL_V(p9))>::type MECACELL_K(p9);           \
		typename ExportableType<decltype(S::MECACELL_V(p10))>::type MECACELL_K(p10);         \
		typename ExportableType<decltype(S::MECACELL_V(p11))>::type MECACELL_K(p11);         \
		typename ExportableType<decltype(S::MECACELL_V(p12))>::type MECACELL_K(p12);         \
		typename ExportableType<decltype(S::MECACELL_V(p13))>::type MECACELL_K(p13);         \
		typename ExportableType<decltype(S::MECACELL_V(p14))>::type MECACELL_K(p14);         \
		typename ExportableType<decltype(S::MECACELL_V(p15))>::type MECACELL_K(p15);         \
		typename ExportableType<decltype(S::MECACELL_V(p16))>::type MECACELL_K(p16);         \
		typename ExportableType<decltype(S::MECACELL_V(p17))>::type MECACELL_K(p17);         \
		typename ExportableType<decltype(S::MECACELL_V(p18))>::type MECACELL_K(p18);         \
		typename ExportableType<decltype(S::MECACELL_V(p19))>::type MECACELL_K(p19);         \
		typename ExportableType<decltype(S::MECACELL_V(p20))>::type MECACELL_K(p20);         \
		typename ExportableType<decltype(S::MECACELL_V(p21))>::type MECACELL_K(p21);         \
		typename ExportableType<decltype(S::MECACELL_V(p22))>::type MECACELL_K(p22);         \
		typename ExportableType<decltype(S::MECACELL_V(p23))>::type MECACELL_K(p23);         \
		typename ExportableType<decltype(S::MECACELL_V(p24))>::type MECACELL_K(p24);         \
		Exportable() {}                                                                      \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }                \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                       \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                          \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                          \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                          \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                          \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                          \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                          \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                          \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                          \
			ExportableConstruct(MECACELL_K(p8), s.MECACELL_V(p8), s);                          \
			ExportableConstruct(MECACELL_K(p9), s.MECACELL_V(p9), s);                          \
			ExportableConstruct(MECACELL_K(p10), s.MECACELL_V(p10), s);                        \
			ExportableConstruct(MECACELL_K(p11), s.MECACELL_V(p11), s);                        \
			ExportableConstruct(MECACELL_K(p12), s.MECACELL_V(p12), s);                        \
			ExportableConstruct(MECACELL_K(p13), s.MECACELL_V(p13), s);                        \
			ExportableConstruct(MECACELL_K(p14), s.MECACELL_V(p14), s);                        \
			ExportableConstruct(MECACELL_K(p15), s.MECACELL_V(p15), s);                        \
			ExportableConstruct(MECACELL_K(p16), s.MECACELL_V(p16), s);                        \
			ExportableConstruct(MECACELL_K(p17), s.MECACELL_V(p17), s);                        \
			ExportableConstruct(MECACELL_K(p18), s.MECACELL_V(p18), s);                        \
			ExportableConstruct(MECACELL_K(p19), s.MECACELL_V(p19), s);                        \
			ExportableConstruct(MECACELL_K(p20), s.MECACELL_V(p20), s);                        \
			ExportableConstruct(MECACELL_K(p21), s.MECACELL_V(p21), s);                        \
			ExportableConstruct(MECACELL_K(p22), s.MECACELL_V(p22), s);                        \
			ExportableConstruct(MECACELL_K(p23), s.MECACELL_V(p23), s);                        \
			ExportableConstruct(MECACELL_K(p24), s.MECACELL_V(p24), s);                        \
		}                                                                                    \
		void from_json(const json& o) {                                                      \
			if (o.count(MECACELL_SK(p0)))                                                      \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();             \
			if (o.count(MECACELL_SK(p1)))                                                      \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();             \
			if (o.count(MECACELL_SK(p2)))                                                      \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();             \
			if (o.count(MECACELL_SK(p3)))                                                      \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();             \
			if (o.count(MECACELL_SK(p4)))                                                      \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();             \
			if (o.count(MECACELL_SK(p5)))                                                      \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();             \
			if (o.count(MECACELL_SK(p6)))                                                      \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();             \
			if (o.count(MECACELL_SK(p7)))                                                      \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();             \
			if (o.count(MECACELL_SK(p8)))                                                      \
				MECACELL_K(p8) = o[MECACELL_SK(p8)].get<decltype(MECACELL_K(p8))>();             \
			if (o.count(MECACELL_SK(p9)))                                                      \
				MECACELL_K(p9) = o[MECACELL_SK(p9)].get<decltype(MECACELL_K(p9))>();             \
			if (o.count(MECACELL_SK(p10)))                                                     \
				MECACELL_K(p10) = o[MECACELL_SK(p10)].get<decltype(MECACELL_K(p10))>();          \
			if (o.count(MECACELL_SK(p11)))                                                     \
				MECACELL_K(p11) = o[MECACELL_SK(p11)].get<decltype(MECACELL_K(p11))>();          \
			if (o.count(MECACELL_SK(p12)))                                                     \
				MECACELL_K(p12) = o[MECACELL_SK(p12)].get<decltype(MECACELL_K(p12))>();          \
			if (o.count(MECACELL_SK(p13)))                                                     \
				MECACELL_K(p13) = o[MECACELL_SK(p13)].get<decltype(MECACELL_K(p13))>();          \
			if (o.count(MECACELL_SK(p14)))                                                     \
				MECACELL_K(p14) = o[MECACELL_SK(p14)].get<decltype(MECACELL_K(p14))>();          \
			if (o.count(MECACELL_SK(p15)))                                                     \
				MECACELL_K(p15) = o[MECACELL_SK(p15)].get<decltype(MECACELL_K(p15))>();          \
			if (o.count(MECACELL_SK(p16)))                                                     \
				MECACELL_K(p16) = o[MECACELL_SK(p16)].get<decltype(MECACELL_K(p16))>();          \
			if (o.count(MECACELL_SK(p17)))                                                     \
				MECACELL_K(p17) = o[MECACELL_SK(p17)].get<decltype(MECACELL_K(p17))>();          \
			if (o.count(MECACELL_SK(p18)))                                                     \
				MECACELL_K(p18) = o[MECACELL_SK(p18)].get<decltype(MECACELL_K(p18))>();          \
			if (o.count(MECACELL_SK(p19)))                                                     \
				MECACELL_K(p19) = o[MECACELL_SK(p19)].get<decltype(MECACELL_K(p19))>();          \
			if (o.count(MECACELL_SK(p20)))                                                     \
				MECACELL_K(p20) = o[MECACELL_SK(p20)].get<decltype(MECACELL_K(p20))>();          \
			if (o.count(MECACELL_SK(p21)))                                                     \
				MECACELL_K(p21) = o[MECACELL_SK(p21)].get<decltype(MECACELL_K(p21))>();          \
			if (o.count(MECACELL_SK(p22)))                                                     \
				MECACELL_K(p22) = o[MECACELL_SK(p22)].get<decltype(MECACELL_K(p22))>();          \
			if (o.count(MECACELL_SK(p23)))                                                     \
				MECACELL_K(p23) = o[MECACELL_SK(p23)].get<decltype(MECACELL_K(p23))>();          \
			if (o.count(MECACELL_SK(p24)))                                                     \
				MECACELL_K(p24) = o[MECACELL_SK(p24)].get<decltype(MECACELL_K(p24))>();          \
		}                                                                                    \
		json to_json() const {                                                               \
			auto b = BType::to_json();                                                         \
			auto r = to_jsonL(*this);                                                          \
			if (!b.empty()) r.update(b);                                                       \
			return r;                                                                          \
		}                                                                                    \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {          \
			return s.saveJ(                                                                    \
			    MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1), s.MECACELL_K(p1),          \
			    MECACELL_SK(p2), s.MECACELL_K(p2), MECACELL_SK(p3), s.MECACELL_K(p3),          \
			    MECACELL_SK(p4), s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),          \
			    MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7), s.MECACELL_K(p7),          \
			    MECACELL_SK(p8), s.MECACELL_K(p8), MECACELL_SK(p9), s.MECACELL_K(p9),          \
			    MECACELL_SK(p10), s.MECACELL_K(p10), MECACELL_SK(p11), s.MECACELL_K(p11),      \
			    MECACELL_SK(p12), s.MECACELL_K(p12), MECACELL_SK(p13), s.MECACELL_K(p13),      \
			    MECACELL_SK(p14), s.MECACELL_K(p14), MECACELL_SK(p15), s.MECACELL_K(p15),      \
			    MECACELL_SK(p16), s.MECACELL_K(p16), MECACELL_SK(p17), s.MECACELL_K(p17),      \
			    MECACELL_SK(p18), s.MECACELL_K(p18), MECACELL_SK(p19), s.MECACELL_K(p19),      \
			    MECACELL_SK(p20), s.MECACELL_K(p20), MECACELL_SK(p21), s.MECACELL_K(p21),      \
			    MECACELL_SK(p22), s.MECACELL_K(p22), MECACELL_SK(p23), s.MECACELL_K(p23),      \
			    MECACELL_SK(p24), s.MECACELL_K(p24));                                          \
		};                                                                                   \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); };      \
	};                                                                                     \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE26(S, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, \
                     v15, v16, v17, v18, v19, v20, v21, v22, v23, v24)                   \
	EXPORTABLE_INHERIT27(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,   \
	                     v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23,  \
	                     v24)

#define EXPORTABLE_INHERIT28(S, Base, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11,  \
                             p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, \
                             p24, p25)                                                   \
	struct Exportable : ExportableType<Base>::type {                                       \
		using BType = ExportableType<Base>::type;                                            \
		using json = nlohmann::json;                                                         \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);           \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);           \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);           \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);           \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);           \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);           \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);           \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);           \
		typename ExportableType<decltype(S::MECACELL_V(p8))>::type MECACELL_K(p8);           \
		typename ExportableType<decltype(S::MECACELL_V(p9))>::type MECACELL_K(p9);           \
		typename ExportableType<decltype(S::MECACELL_V(p10))>::type MECACELL_K(p10);         \
		typename ExportableType<decltype(S::MECACELL_V(p11))>::type MECACELL_K(p11);         \
		typename ExportableType<decltype(S::MECACELL_V(p12))>::type MECACELL_K(p12);         \
		typename ExportableType<decltype(S::MECACELL_V(p13))>::type MECACELL_K(p13);         \
		typename ExportableType<decltype(S::MECACELL_V(p14))>::type MECACELL_K(p14);         \
		typename ExportableType<decltype(S::MECACELL_V(p15))>::type MECACELL_K(p15);         \
		typename ExportableType<decltype(S::MECACELL_V(p16))>::type MECACELL_K(p16);         \
		typename ExportableType<decltype(S::MECACELL_V(p17))>::type MECACELL_K(p17);         \
		typename ExportableType<decltype(S::MECACELL_V(p18))>::type MECACELL_K(p18);         \
		typename ExportableType<decltype(S::MECACELL_V(p19))>::type MECACELL_K(p19);         \
		typename ExportableType<decltype(S::MECACELL_V(p20))>::type MECACELL_K(p20);         \
		typename ExportableType<decltype(S::MECACELL_V(p21))>::type MECACELL_K(p21);         \
		typename ExportableType<decltype(S::MECACELL_V(p22))>::type MECACELL_K(p22);         \
		typename ExportableType<decltype(S::MECACELL_V(p23))>::type MECACELL_K(p23);         \
		typename ExportableType<decltype(S::MECACELL_V(p24))>::type MECACELL_K(p24);         \
		typename ExportableType<decltype(S::MECACELL_V(p25))>::type MECACELL_K(p25);         \
		Exportable() {}                                                                      \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }                \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                       \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                          \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                          \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                          \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                          \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                          \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                          \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                          \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                          \
			ExportableConstruct(MECACELL_K(p8), s.MECACELL_V(p8), s);                          \
			ExportableConstruct(MECACELL_K(p9), s.MECACELL_V(p9), s);                          \
			ExportableConstruct(MECACELL_K(p10), s.MECACELL_V(p10), s);                        \
			ExportableConstruct(MECACELL_K(p11), s.MECACELL_V(p11), s);                        \
			ExportableConstruct(MECACELL_K(p12), s.MECACELL_V(p12), s);                        \
			ExportableConstruct(MECACELL_K(p13), s.MECACELL_V(p13), s);                        \
			ExportableConstruct(MECACELL_K(p14), s.MECACELL_V(p14), s);                        \
			ExportableConstruct(MECACELL_K(p15), s.MECACELL_V(p15), s);                        \
			ExportableConstruct(MECACELL_K(p16), s.MECACELL_V(p16), s);                        \
			ExportableConstruct(MECACELL_K(p17), s.MECACELL_V(p17), s);                        \
			ExportableConstruct(MECACELL_K(p18), s.MECACELL_V(p18), s);                        \
			ExportableConstruct(MECACELL_K(p19), s.MECACELL_V(p19), s);                        \
			ExportableConstruct(MECACELL_K(p20), s.MECACELL_V(p20), s);                        \
			ExportableConstruct(MECACELL_K(p21), s.MECACELL_V(p21), s);                        \
			ExportableConstruct(MECACELL_K(p22), s.MECACELL_V(p22), s);                        \
			ExportableConstruct(MECACELL_K(p23), s.MECACELL_V(p23), s);                        \
			ExportableConstruct(MECACELL_K(p24), s.MECACELL_V(p24), s);                        \
			ExportableConstruct(MECACELL_K(p25), s.MECACELL_V(p25), s);                        \
		}                                                                                    \
		void from_json(const json& o) {                                                      \
			if (o.count(MECACELL_SK(p0)))                                                      \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();             \
			if (o.count(MECACELL_SK(p1)))                                                      \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();             \
			if (o.count(MECACELL_SK(p2)))                                                      \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();             \
			if (o.count(MECACELL_SK(p3)))                                                      \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();             \
			if (o.count(MECACELL_SK(p4)))                                                      \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();             \
			if (o.count(MECACELL_SK(p5)))                                                      \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();             \
			if (o.count(MECACELL_SK(p6)))                                                      \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();             \
			if (o.count(MECACELL_SK(p7)))                                                      \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();             \
			if (o.count(MECACELL_SK(p8)))                                                      \
				MECACELL_K(p8) = o[MECACELL_SK(p8)].get<decltype(MECACELL_K(p8))>();             \
			if (o.count(MECACELL_SK(p9)))                                                      \
				MECACELL_K(p9) = o[MECACELL_SK(p9)].get<decltype(MECACELL_K(p9))>();             \
			if (o.count(MECACELL_SK(p10)))                                                     \
				MECACELL_K(p10) = o[MECACELL_SK(p10)].get<decltype(MECACELL_K(p10))>();          \
			if (o.count(MECACELL_SK(p11)))                                                     \
				MECACELL_K(p11) = o[MECACELL_SK(p11)].get<decltype(MECACELL_K(p11))>();          \
			if (o.count(MECACELL_SK(p12)))                                                     \
				MECACELL_K(p12) = o[MECACELL_SK(p12)].get<decltype(MECACELL_K(p12))>();          \
			if (o.count(MECACELL_SK(p13)))                                                     \
				MECACELL_K(p13) = o[MECACELL_SK(p13)].get<decltype(MECACELL_K(p13))>();          \
			if (o.count(MECACELL_SK(p14)))                                                     \
				MECACELL_K(p14) = o[MECACELL_SK(p14)].get<decltype(MECACELL_K(p14))>();          \
			if (o.count(MECACELL_SK(p15)))                                                     \
				MECACELL_K(p15) = o[MECACELL_SK(p15)].get<decltype(MECACELL_K(p15))>();          \
			if (o.count(MECACELL_SK(p16)))                                                     \
				MECACELL_K(p16) = o[MECACELL_SK(p16)].get<decltype(MECACELL_K(p16))>();          \
			if (o.count(MECACELL_SK(p17)))                                                     \
				MECACELL_K(p17) = o[MECACELL_SK(p17)].get<decltype(MECACELL_K(p17))>();          \
			if (o.count(MECACELL_SK(p18)))                                                     \
				MECACELL_K(p18) = o[MECACELL_SK(p18)].get<decltype(MECACELL_K(p18))>();          \
			if (o.count(MECACELL_SK(p19)))                                                     \
				MECACELL_K(p19) = o[MECACELL_SK(p19)].get<decltype(MECACELL_K(p19))>();          \
			if (o.count(MECACELL_SK(p20)))                                                     \
				MECACELL_K(p20) = o[MECACELL_SK(p20)].get<decltype(MECACELL_K(p20))>();          \
			if (o.count(MECACELL_SK(p21)))                                                     \
				MECACELL_K(p21) = o[MECACELL_SK(p21)].get<decltype(MECACELL_K(p21))>();          \
			if (o.count(MECACELL_SK(p22)))                                                     \
				MECACELL_K(p22) = o[MECACELL_SK(p22)].get<decltype(MECACELL_K(p22))>();          \
			if (o.count(MECACELL_SK(p23)))                                                     \
				MECACELL_K(p23) = o[MECACELL_SK(p23)].get<decltype(MECACELL_K(p23))>();          \
			if (o.count(MECACELL_SK(p24)))                                                     \
				MECACELL_K(p24) = o[MECACELL_SK(p24)].get<decltype(MECACELL_K(p24))>();          \
			if (o.count(MECACELL_SK(p25)))                                                     \
				MECACELL_K(p25) = o[MECACELL_SK(p25)].get<decltype(MECACELL_K(p25))>();          \
		}                                                                                    \
		json to_json() const {                                                               \
			auto b = BType::to_json();                                                         \
			auto r = to_jsonL(*this);                                                          \
			if (!b.empty()) r.update(b);                                                       \
			return r;                                                                          \
		}                                                                                    \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {          \
			return s.saveJ(                                                                    \
			    MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1), s.MECACELL_K(p1),          \
			    MECACELL_SK(p2), s.MECACELL_K(p2), MECACELL_SK(p3), s.MECACELL_K(p3),          \
			    MECACELL_SK(p4), s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),          \
			    MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7), s.MECACELL_K(p7),          \
			    MECACELL_SK(p8), s.MECACELL_K(p8), MECACELL_SK(p9), s.MECACELL_K(p9),          \
			    MECACELL_SK(p10), s.MECACELL_K(p10), MECACELL_SK(p11), s.MECACELL_K(p11),      \
			    MECACELL_SK(p12), s.MECACELL_K(p12), MECACELL_SK(p13), s.MECACELL_K(p13),      \
			    MECACELL_SK(p14), s.MECACELL_K(p14), MECACELL_SK(p15), s.MECACELL_K(p15),      \
			    MECACELL_SK(p16), s.MECACELL_K(p16), MECACELL_SK(p17), s.MECACELL_K(p17),      \
			    MECACELL_SK(p18), s.MECACELL_K(p18), MECACELL_SK(p19), s.MECACELL_K(p19),      \
			    MECACELL_SK(p20), s.MECACELL_K(p20), MECACELL_SK(p21), s.MECACELL_K(p21),      \
			    MECACELL_SK(p22), s.MECACELL_K(p22), MECACELL_SK(p23), s.MECACELL_K(p23),      \
			    MECACELL_SK(p24), s.MECACELL_K(p24), MECACELL_SK(p25), s.MECACELL_K(p25));     \
		};                                                                                   \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); };      \
	};                                                                                     \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE27(S, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, \
                     v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25)              \
	EXPORTABLE_INHERIT28(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,   \
	                     v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23,  \
	                     v24, v25)

#define EXPORTABLE_INHERIT29(S, Base, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11,  \
                             p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, \
                             p24, p25, p26)                                              \
	struct Exportable : ExportableType<Base>::type {                                       \
		using BType = ExportableType<Base>::type;                                            \
		using json = nlohmann::json;                                                         \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);           \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);           \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);           \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);           \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);           \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);           \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);           \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);           \
		typename ExportableType<decltype(S::MECACELL_V(p8))>::type MECACELL_K(p8);           \
		typename ExportableType<decltype(S::MECACELL_V(p9))>::type MECACELL_K(p9);           \
		typename ExportableType<decltype(S::MECACELL_V(p10))>::type MECACELL_K(p10);         \
		typename ExportableType<decltype(S::MECACELL_V(p11))>::type MECACELL_K(p11);         \
		typename ExportableType<decltype(S::MECACELL_V(p12))>::type MECACELL_K(p12);         \
		typename ExportableType<decltype(S::MECACELL_V(p13))>::type MECACELL_K(p13);         \
		typename ExportableType<decltype(S::MECACELL_V(p14))>::type MECACELL_K(p14);         \
		typename ExportableType<decltype(S::MECACELL_V(p15))>::type MECACELL_K(p15);         \
		typename ExportableType<decltype(S::MECACELL_V(p16))>::type MECACELL_K(p16);         \
		typename ExportableType<decltype(S::MECACELL_V(p17))>::type MECACELL_K(p17);         \
		typename ExportableType<decltype(S::MECACELL_V(p18))>::type MECACELL_K(p18);         \
		typename ExportableType<decltype(S::MECACELL_V(p19))>::type MECACELL_K(p19);         \
		typename ExportableType<decltype(S::MECACELL_V(p20))>::type MECACELL_K(p20);         \
		typename ExportableType<decltype(S::MECACELL_V(p21))>::type MECACELL_K(p21);         \
		typename ExportableType<decltype(S::MECACELL_V(p22))>::type MECACELL_K(p22);         \
		typename ExportableType<decltype(S::MECACELL_V(p23))>::type MECACELL_K(p23);         \
		typename ExportableType<decltype(S::MECACELL_V(p24))>::type MECACELL_K(p24);         \
		typename ExportableType<decltype(S::MECACELL_V(p25))>::type MECACELL_K(p25);         \
		typename ExportableType<decltype(S::MECACELL_V(p26))>::type MECACELL_K(p26);         \
		Exportable() {}                                                                      \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }                \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                       \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                          \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                          \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                          \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                          \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                          \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                          \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                          \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                          \
			ExportableConstruct(MECACELL_K(p8), s.MECACELL_V(p8), s);                          \
			ExportableConstruct(MECACELL_K(p9), s.MECACELL_V(p9), s);                          \
			ExportableConstruct(MECACELL_K(p10), s.MECACELL_V(p10), s);                        \
			ExportableConstruct(MECACELL_K(p11), s.MECACELL_V(p11), s);                        \
			ExportableConstruct(MECACELL_K(p12), s.MECACELL_V(p12), s);                        \
			ExportableConstruct(MECACELL_K(p13), s.MECACELL_V(p13), s);                        \
			ExportableConstruct(MECACELL_K(p14), s.MECACELL_V(p14), s);                        \
			ExportableConstruct(MECACELL_K(p15), s.MECACELL_V(p15), s);                        \
			ExportableConstruct(MECACELL_K(p16), s.MECACELL_V(p16), s);                        \
			ExportableConstruct(MECACELL_K(p17), s.MECACELL_V(p17), s);                        \
			ExportableConstruct(MECACELL_K(p18), s.MECACELL_V(p18), s);                        \
			ExportableConstruct(MECACELL_K(p19), s.MECACELL_V(p19), s);                        \
			ExportableConstruct(MECACELL_K(p20), s.MECACELL_V(p20), s);                        \
			ExportableConstruct(MECACELL_K(p21), s.MECACELL_V(p21), s);                        \
			ExportableConstruct(MECACELL_K(p22), s.MECACELL_V(p22), s);                        \
			ExportableConstruct(MECACELL_K(p23), s.MECACELL_V(p23), s);                        \
			ExportableConstruct(MECACELL_K(p24), s.MECACELL_V(p24), s);                        \
			ExportableConstruct(MECACELL_K(p25), s.MECACELL_V(p25), s);                        \
			ExportableConstruct(MECACELL_K(p26), s.MECACELL_V(p26), s);                        \
		}                                                                                    \
		void from_json(const json& o) {                                                      \
			if (o.count(MECACELL_SK(p0)))                                                      \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();             \
			if (o.count(MECACELL_SK(p1)))                                                      \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();             \
			if (o.count(MECACELL_SK(p2)))                                                      \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();             \
			if (o.count(MECACELL_SK(p3)))                                                      \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();             \
			if (o.count(MECACELL_SK(p4)))                                                      \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();             \
			if (o.count(MECACELL_SK(p5)))                                                      \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();             \
			if (o.count(MECACELL_SK(p6)))                                                      \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();             \
			if (o.count(MECACELL_SK(p7)))                                                      \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();             \
			if (o.count(MECACELL_SK(p8)))                                                      \
				MECACELL_K(p8) = o[MECACELL_SK(p8)].get<decltype(MECACELL_K(p8))>();             \
			if (o.count(MECACELL_SK(p9)))                                                      \
				MECACELL_K(p9) = o[MECACELL_SK(p9)].get<decltype(MECACELL_K(p9))>();             \
			if (o.count(MECACELL_SK(p10)))                                                     \
				MECACELL_K(p10) = o[MECACELL_SK(p10)].get<decltype(MECACELL_K(p10))>();          \
			if (o.count(MECACELL_SK(p11)))                                                     \
				MECACELL_K(p11) = o[MECACELL_SK(p11)].get<decltype(MECACELL_K(p11))>();          \
			if (o.count(MECACELL_SK(p12)))                                                     \
				MECACELL_K(p12) = o[MECACELL_SK(p12)].get<decltype(MECACELL_K(p12))>();          \
			if (o.count(MECACELL_SK(p13)))                                                     \
				MECACELL_K(p13) = o[MECACELL_SK(p13)].get<decltype(MECACELL_K(p13))>();          \
			if (o.count(MECACELL_SK(p14)))                                                     \
				MECACELL_K(p14) = o[MECACELL_SK(p14)].get<decltype(MECACELL_K(p14))>();          \
			if (o.count(MECACELL_SK(p15)))                                                     \
				MECACELL_K(p15) = o[MECACELL_SK(p15)].get<decltype(MECACELL_K(p15))>();          \
			if (o.count(MECACELL_SK(p16)))                                                     \
				MECACELL_K(p16) = o[MECACELL_SK(p16)].get<decltype(MECACELL_K(p16))>();          \
			if (o.count(MECACELL_SK(p17)))                                                     \
				MECACELL_K(p17) = o[MECACELL_SK(p17)].get<decltype(MECACELL_K(p17))>();          \
			if (o.count(MECACELL_SK(p18)))                                                     \
				MECACELL_K(p18) = o[MECACELL_SK(p18)].get<decltype(MECACELL_K(p18))>();          \
			if (o.count(MECACELL_SK(p19)))                                                     \
				MECACELL_K(p19) = o[MECACELL_SK(p19)].get<decltype(MECACELL_K(p19))>();          \
			if (o.count(MECACELL_SK(p20)))                                                     \
				MECACELL_K(p20) = o[MECACELL_SK(p20)].get<decltype(MECACELL_K(p20))>();          \
			if (o.count(MECACELL_SK(p21)))                                                     \
				MECACELL_K(p21) = o[MECACELL_SK(p21)].get<decltype(MECACELL_K(p21))>();          \
			if (o.count(MECACELL_SK(p22)))                                                     \
				MECACELL_K(p22) = o[MECACELL_SK(p22)].get<decltype(MECACELL_K(p22))>();          \
			if (o.count(MECACELL_SK(p23)))                                                     \
				MECACELL_K(p23) = o[MECACELL_SK(p23)].get<decltype(MECACELL_K(p23))>();          \
			if (o.count(MECACELL_SK(p24)))                                                     \
				MECACELL_K(p24) = o[MECACELL_SK(p24)].get<decltype(MECACELL_K(p24))>();          \
			if (o.count(MECACELL_SK(p25)))                                                     \
				MECACELL_K(p25) = o[MECACELL_SK(p25)].get<decltype(MECACELL_K(p25))>();          \
			if (o.count(MECACELL_SK(p26)))                                                     \
				MECACELL_K(p26) = o[MECACELL_SK(p26)].get<decltype(MECACELL_K(p26))>();          \
		}                                                                                    \
		json to_json() const {                                                               \
			auto b = BType::to_json();                                                         \
			auto r = to_jsonL(*this);                                                          \
			if (!b.empty()) r.update(b);                                                       \
			return r;                                                                          \
		}                                                                                    \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {          \
			return s.saveJ(                                                                    \
			    MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1), s.MECACELL_K(p1),          \
			    MECACELL_SK(p2), s.MECACELL_K(p2), MECACELL_SK(p3), s.MECACELL_K(p3),          \
			    MECACELL_SK(p4), s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),          \
			    MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7), s.MECACELL_K(p7),          \
			    MECACELL_SK(p8), s.MECACELL_K(p8), MECACELL_SK(p9), s.MECACELL_K(p9),          \
			    MECACELL_SK(p10), s.MECACELL_K(p10), MECACELL_SK(p11), s.MECACELL_K(p11),      \
			    MECACELL_SK(p12), s.MECACELL_K(p12), MECACELL_SK(p13), s.MECACELL_K(p13),      \
			    MECACELL_SK(p14), s.MECACELL_K(p14), MECACELL_SK(p15), s.MECACELL_K(p15),      \
			    MECACELL_SK(p16), s.MECACELL_K(p16), MECACELL_SK(p17), s.MECACELL_K(p17),      \
			    MECACELL_SK(p18), s.MECACELL_K(p18), MECACELL_SK(p19), s.MECACELL_K(p19),      \
			    MECACELL_SK(p20), s.MECACELL_K(p20), MECACELL_SK(p21), s.MECACELL_K(p21),      \
			    MECACELL_SK(p22), s.MECACELL_K(p22), MECACELL_SK(p23), s.MECACELL_K(p23),      \
			    MECACELL_SK(p24), s.MECACELL_K(p24), MECACELL_SK(p25), s.MECACELL_K(p25),      \
			    MECACELL_SK(p26), s.MECACELL_K(p26));                                          \
		};                                                                                   \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); };      \
	};                                                                                     \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE28(S, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, \
                     v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26)         \
	EXPORTABLE_INHERIT29(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,   \
	                     v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23,  \
	                     v24, v25, v26)

#define EXPORTABLE_INHERIT30(S, Base, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11,  \
                             p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, \
                             p24, p25, p26, p27)                                         \
	struct Exportable : ExportableType<Base>::type {                                       \
		using BType = ExportableType<Base>::type;                                            \
		using json = nlohmann::json;                                                         \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);           \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);           \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);           \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);           \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);           \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);           \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);           \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);           \
		typename ExportableType<decltype(S::MECACELL_V(p8))>::type MECACELL_K(p8);           \
		typename ExportableType<decltype(S::MECACELL_V(p9))>::type MECACELL_K(p9);           \
		typename ExportableType<decltype(S::MECACELL_V(p10))>::type MECACELL_K(p10);         \
		typename ExportableType<decltype(S::MECACELL_V(p11))>::type MECACELL_K(p11);         \
		typename ExportableType<decltype(S::MECACELL_V(p12))>::type MECACELL_K(p12);         \
		typename ExportableType<decltype(S::MECACELL_V(p13))>::type MECACELL_K(p13);         \
		typename ExportableType<decltype(S::MECACELL_V(p14))>::type MECACELL_K(p14);         \
		typename ExportableType<decltype(S::MECACELL_V(p15))>::type MECACELL_K(p15);         \
		typename ExportableType<decltype(S::MECACELL_V(p16))>::type MECACELL_K(p16);         \
		typename ExportableType<decltype(S::MECACELL_V(p17))>::type MECACELL_K(p17);         \
		typename ExportableType<decltype(S::MECACELL_V(p18))>::type MECACELL_K(p18);         \
		typename ExportableType<decltype(S::MECACELL_V(p19))>::type MECACELL_K(p19);         \
		typename ExportableType<decltype(S::MECACELL_V(p20))>::type MECACELL_K(p20);         \
		typename ExportableType<decltype(S::MECACELL_V(p21))>::type MECACELL_K(p21);         \
		typename ExportableType<decltype(S::MECACELL_V(p22))>::type MECACELL_K(p22);         \
		typename ExportableType<decltype(S::MECACELL_V(p23))>::type MECACELL_K(p23);         \
		typename ExportableType<decltype(S::MECACELL_V(p24))>::type MECACELL_K(p24);         \
		typename ExportableType<decltype(S::MECACELL_V(p25))>::type MECACELL_K(p25);         \
		typename ExportableType<decltype(S::MECACELL_V(p26))>::type MECACELL_K(p26);         \
		typename ExportableType<decltype(S::MECACELL_V(p27))>::type MECACELL_K(p27);         \
		Exportable() {}                                                                      \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }                \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                       \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                          \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                          \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                          \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                          \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                          \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                          \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                          \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                          \
			ExportableConstruct(MECACELL_K(p8), s.MECACELL_V(p8), s);                          \
			ExportableConstruct(MECACELL_K(p9), s.MECACELL_V(p9), s);                          \
			ExportableConstruct(MECACELL_K(p10), s.MECACELL_V(p10), s);                        \
			ExportableConstruct(MECACELL_K(p11), s.MECACELL_V(p11), s);                        \
			ExportableConstruct(MECACELL_K(p12), s.MECACELL_V(p12), s);                        \
			ExportableConstruct(MECACELL_K(p13), s.MECACELL_V(p13), s);                        \
			ExportableConstruct(MECACELL_K(p14), s.MECACELL_V(p14), s);                        \
			ExportableConstruct(MECACELL_K(p15), s.MECACELL_V(p15), s);                        \
			ExportableConstruct(MECACELL_K(p16), s.MECACELL_V(p16), s);                        \
			ExportableConstruct(MECACELL_K(p17), s.MECACELL_V(p17), s);                        \
			ExportableConstruct(MECACELL_K(p18), s.MECACELL_V(p18), s);                        \
			ExportableConstruct(MECACELL_K(p19), s.MECACELL_V(p19), s);                        \
			ExportableConstruct(MECACELL_K(p20), s.MECACELL_V(p20), s);                        \
			ExportableConstruct(MECACELL_K(p21), s.MECACELL_V(p21), s);                        \
			ExportableConstruct(MECACELL_K(p22), s.MECACELL_V(p22), s);                        \
			ExportableConstruct(MECACELL_K(p23), s.MECACELL_V(p23), s);                        \
			ExportableConstruct(MECACELL_K(p24), s.MECACELL_V(p24), s);                        \
			ExportableConstruct(MECACELL_K(p25), s.MECACELL_V(p25), s);                        \
			ExportableConstruct(MECACELL_K(p26), s.MECACELL_V(p26), s);                        \
			ExportableConstruct(MECACELL_K(p27), s.MECACELL_V(p27), s);                        \
		}                                                                                    \
		void from_json(const json& o) {                                                      \
			if (o.count(MECACELL_SK(p0)))                                                      \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();             \
			if (o.count(MECACELL_SK(p1)))                                                      \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();             \
			if (o.count(MECACELL_SK(p2)))                                                      \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();             \
			if (o.count(MECACELL_SK(p3)))                                                      \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();             \
			if (o.count(MECACELL_SK(p4)))                                                      \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();             \
			if (o.count(MECACELL_SK(p5)))                                                      \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();             \
			if (o.count(MECACELL_SK(p6)))                                                      \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();             \
			if (o.count(MECACELL_SK(p7)))                                                      \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();             \
			if (o.count(MECACELL_SK(p8)))                                                      \
				MECACELL_K(p8) = o[MECACELL_SK(p8)].get<decltype(MECACELL_K(p8))>();             \
			if (o.count(MECACELL_SK(p9)))                                                      \
				MECACELL_K(p9) = o[MECACELL_SK(p9)].get<decltype(MECACELL_K(p9))>();             \
			if (o.count(MECACELL_SK(p10)))                                                     \
				MECACELL_K(p10) = o[MECACELL_SK(p10)].get<decltype(MECACELL_K(p10))>();          \
			if (o.count(MECACELL_SK(p11)))                                                     \
				MECACELL_K(p11) = o[MECACELL_SK(p11)].get<decltype(MECACELL_K(p11))>();          \
			if (o.count(MECACELL_SK(p12)))                                                     \
				MECACELL_K(p12) = o[MECACELL_SK(p12)].get<decltype(MECACELL_K(p12))>();          \
			if (o.count(MECACELL_SK(p13)))                                                     \
				MECACELL_K(p13) = o[MECACELL_SK(p13)].get<decltype(MECACELL_K(p13))>();          \
			if (o.count(MECACELL_SK(p14)))                                                     \
				MECACELL_K(p14) = o[MECACELL_SK(p14)].get<decltype(MECACELL_K(p14))>();          \
			if (o.count(MECACELL_SK(p15)))                                                     \
				MECACELL_K(p15) = o[MECACELL_SK(p15)].get<decltype(MECACELL_K(p15))>();          \
			if (o.count(MECACELL_SK(p16)))                                                     \
				MECACELL_K(p16) = o[MECACELL_SK(p16)].get<decltype(MECACELL_K(p16))>();          \
			if (o.count(MECACELL_SK(p17)))                                                     \
				MECACELL_K(p17) = o[MECACELL_SK(p17)].get<decltype(MECACELL_K(p17))>();          \
			if (o.count(MECACELL_SK(p18)))                                                     \
				MECACELL_K(p18) = o[MECACELL_SK(p18)].get<decltype(MECACELL_K(p18))>();          \
			if (o.count(MECACELL_SK(p19)))                                                     \
				MECACELL_K(p19) = o[MECACELL_SK(p19)].get<decltype(MECACELL_K(p19))>();          \
			if (o.count(MECACELL_SK(p20)))                                                     \
				MECACELL_K(p20) = o[MECACELL_SK(p20)].get<decltype(MECACELL_K(p20))>();          \
			if (o.count(MECACELL_SK(p21)))                                                     \
				MECACELL_K(p21) = o[MECACELL_SK(p21)].get<decltype(MECACELL_K(p21))>();          \
			if (o.count(MECACELL_SK(p22)))                                                     \
				MECACELL_K(p22) = o[MECACELL_SK(p22)].get<decltype(MECACELL_K(p22))>();          \
			if (o.count(MECACELL_SK(p23)))                                                     \
				MECACELL_K(p23) = o[MECACELL_SK(p23)].get<decltype(MECACELL_K(p23))>();          \
			if (o.count(MECACELL_SK(p24)))                                                     \
				MECACELL_K(p24) = o[MECACELL_SK(p24)].get<decltype(MECACELL_K(p24))>();          \
			if (o.count(MECACELL_SK(p25)))                                                     \
				MECACELL_K(p25) = o[MECACELL_SK(p25)].get<decltype(MECACELL_K(p25))>();          \
			if (o.count(MECACELL_SK(p26)))                                                     \
				MECACELL_K(p26) = o[MECACELL_SK(p26)].get<decltype(MECACELL_K(p26))>();          \
			if (o.count(MECACELL_SK(p27)))                                                     \
				MECACELL_K(p27) = o[MECACELL_SK(p27)].get<decltype(MECACELL_K(p27))>();          \
		}                                                                                    \
		json to_json() const {                                                               \
			auto b = BType::to_json();                                                         \
			auto r = to_jsonL(*this);                                                          \
			if (!b.empty()) r.update(b);                                                       \
			return r;                                                                          \
		}                                                                                    \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {          \
			return s.saveJ(                                                                    \
			    MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1), s.MECACELL_K(p1),          \
			    MECACELL_SK(p2), s.MECACELL_K(p2), MECACELL_SK(p3), s.MECACELL_K(p3),          \
			    MECACELL_SK(p4), s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),          \
			    MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7), s.MECACELL_K(p7),          \
			    MECACELL_SK(p8), s.MECACELL_K(p8), MECACELL_SK(p9), s.MECACELL_K(p9),          \
			    MECACELL_SK(p10), s.MECACELL_K(p10), MECACELL_SK(p11), s.MECACELL_K(p11),      \
			    MECACELL_SK(p12), s.MECACELL_K(p12), MECACELL_SK(p13), s.MECACELL_K(p13),      \
			    MECACELL_SK(p14), s.MECACELL_K(p14), MECACELL_SK(p15), s.MECACELL_K(p15),      \
			    MECACELL_SK(p16), s.MECACELL_K(p16), MECACELL_SK(p17), s.MECACELL_K(p17),      \
			    MECACELL_SK(p18), s.MECACELL_K(p18), MECACELL_SK(p19), s.MECACELL_K(p19),      \
			    MECACELL_SK(p20), s.MECACELL_K(p20), MECACELL_SK(p21), s.MECACELL_K(p21),      \
			    MECACELL_SK(p22), s.MECACELL_K(p22), MECACELL_SK(p23), s.MECACELL_K(p23),      \
			    MECACELL_SK(p24), s.MECACELL_K(p24), MECACELL_SK(p25), s.MECACELL_K(p25),      \
			    MECACELL_SK(p26), s.MECACELL_K(p26), MECACELL_SK(p27), s.MECACELL_K(p27));     \
		};                                                                                   \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); };      \
	};                                                                                     \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE29(S, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, \
                     v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27)    \
	EXPORTABLE_INHERIT30(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,   \
	                     v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23,  \
	                     v24, v25, v26, v27)

#define EXPORTABLE_INHERIT31(S, Base, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11,  \
                             p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, \
                             p24, p25, p26, p27, p28)                                    \
	struct Exportable : ExportableType<Base>::type {                                       \
		using BType = ExportableType<Base>::type;                                            \
		using json = nlohmann::json;                                                         \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);           \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);           \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);           \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);           \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);           \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);           \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);           \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);           \
		typename ExportableType<decltype(S::MECACELL_V(p8))>::type MECACELL_K(p8);           \
		typename ExportableType<decltype(S::MECACELL_V(p9))>::type MECACELL_K(p9);           \
		typename ExportableType<decltype(S::MECACELL_V(p10))>::type MECACELL_K(p10);         \
		typename ExportableType<decltype(S::MECACELL_V(p11))>::type MECACELL_K(p11);         \
		typename ExportableType<decltype(S::MECACELL_V(p12))>::type MECACELL_K(p12);         \
		typename ExportableType<decltype(S::MECACELL_V(p13))>::type MECACELL_K(p13);         \
		typename ExportableType<decltype(S::MECACELL_V(p14))>::type MECACELL_K(p14);         \
		typename ExportableType<decltype(S::MECACELL_V(p15))>::type MECACELL_K(p15);         \
		typename ExportableType<decltype(S::MECACELL_V(p16))>::type MECACELL_K(p16);         \
		typename ExportableType<decltype(S::MECACELL_V(p17))>::type MECACELL_K(p17);         \
		typename ExportableType<decltype(S::MECACELL_V(p18))>::type MECACELL_K(p18);         \
		typename ExportableType<decltype(S::MECACELL_V(p19))>::type MECACELL_K(p19);         \
		typename ExportableType<decltype(S::MECACELL_V(p20))>::type MECACELL_K(p20);         \
		typename ExportableType<decltype(S::MECACELL_V(p21))>::type MECACELL_K(p21);         \
		typename ExportableType<decltype(S::MECACELL_V(p22))>::type MECACELL_K(p22);         \
		typename ExportableType<decltype(S::MECACELL_V(p23))>::type MECACELL_K(p23);         \
		typename ExportableType<decltype(S::MECACELL_V(p24))>::type MECACELL_K(p24);         \
		typename ExportableType<decltype(S::MECACELL_V(p25))>::type MECACELL_K(p25);         \
		typename ExportableType<decltype(S::MECACELL_V(p26))>::type MECACELL_K(p26);         \
		typename ExportableType<decltype(S::MECACELL_V(p27))>::type MECACELL_K(p27);         \
		typename ExportableType<decltype(S::MECACELL_V(p28))>::type MECACELL_K(p28);         \
		Exportable() {}                                                                      \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }                \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                       \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                          \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                          \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                          \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                          \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                          \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                          \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                          \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                          \
			ExportableConstruct(MECACELL_K(p8), s.MECACELL_V(p8), s);                          \
			ExportableConstruct(MECACELL_K(p9), s.MECACELL_V(p9), s);                          \
			ExportableConstruct(MECACELL_K(p10), s.MECACELL_V(p10), s);                        \
			ExportableConstruct(MECACELL_K(p11), s.MECACELL_V(p11), s);                        \
			ExportableConstruct(MECACELL_K(p12), s.MECACELL_V(p12), s);                        \
			ExportableConstruct(MECACELL_K(p13), s.MECACELL_V(p13), s);                        \
			ExportableConstruct(MECACELL_K(p14), s.MECACELL_V(p14), s);                        \
			ExportableConstruct(MECACELL_K(p15), s.MECACELL_V(p15), s);                        \
			ExportableConstruct(MECACELL_K(p16), s.MECACELL_V(p16), s);                        \
			ExportableConstruct(MECACELL_K(p17), s.MECACELL_V(p17), s);                        \
			ExportableConstruct(MECACELL_K(p18), s.MECACELL_V(p18), s);                        \
			ExportableConstruct(MECACELL_K(p19), s.MECACELL_V(p19), s);                        \
			ExportableConstruct(MECACELL_K(p20), s.MECACELL_V(p20), s);                        \
			ExportableConstruct(MECACELL_K(p21), s.MECACELL_V(p21), s);                        \
			ExportableConstruct(MECACELL_K(p22), s.MECACELL_V(p22), s);                        \
			ExportableConstruct(MECACELL_K(p23), s.MECACELL_V(p23), s);                        \
			ExportableConstruct(MECACELL_K(p24), s.MECACELL_V(p24), s);                        \
			ExportableConstruct(MECACELL_K(p25), s.MECACELL_V(p25), s);                        \
			ExportableConstruct(MECACELL_K(p26), s.MECACELL_V(p26), s);                        \
			ExportableConstruct(MECACELL_K(p27), s.MECACELL_V(p27), s);                        \
			ExportableConstruct(MECACELL_K(p28), s.MECACELL_V(p28), s);                        \
		}                                                                                    \
		void from_json(const json& o) {                                                      \
			if (o.count(MECACELL_SK(p0)))                                                      \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();             \
			if (o.count(MECACELL_SK(p1)))                                                      \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();             \
			if (o.count(MECACELL_SK(p2)))                                                      \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();             \
			if (o.count(MECACELL_SK(p3)))                                                      \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();             \
			if (o.count(MECACELL_SK(p4)))                                                      \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();             \
			if (o.count(MECACELL_SK(p5)))                                                      \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();             \
			if (o.count(MECACELL_SK(p6)))                                                      \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();             \
			if (o.count(MECACELL_SK(p7)))                                                      \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();             \
			if (o.count(MECACELL_SK(p8)))                                                      \
				MECACELL_K(p8) = o[MECACELL_SK(p8)].get<decltype(MECACELL_K(p8))>();             \
			if (o.count(MECACELL_SK(p9)))                                                      \
				MECACELL_K(p9) = o[MECACELL_SK(p9)].get<decltype(MECACELL_K(p9))>();             \
			if (o.count(MECACELL_SK(p10)))                                                     \
				MECACELL_K(p10) = o[MECACELL_SK(p10)].get<decltype(MECACELL_K(p10))>();          \
			if (o.count(MECACELL_SK(p11)))                                                     \
				MECACELL_K(p11) = o[MECACELL_SK(p11)].get<decltype(MECACELL_K(p11))>();          \
			if (o.count(MECACELL_SK(p12)))                                                     \
				MECACELL_K(p12) = o[MECACELL_SK(p12)].get<decltype(MECACELL_K(p12))>();          \
			if (o.count(MECACELL_SK(p13)))                                                     \
				MECACELL_K(p13) = o[MECACELL_SK(p13)].get<decltype(MECACELL_K(p13))>();          \
			if (o.count(MECACELL_SK(p14)))                                                     \
				MECACELL_K(p14) = o[MECACELL_SK(p14)].get<decltype(MECACELL_K(p14))>();          \
			if (o.count(MECACELL_SK(p15)))                                                     \
				MECACELL_K(p15) = o[MECACELL_SK(p15)].get<decltype(MECACELL_K(p15))>();          \
			if (o.count(MECACELL_SK(p16)))                                                     \
				MECACELL_K(p16) = o[MECACELL_SK(p16)].get<decltype(MECACELL_K(p16))>();          \
			if (o.count(MECACELL_SK(p17)))                                                     \
				MECACELL_K(p17) = o[MECACELL_SK(p17)].get<decltype(MECACELL_K(p17))>();          \
			if (o.count(MECACELL_SK(p18)))                                                     \
				MECACELL_K(p18) = o[MECACELL_SK(p18)].get<decltype(MECACELL_K(p18))>();          \
			if (o.count(MECACELL_SK(p19)))                                                     \
				MECACELL_K(p19) = o[MECACELL_SK(p19)].get<decltype(MECACELL_K(p19))>();          \
			if (o.count(MECACELL_SK(p20)))                                                     \
				MECACELL_K(p20) = o[MECACELL_SK(p20)].get<decltype(MECACELL_K(p20))>();          \
			if (o.count(MECACELL_SK(p21)))                                                     \
				MECACELL_K(p21) = o[MECACELL_SK(p21)].get<decltype(MECACELL_K(p21))>();          \
			if (o.count(MECACELL_SK(p22)))                                                     \
				MECACELL_K(p22) = o[MECACELL_SK(p22)].get<decltype(MECACELL_K(p22))>();          \
			if (o.count(MECACELL_SK(p23)))                                                     \
				MECACELL_K(p23) = o[MECACELL_SK(p23)].get<decltype(MECACELL_K(p23))>();          \
			if (o.count(MECACELL_SK(p24)))                                                     \
				MECACELL_K(p24) = o[MECACELL_SK(p24)].get<decltype(MECACELL_K(p24))>();          \
			if (o.count(MECACELL_SK(p25)))                                                     \
				MECACELL_K(p25) = o[MECACELL_SK(p25)].get<decltype(MECACELL_K(p25))>();          \
			if (o.count(MECACELL_SK(p26)))                                                     \
				MECACELL_K(p26) = o[MECACELL_SK(p26)].get<decltype(MECACELL_K(p26))>();          \
			if (o.count(MECACELL_SK(p27)))                                                     \
				MECACELL_K(p27) = o[MECACELL_SK(p27)].get<decltype(MECACELL_K(p27))>();          \
			if (o.count(MECACELL_SK(p28)))                                                     \
				MECACELL_K(p28) = o[MECACELL_SK(p28)].get<decltype(MECACELL_K(p28))>();          \
		}                                                                                    \
		json to_json() const {                                                               \
			auto b = BType::to_json();                                                         \
			auto r = to_jsonL(*this);                                                          \
			if (!b.empty()) r.update(b);                                                       \
			return r;                                                                          \
		}                                                                                    \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {          \
			return s.saveJ(                                                                    \
			    MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1), s.MECACELL_K(p1),          \
			    MECACELL_SK(p2), s.MECACELL_K(p2), MECACELL_SK(p3), s.MECACELL_K(p3),          \
			    MECACELL_SK(p4), s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),          \
			    MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7), s.MECACELL_K(p7),          \
			    MECACELL_SK(p8), s.MECACELL_K(p8), MECACELL_SK(p9), s.MECACELL_K(p9),          \
			    MECACELL_SK(p10), s.MECACELL_K(p10), MECACELL_SK(p11), s.MECACELL_K(p11),      \
			    MECACELL_SK(p12), s.MECACELL_K(p12), MECACELL_SK(p13), s.MECACELL_K(p13),      \
			    MECACELL_SK(p14), s.MECACELL_K(p14), MECACELL_SK(p15), s.MECACELL_K(p15),      \
			    MECACELL_SK(p16), s.MECACELL_K(p16), MECACELL_SK(p17), s.MECACELL_K(p17),      \
			    MECACELL_SK(p18), s.MECACELL_K(p18), MECACELL_SK(p19), s.MECACELL_K(p19),      \
			    MECACELL_SK(p20), s.MECACELL_K(p20), MECACELL_SK(p21), s.MECACELL_K(p21),      \
			    MECACELL_SK(p22), s.MECACELL_K(p22), MECACELL_SK(p23), s.MECACELL_K(p23),      \
			    MECACELL_SK(p24), s.MECACELL_K(p24), MECACELL_SK(p25), s.MECACELL_K(p25),      \
			    MECACELL_SK(p26), s.MECACELL_K(p26), MECACELL_SK(p27), s.MECACELL_K(p27),      \
			    MECACELL_SK(p28), s.MECACELL_K(p28));                                          \
		};                                                                                   \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); };      \
	};                                                                                     \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE30(S, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, \
                     v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27,    \
                     v28)                                                                \
	EXPORTABLE_INHERIT31(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,   \
	                     v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23,  \
	                     v24, v25, v26, v27, v28)

#define EXPORTABLE_INHERIT32(S, Base, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11,  \
                             p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, \
                             p24, p25, p26, p27, p28, p29)                               \
	struct Exportable : ExportableType<Base>::type {                                       \
		using BType = ExportableType<Base>::type;                                            \
		using json = nlohmann::json;                                                         \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);           \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);           \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);           \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);           \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);           \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);           \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);           \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);           \
		typename ExportableType<decltype(S::MECACELL_V(p8))>::type MECACELL_K(p8);           \
		typename ExportableType<decltype(S::MECACELL_V(p9))>::type MECACELL_K(p9);           \
		typename ExportableType<decltype(S::MECACELL_V(p10))>::type MECACELL_K(p10);         \
		typename ExportableType<decltype(S::MECACELL_V(p11))>::type MECACELL_K(p11);         \
		typename ExportableType<decltype(S::MECACELL_V(p12))>::type MECACELL_K(p12);         \
		typename ExportableType<decltype(S::MECACELL_V(p13))>::type MECACELL_K(p13);         \
		typename ExportableType<decltype(S::MECACELL_V(p14))>::type MECACELL_K(p14);         \
		typename ExportableType<decltype(S::MECACELL_V(p15))>::type MECACELL_K(p15);         \
		typename ExportableType<decltype(S::MECACELL_V(p16))>::type MECACELL_K(p16);         \
		typename ExportableType<decltype(S::MECACELL_V(p17))>::type MECACELL_K(p17);         \
		typename ExportableType<decltype(S::MECACELL_V(p18))>::type MECACELL_K(p18);         \
		typename ExportableType<decltype(S::MECACELL_V(p19))>::type MECACELL_K(p19);         \
		typename ExportableType<decltype(S::MECACELL_V(p20))>::type MECACELL_K(p20);         \
		typename ExportableType<decltype(S::MECACELL_V(p21))>::type MECACELL_K(p21);         \
		typename ExportableType<decltype(S::MECACELL_V(p22))>::type MECACELL_K(p22);         \
		typename ExportableType<decltype(S::MECACELL_V(p23))>::type MECACELL_K(p23);         \
		typename ExportableType<decltype(S::MECACELL_V(p24))>::type MECACELL_K(p24);         \
		typename ExportableType<decltype(S::MECACELL_V(p25))>::type MECACELL_K(p25);         \
		typename ExportableType<decltype(S::MECACELL_V(p26))>::type MECACELL_K(p26);         \
		typename ExportableType<decltype(S::MECACELL_V(p27))>::type MECACELL_K(p27);         \
		typename ExportableType<decltype(S::MECACELL_V(p28))>::type MECACELL_K(p28);         \
		typename ExportableType<decltype(S::MECACELL_V(p29))>::type MECACELL_K(p29);         \
		Exportable() {}                                                                      \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }                \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                       \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                          \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                          \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                          \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                          \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                          \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                          \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                          \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                          \
			ExportableConstruct(MECACELL_K(p8), s.MECACELL_V(p8), s);                          \
			ExportableConstruct(MECACELL_K(p9), s.MECACELL_V(p9), s);                          \
			ExportableConstruct(MECACELL_K(p10), s.MECACELL_V(p10), s);                        \
			ExportableConstruct(MECACELL_K(p11), s.MECACELL_V(p11), s);                        \
			ExportableConstruct(MECACELL_K(p12), s.MECACELL_V(p12), s);                        \
			ExportableConstruct(MECACELL_K(p13), s.MECACELL_V(p13), s);                        \
			ExportableConstruct(MECACELL_K(p14), s.MECACELL_V(p14), s);                        \
			ExportableConstruct(MECACELL_K(p15), s.MECACELL_V(p15), s);                        \
			ExportableConstruct(MECACELL_K(p16), s.MECACELL_V(p16), s);                        \
			ExportableConstruct(MECACELL_K(p17), s.MECACELL_V(p17), s);                        \
			ExportableConstruct(MECACELL_K(p18), s.MECACELL_V(p18), s);                        \
			ExportableConstruct(MECACELL_K(p19), s.MECACELL_V(p19), s);                        \
			ExportableConstruct(MECACELL_K(p20), s.MECACELL_V(p20), s);                        \
			ExportableConstruct(MECACELL_K(p21), s.MECACELL_V(p21), s);                        \
			ExportableConstruct(MECACELL_K(p22), s.MECACELL_V(p22), s);                        \
			ExportableConstruct(MECACELL_K(p23), s.MECACELL_V(p23), s);                        \
			ExportableConstruct(MECACELL_K(p24), s.MECACELL_V(p24), s);                        \
			ExportableConstruct(MECACELL_K(p25), s.MECACELL_V(p25), s);                        \
			ExportableConstruct(MECACELL_K(p26), s.MECACELL_V(p26), s);                        \
			ExportableConstruct(MECACELL_K(p27), s.MECACELL_V(p27), s);                        \
			ExportableConstruct(MECACELL_K(p28), s.MECACELL_V(p28), s);                        \
			ExportableConstruct(MECACELL_K(p29), s.MECACELL_V(p29), s);                        \
		}                                                                                    \
		void from_json(const json& o) {                                                      \
			if (o.count(MECACELL_SK(p0)))                                                      \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();             \
			if (o.count(MECACELL_SK(p1)))                                                      \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();             \
			if (o.count(MECACELL_SK(p2)))                                                      \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();             \
			if (o.count(MECACELL_SK(p3)))                                                      \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();             \
			if (o.count(MECACELL_SK(p4)))                                                      \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();             \
			if (o.count(MECACELL_SK(p5)))                                                      \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();             \
			if (o.count(MECACELL_SK(p6)))                                                      \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();             \
			if (o.count(MECACELL_SK(p7)))                                                      \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();             \
			if (o.count(MECACELL_SK(p8)))                                                      \
				MECACELL_K(p8) = o[MECACELL_SK(p8)].get<decltype(MECACELL_K(p8))>();             \
			if (o.count(MECACELL_SK(p9)))                                                      \
				MECACELL_K(p9) = o[MECACELL_SK(p9)].get<decltype(MECACELL_K(p9))>();             \
			if (o.count(MECACELL_SK(p10)))                                                     \
				MECACELL_K(p10) = o[MECACELL_SK(p10)].get<decltype(MECACELL_K(p10))>();          \
			if (o.count(MECACELL_SK(p11)))                                                     \
				MECACELL_K(p11) = o[MECACELL_SK(p11)].get<decltype(MECACELL_K(p11))>();          \
			if (o.count(MECACELL_SK(p12)))                                                     \
				MECACELL_K(p12) = o[MECACELL_SK(p12)].get<decltype(MECACELL_K(p12))>();          \
			if (o.count(MECACELL_SK(p13)))                                                     \
				MECACELL_K(p13) = o[MECACELL_SK(p13)].get<decltype(MECACELL_K(p13))>();          \
			if (o.count(MECACELL_SK(p14)))                                                     \
				MECACELL_K(p14) = o[MECACELL_SK(p14)].get<decltype(MECACELL_K(p14))>();          \
			if (o.count(MECACELL_SK(p15)))                                                     \
				MECACELL_K(p15) = o[MECACELL_SK(p15)].get<decltype(MECACELL_K(p15))>();          \
			if (o.count(MECACELL_SK(p16)))                                                     \
				MECACELL_K(p16) = o[MECACELL_SK(p16)].get<decltype(MECACELL_K(p16))>();          \
			if (o.count(MECACELL_SK(p17)))                                                     \
				MECACELL_K(p17) = o[MECACELL_SK(p17)].get<decltype(MECACELL_K(p17))>();          \
			if (o.count(MECACELL_SK(p18)))                                                     \
				MECACELL_K(p18) = o[MECACELL_SK(p18)].get<decltype(MECACELL_K(p18))>();          \
			if (o.count(MECACELL_SK(p19)))                                                     \
				MECACELL_K(p19) = o[MECACELL_SK(p19)].get<decltype(MECACELL_K(p19))>();          \
			if (o.count(MECACELL_SK(p20)))                                                     \
				MECACELL_K(p20) = o[MECACELL_SK(p20)].get<decltype(MECACELL_K(p20))>();          \
			if (o.count(MECACELL_SK(p21)))                                                     \
				MECACELL_K(p21) = o[MECACELL_SK(p21)].get<decltype(MECACELL_K(p21))>();          \
			if (o.count(MECACELL_SK(p22)))                                                     \
				MECACELL_K(p22) = o[MECACELL_SK(p22)].get<decltype(MECACELL_K(p22))>();          \
			if (o.count(MECACELL_SK(p23)))                                                     \
				MECACELL_K(p23) = o[MECACELL_SK(p23)].get<decltype(MECACELL_K(p23))>();          \
			if (o.count(MECACELL_SK(p24)))                                                     \
				MECACELL_K(p24) = o[MECACELL_SK(p24)].get<decltype(MECACELL_K(p24))>();          \
			if (o.count(MECACELL_SK(p25)))                                                     \
				MECACELL_K(p25) = o[MECACELL_SK(p25)].get<decltype(MECACELL_K(p25))>();          \
			if (o.count(MECACELL_SK(p26)))                                                     \
				MECACELL_K(p26) = o[MECACELL_SK(p26)].get<decltype(MECACELL_K(p26))>();          \
			if (o.count(MECACELL_SK(p27)))                                                     \
				MECACELL_K(p27) = o[MECACELL_SK(p27)].get<decltype(MECACELL_K(p27))>();          \
			if (o.count(MECACELL_SK(p28)))                                                     \
				MECACELL_K(p28) = o[MECACELL_SK(p28)].get<decltype(MECACELL_K(p28))>();          \
			if (o.count(MECACELL_SK(p29)))                                                     \
				MECACELL_K(p29) = o[MECACELL_SK(p29)].get<decltype(MECACELL_K(p29))>();          \
		}                                                                                    \
		json to_json() const {                                                               \
			auto b = BType::to_json();                                                         \
			auto r = to_jsonL(*this);                                                          \
			if (!b.empty()) r.update(b);                                                       \
			return r;                                                                          \
		}                                                                                    \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {          \
			return s.saveJ(                                                                    \
			    MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1), s.MECACELL_K(p1),          \
			    MECACELL_SK(p2), s.MECACELL_K(p2), MECACELL_SK(p3), s.MECACELL_K(p3),          \
			    MECACELL_SK(p4), s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),          \
			    MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7), s.MECACELL_K(p7),          \
			    MECACELL_SK(p8), s.MECACELL_K(p8), MECACELL_SK(p9), s.MECACELL_K(p9),          \
			    MECACELL_SK(p10), s.MECACELL_K(p10), MECACELL_SK(p11), s.MECACELL_K(p11),      \
			    MECACELL_SK(p12), s.MECACELL_K(p12), MECACELL_SK(p13), s.MECACELL_K(p13),      \
			    MECACELL_SK(p14), s.MECACELL_K(p14), MECACELL_SK(p15), s.MECACELL_K(p15),      \
			    MECACELL_SK(p16), s.MECACELL_K(p16), MECACELL_SK(p17), s.MECACELL_K(p17),      \
			    MECACELL_SK(p18), s.MECACELL_K(p18), MECACELL_SK(p19), s.MECACELL_K(p19),      \
			    MECACELL_SK(p20), s.MECACELL_K(p20), MECACELL_SK(p21), s.MECACELL_K(p21),      \
			    MECACELL_SK(p22), s.MECACELL_K(p22), MECACELL_SK(p23), s.MECACELL_K(p23),      \
			    MECACELL_SK(p24), s.MECACELL_K(p24), MECACELL_SK(p25), s.MECACELL_K(p25),      \
			    MECACELL_SK(p26), s.MECACELL_K(p26), MECACELL_SK(p27), s.MECACELL_K(p27),      \
			    MECACELL_SK(p28), s.MECACELL_K(p28), MECACELL_SK(p29), s.MECACELL_K(p29));     \
		};                                                                                   \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); };      \
	};                                                                                     \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE31(S, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, \
                     v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27,    \
                     v28, v29)                                                           \
	EXPORTABLE_INHERIT32(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,   \
	                     v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23,  \
	                     v24, v25, v26, v27, v28, v29)

#define EXPORTABLE_INHERIT33(S, Base, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11,  \
                             p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, \
                             p24, p25, p26, p27, p28, p29, p30)                          \
	struct Exportable : ExportableType<Base>::type {                                       \
		using BType = ExportableType<Base>::type;                                            \
		using json = nlohmann::json;                                                         \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);           \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);           \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);           \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);           \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);           \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);           \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);           \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);           \
		typename ExportableType<decltype(S::MECACELL_V(p8))>::type MECACELL_K(p8);           \
		typename ExportableType<decltype(S::MECACELL_V(p9))>::type MECACELL_K(p9);           \
		typename ExportableType<decltype(S::MECACELL_V(p10))>::type MECACELL_K(p10);         \
		typename ExportableType<decltype(S::MECACELL_V(p11))>::type MECACELL_K(p11);         \
		typename ExportableType<decltype(S::MECACELL_V(p12))>::type MECACELL_K(p12);         \
		typename ExportableType<decltype(S::MECACELL_V(p13))>::type MECACELL_K(p13);         \
		typename ExportableType<decltype(S::MECACELL_V(p14))>::type MECACELL_K(p14);         \
		typename ExportableType<decltype(S::MECACELL_V(p15))>::type MECACELL_K(p15);         \
		typename ExportableType<decltype(S::MECACELL_V(p16))>::type MECACELL_K(p16);         \
		typename ExportableType<decltype(S::MECACELL_V(p17))>::type MECACELL_K(p17);         \
		typename ExportableType<decltype(S::MECACELL_V(p18))>::type MECACELL_K(p18);         \
		typename ExportableType<decltype(S::MECACELL_V(p19))>::type MECACELL_K(p19);         \
		typename ExportableType<decltype(S::MECACELL_V(p20))>::type MECACELL_K(p20);         \
		typename ExportableType<decltype(S::MECACELL_V(p21))>::type MECACELL_K(p21);         \
		typename ExportableType<decltype(S::MECACELL_V(p22))>::type MECACELL_K(p22);         \
		typename ExportableType<decltype(S::MECACELL_V(p23))>::type MECACELL_K(p23);         \
		typename ExportableType<decltype(S::MECACELL_V(p24))>::type MECACELL_K(p24);         \
		typename ExportableType<decltype(S::MECACELL_V(p25))>::type MECACELL_K(p25);         \
		typename ExportableType<decltype(S::MECACELL_V(p26))>::type MECACELL_K(p26);         \
		typename ExportableType<decltype(S::MECACELL_V(p27))>::type MECACELL_K(p27);         \
		typename ExportableType<decltype(S::MECACELL_V(p28))>::type MECACELL_K(p28);         \
		typename ExportableType<decltype(S::MECACELL_V(p29))>::type MECACELL_K(p29);         \
		typename ExportableType<decltype(S::MECACELL_V(p30))>::type MECACELL_K(p30);         \
		Exportable() {}                                                                      \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }                \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                       \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                          \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                          \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                          \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                          \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                          \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                          \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                          \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                          \
			ExportableConstruct(MECACELL_K(p8), s.MECACELL_V(p8), s);                          \
			ExportableConstruct(MECACELL_K(p9), s.MECACELL_V(p9), s);                          \
			ExportableConstruct(MECACELL_K(p10), s.MECACELL_V(p10), s);                        \
			ExportableConstruct(MECACELL_K(p11), s.MECACELL_V(p11), s);                        \
			ExportableConstruct(MECACELL_K(p12), s.MECACELL_V(p12), s);                        \
			ExportableConstruct(MECACELL_K(p13), s.MECACELL_V(p13), s);                        \
			ExportableConstruct(MECACELL_K(p14), s.MECACELL_V(p14), s);                        \
			ExportableConstruct(MECACELL_K(p15), s.MECACELL_V(p15), s);                        \
			ExportableConstruct(MECACELL_K(p16), s.MECACELL_V(p16), s);                        \
			ExportableConstruct(MECACELL_K(p17), s.MECACELL_V(p17), s);                        \
			ExportableConstruct(MECACELL_K(p18), s.MECACELL_V(p18), s);                        \
			ExportableConstruct(MECACELL_K(p19), s.MECACELL_V(p19), s);                        \
			ExportableConstruct(MECACELL_K(p20), s.MECACELL_V(p20), s);                        \
			ExportableConstruct(MECACELL_K(p21), s.MECACELL_V(p21), s);                        \
			ExportableConstruct(MECACELL_K(p22), s.MECACELL_V(p22), s);                        \
			ExportableConstruct(MECACELL_K(p23), s.MECACELL_V(p23), s);                        \
			ExportableConstruct(MECACELL_K(p24), s.MECACELL_V(p24), s);                        \
			ExportableConstruct(MECACELL_K(p25), s.MECACELL_V(p25), s);                        \
			ExportableConstruct(MECACELL_K(p26), s.MECACELL_V(p26), s);                        \
			ExportableConstruct(MECACELL_K(p27), s.MECACELL_V(p27), s);                        \
			ExportableConstruct(MECACELL_K(p28), s.MECACELL_V(p28), s);                        \
			ExportableConstruct(MECACELL_K(p29), s.MECACELL_V(p29), s);                        \
			ExportableConstruct(MECACELL_K(p30), s.MECACELL_V(p30), s);                        \
		}                                                                                    \
		void from_json(const json& o) {                                                      \
			if (o.count(MECACELL_SK(p0)))                                                      \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();             \
			if (o.count(MECACELL_SK(p1)))                                                      \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();             \
			if (o.count(MECACELL_SK(p2)))                                                      \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();             \
			if (o.count(MECACELL_SK(p3)))                                                      \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();             \
			if (o.count(MECACELL_SK(p4)))                                                      \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();             \
			if (o.count(MECACELL_SK(p5)))                                                      \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();             \
			if (o.count(MECACELL_SK(p6)))                                                      \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();             \
			if (o.count(MECACELL_SK(p7)))                                                      \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();             \
			if (o.count(MECACELL_SK(p8)))                                                      \
				MECACELL_K(p8) = o[MECACELL_SK(p8)].get<decltype(MECACELL_K(p8))>();             \
			if (o.count(MECACELL_SK(p9)))                                                      \
				MECACELL_K(p9) = o[MECACELL_SK(p9)].get<decltype(MECACELL_K(p9))>();             \
			if (o.count(MECACELL_SK(p10)))                                                     \
				MECACELL_K(p10) = o[MECACELL_SK(p10)].get<decltype(MECACELL_K(p10))>();          \
			if (o.count(MECACELL_SK(p11)))                                                     \
				MECACELL_K(p11) = o[MECACELL_SK(p11)].get<decltype(MECACELL_K(p11))>();          \
			if (o.count(MECACELL_SK(p12)))                                                     \
				MECACELL_K(p12) = o[MECACELL_SK(p12)].get<decltype(MECACELL_K(p12))>();          \
			if (o.count(MECACELL_SK(p13)))                                                     \
				MECACELL_K(p13) = o[MECACELL_SK(p13)].get<decltype(MECACELL_K(p13))>();          \
			if (o.count(MECACELL_SK(p14)))                                                     \
				MECACELL_K(p14) = o[MECACELL_SK(p14)].get<decltype(MECACELL_K(p14))>();          \
			if (o.count(MECACELL_SK(p15)))                                                     \
				MECACELL_K(p15) = o[MECACELL_SK(p15)].get<decltype(MECACELL_K(p15))>();          \
			if (o.count(MECACELL_SK(p16)))                                                     \
				MECACELL_K(p16) = o[MECACELL_SK(p16)].get<decltype(MECACELL_K(p16))>();          \
			if (o.count(MECACELL_SK(p17)))                                                     \
				MECACELL_K(p17) = o[MECACELL_SK(p17)].get<decltype(MECACELL_K(p17))>();          \
			if (o.count(MECACELL_SK(p18)))                                                     \
				MECACELL_K(p18) = o[MECACELL_SK(p18)].get<decltype(MECACELL_K(p18))>();          \
			if (o.count(MECACELL_SK(p19)))                                                     \
				MECACELL_K(p19) = o[MECACELL_SK(p19)].get<decltype(MECACELL_K(p19))>();          \
			if (o.count(MECACELL_SK(p20)))                                                     \
				MECACELL_K(p20) = o[MECACELL_SK(p20)].get<decltype(MECACELL_K(p20))>();          \
			if (o.count(MECACELL_SK(p21)))                                                     \
				MECACELL_K(p21) = o[MECACELL_SK(p21)].get<decltype(MECACELL_K(p21))>();          \
			if (o.count(MECACELL_SK(p22)))                                                     \
				MECACELL_K(p22) = o[MECACELL_SK(p22)].get<decltype(MECACELL_K(p22))>();          \
			if (o.count(MECACELL_SK(p23)))                                                     \
				MECACELL_K(p23) = o[MECACELL_SK(p23)].get<decltype(MECACELL_K(p23))>();          \
			if (o.count(MECACELL_SK(p24)))                                                     \
				MECACELL_K(p24) = o[MECACELL_SK(p24)].get<decltype(MECACELL_K(p24))>();          \
			if (o.count(MECACELL_SK(p25)))                                                     \
				MECACELL_K(p25) = o[MECACELL_SK(p25)].get<decltype(MECACELL_K(p25))>();          \
			if (o.count(MECACELL_SK(p26)))                                                     \
				MECACELL_K(p26) = o[MECACELL_SK(p26)].get<decltype(MECACELL_K(p26))>();          \
			if (o.count(MECACELL_SK(p27)))                                                     \
				MECACELL_K(p27) = o[MECACELL_SK(p27)].get<decltype(MECACELL_K(p27))>();          \
			if (o.count(MECACELL_SK(p28)))                                                     \
				MECACELL_K(p28) = o[MECACELL_SK(p28)].get<decltype(MECACELL_K(p28))>();          \
			if (o.count(MECACELL_SK(p29)))                                                     \
				MECACELL_K(p29) = o[MECACELL_SK(p29)].get<decltype(MECACELL_K(p29))>();          \
			if (o.count(MECACELL_SK(p30)))                                                     \
				MECACELL_K(p30) = o[MECACELL_SK(p30)].get<decltype(MECACELL_K(p30))>();          \
		}                                                                                    \
		json to_json() const {                                                               \
			auto b = BType::to_json();                                                         \
			auto r = to_jsonL(*this);                                                          \
			if (!b.empty()) r.update(b);                                                       \
			return r;                                                                          \
		}                                                                                    \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {          \
			return s.saveJ(                                                                    \
			    MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1), s.MECACELL_K(p1),          \
			    MECACELL_SK(p2), s.MECACELL_K(p2), MECACELL_SK(p3), s.MECACELL_K(p3),          \
			    MECACELL_SK(p4), s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),          \
			    MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7), s.MECACELL_K(p7),          \
			    MECACELL_SK(p8), s.MECACELL_K(p8), MECACELL_SK(p9), s.MECACELL_K(p9),          \
			    MECACELL_SK(p10), s.MECACELL_K(p10), MECACELL_SK(p11), s.MECACELL_K(p11),      \
			    MECACELL_SK(p12), s.MECACELL_K(p12), MECACELL_SK(p13), s.MECACELL_K(p13),      \
			    MECACELL_SK(p14), s.MECACELL_K(p14), MECACELL_SK(p15), s.MECACELL_K(p15),      \
			    MECACELL_SK(p16), s.MECACELL_K(p16), MECACELL_SK(p17), s.MECACELL_K(p17),      \
			    MECACELL_SK(p18), s.MECACELL_K(p18), MECACELL_SK(p19), s.MECACELL_K(p19),      \
			    MECACELL_SK(p20), s.MECACELL_K(p20), MECACELL_SK(p21), s.MECACELL_K(p21),      \
			    MECACELL_SK(p22), s.MECACELL_K(p22), MECACELL_SK(p23), s.MECACELL_K(p23),      \
			    MECACELL_SK(p24), s.MECACELL_K(p24), MECACELL_SK(p25), s.MECACELL_K(p25),      \
			    MECACELL_SK(p26), s.MECACELL_K(p26), MECACELL_SK(p27), s.MECACELL_K(p27),      \
			    MECACELL_SK(p28), s.MECACELL_K(p28), MECACELL_SK(p29), s.MECACELL_K(p29),      \
			    MECACELL_SK(p30), s.MECACELL_K(p30));                                          \
		};                                                                                   \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); };      \
	};                                                                                     \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE32(S, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, \
                     v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27,    \
                     v28, v29, v30)                                                      \
	EXPORTABLE_INHERIT33(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,   \
	                     v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23,  \
	                     v24, v25, v26, v27, v28, v29, v30)

#define EXPORTABLE_INHERIT34(S, Base, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11,  \
                             p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, \
                             p24, p25, p26, p27, p28, p29, p30, p31)                     \
	struct Exportable : ExportableType<Base>::type {                                       \
		using BType = ExportableType<Base>::type;                                            \
		using json = nlohmann::json;                                                         \
		typename ExportableType<decltype(S::MECACELL_V(p0))>::type MECACELL_K(p0);           \
		typename ExportableType<decltype(S::MECACELL_V(p1))>::type MECACELL_K(p1);           \
		typename ExportableType<decltype(S::MECACELL_V(p2))>::type MECACELL_K(p2);           \
		typename ExportableType<decltype(S::MECACELL_V(p3))>::type MECACELL_K(p3);           \
		typename ExportableType<decltype(S::MECACELL_V(p4))>::type MECACELL_K(p4);           \
		typename ExportableType<decltype(S::MECACELL_V(p5))>::type MECACELL_K(p5);           \
		typename ExportableType<decltype(S::MECACELL_V(p6))>::type MECACELL_K(p6);           \
		typename ExportableType<decltype(S::MECACELL_V(p7))>::type MECACELL_K(p7);           \
		typename ExportableType<decltype(S::MECACELL_V(p8))>::type MECACELL_K(p8);           \
		typename ExportableType<decltype(S::MECACELL_V(p9))>::type MECACELL_K(p9);           \
		typename ExportableType<decltype(S::MECACELL_V(p10))>::type MECACELL_K(p10);         \
		typename ExportableType<decltype(S::MECACELL_V(p11))>::type MECACELL_K(p11);         \
		typename ExportableType<decltype(S::MECACELL_V(p12))>::type MECACELL_K(p12);         \
		typename ExportableType<decltype(S::MECACELL_V(p13))>::type MECACELL_K(p13);         \
		typename ExportableType<decltype(S::MECACELL_V(p14))>::type MECACELL_K(p14);         \
		typename ExportableType<decltype(S::MECACELL_V(p15))>::type MECACELL_K(p15);         \
		typename ExportableType<decltype(S::MECACELL_V(p16))>::type MECACELL_K(p16);         \
		typename ExportableType<decltype(S::MECACELL_V(p17))>::type MECACELL_K(p17);         \
		typename ExportableType<decltype(S::MECACELL_V(p18))>::type MECACELL_K(p18);         \
		typename ExportableType<decltype(S::MECACELL_V(p19))>::type MECACELL_K(p19);         \
		typename ExportableType<decltype(S::MECACELL_V(p20))>::type MECACELL_K(p20);         \
		typename ExportableType<decltype(S::MECACELL_V(p21))>::type MECACELL_K(p21);         \
		typename ExportableType<decltype(S::MECACELL_V(p22))>::type MECACELL_K(p22);         \
		typename ExportableType<decltype(S::MECACELL_V(p23))>::type MECACELL_K(p23);         \
		typename ExportableType<decltype(S::MECACELL_V(p24))>::type MECACELL_K(p24);         \
		typename ExportableType<decltype(S::MECACELL_V(p25))>::type MECACELL_K(p25);         \
		typename ExportableType<decltype(S::MECACELL_V(p26))>::type MECACELL_K(p26);         \
		typename ExportableType<decltype(S::MECACELL_V(p27))>::type MECACELL_K(p27);         \
		typename ExportableType<decltype(S::MECACELL_V(p28))>::type MECACELL_K(p28);         \
		typename ExportableType<decltype(S::MECACELL_V(p29))>::type MECACELL_K(p29);         \
		typename ExportableType<decltype(S::MECACELL_V(p30))>::type MECACELL_K(p30);         \
		typename ExportableType<decltype(S::MECACELL_V(p31))>::type MECACELL_K(p31);         \
		Exportable() {}                                                                      \
		Exportable(const std::string& dump) { from_json(json::parse(dump)); }                \
		Exportable(const S& s) : BType(static_cast<const BType&>(s)) {                       \
			ExportableConstruct(MECACELL_K(p0), s.MECACELL_V(p0), s);                          \
			ExportableConstruct(MECACELL_K(p1), s.MECACELL_V(p1), s);                          \
			ExportableConstruct(MECACELL_K(p2), s.MECACELL_V(p2), s);                          \
			ExportableConstruct(MECACELL_K(p3), s.MECACELL_V(p3), s);                          \
			ExportableConstruct(MECACELL_K(p4), s.MECACELL_V(p4), s);                          \
			ExportableConstruct(MECACELL_K(p5), s.MECACELL_V(p5), s);                          \
			ExportableConstruct(MECACELL_K(p6), s.MECACELL_V(p6), s);                          \
			ExportableConstruct(MECACELL_K(p7), s.MECACELL_V(p7), s);                          \
			ExportableConstruct(MECACELL_K(p8), s.MECACELL_V(p8), s);                          \
			ExportableConstruct(MECACELL_K(p9), s.MECACELL_V(p9), s);                          \
			ExportableConstruct(MECACELL_K(p10), s.MECACELL_V(p10), s);                        \
			ExportableConstruct(MECACELL_K(p11), s.MECACELL_V(p11), s);                        \
			ExportableConstruct(MECACELL_K(p12), s.MECACELL_V(p12), s);                        \
			ExportableConstruct(MECACELL_K(p13), s.MECACELL_V(p13), s);                        \
			ExportableConstruct(MECACELL_K(p14), s.MECACELL_V(p14), s);                        \
			ExportableConstruct(MECACELL_K(p15), s.MECACELL_V(p15), s);                        \
			ExportableConstruct(MECACELL_K(p16), s.MECACELL_V(p16), s);                        \
			ExportableConstruct(MECACELL_K(p17), s.MECACELL_V(p17), s);                        \
			ExportableConstruct(MECACELL_K(p18), s.MECACELL_V(p18), s);                        \
			ExportableConstruct(MECACELL_K(p19), s.MECACELL_V(p19), s);                        \
			ExportableConstruct(MECACELL_K(p20), s.MECACELL_V(p20), s);                        \
			ExportableConstruct(MECACELL_K(p21), s.MECACELL_V(p21), s);                        \
			ExportableConstruct(MECACELL_K(p22), s.MECACELL_V(p22), s);                        \
			ExportableConstruct(MECACELL_K(p23), s.MECACELL_V(p23), s);                        \
			ExportableConstruct(MECACELL_K(p24), s.MECACELL_V(p24), s);                        \
			ExportableConstruct(MECACELL_K(p25), s.MECACELL_V(p25), s);                        \
			ExportableConstruct(MECACELL_K(p26), s.MECACELL_V(p26), s);                        \
			ExportableConstruct(MECACELL_K(p27), s.MECACELL_V(p27), s);                        \
			ExportableConstruct(MECACELL_K(p28), s.MECACELL_V(p28), s);                        \
			ExportableConstruct(MECACELL_K(p29), s.MECACELL_V(p29), s);                        \
			ExportableConstruct(MECACELL_K(p30), s.MECACELL_V(p30), s);                        \
			ExportableConstruct(MECACELL_K(p31), s.MECACELL_V(p31), s);                        \
		}                                                                                    \
		void from_json(const json& o) {                                                      \
			if (o.count(MECACELL_SK(p0)))                                                      \
				MECACELL_K(p0) = o[MECACELL_SK(p0)].get<decltype(MECACELL_K(p0))>();             \
			if (o.count(MECACELL_SK(p1)))                                                      \
				MECACELL_K(p1) = o[MECACELL_SK(p1)].get<decltype(MECACELL_K(p1))>();             \
			if (o.count(MECACELL_SK(p2)))                                                      \
				MECACELL_K(p2) = o[MECACELL_SK(p2)].get<decltype(MECACELL_K(p2))>();             \
			if (o.count(MECACELL_SK(p3)))                                                      \
				MECACELL_K(p3) = o[MECACELL_SK(p3)].get<decltype(MECACELL_K(p3))>();             \
			if (o.count(MECACELL_SK(p4)))                                                      \
				MECACELL_K(p4) = o[MECACELL_SK(p4)].get<decltype(MECACELL_K(p4))>();             \
			if (o.count(MECACELL_SK(p5)))                                                      \
				MECACELL_K(p5) = o[MECACELL_SK(p5)].get<decltype(MECACELL_K(p5))>();             \
			if (o.count(MECACELL_SK(p6)))                                                      \
				MECACELL_K(p6) = o[MECACELL_SK(p6)].get<decltype(MECACELL_K(p6))>();             \
			if (o.count(MECACELL_SK(p7)))                                                      \
				MECACELL_K(p7) = o[MECACELL_SK(p7)].get<decltype(MECACELL_K(p7))>();             \
			if (o.count(MECACELL_SK(p8)))                                                      \
				MECACELL_K(p8) = o[MECACELL_SK(p8)].get<decltype(MECACELL_K(p8))>();             \
			if (o.count(MECACELL_SK(p9)))                                                      \
				MECACELL_K(p9) = o[MECACELL_SK(p9)].get<decltype(MECACELL_K(p9))>();             \
			if (o.count(MECACELL_SK(p10)))                                                     \
				MECACELL_K(p10) = o[MECACELL_SK(p10)].get<decltype(MECACELL_K(p10))>();          \
			if (o.count(MECACELL_SK(p11)))                                                     \
				MECACELL_K(p11) = o[MECACELL_SK(p11)].get<decltype(MECACELL_K(p11))>();          \
			if (o.count(MECACELL_SK(p12)))                                                     \
				MECACELL_K(p12) = o[MECACELL_SK(p12)].get<decltype(MECACELL_K(p12))>();          \
			if (o.count(MECACELL_SK(p13)))                                                     \
				MECACELL_K(p13) = o[MECACELL_SK(p13)].get<decltype(MECACELL_K(p13))>();          \
			if (o.count(MECACELL_SK(p14)))                                                     \
				MECACELL_K(p14) = o[MECACELL_SK(p14)].get<decltype(MECACELL_K(p14))>();          \
			if (o.count(MECACELL_SK(p15)))                                                     \
				MECACELL_K(p15) = o[MECACELL_SK(p15)].get<decltype(MECACELL_K(p15))>();          \
			if (o.count(MECACELL_SK(p16)))                                                     \
				MECACELL_K(p16) = o[MECACELL_SK(p16)].get<decltype(MECACELL_K(p16))>();          \
			if (o.count(MECACELL_SK(p17)))                                                     \
				MECACELL_K(p17) = o[MECACELL_SK(p17)].get<decltype(MECACELL_K(p17))>();          \
			if (o.count(MECACELL_SK(p18)))                                                     \
				MECACELL_K(p18) = o[MECACELL_SK(p18)].get<decltype(MECACELL_K(p18))>();          \
			if (o.count(MECACELL_SK(p19)))                                                     \
				MECACELL_K(p19) = o[MECACELL_SK(p19)].get<decltype(MECACELL_K(p19))>();          \
			if (o.count(MECACELL_SK(p20)))                                                     \
				MECACELL_K(p20) = o[MECACELL_SK(p20)].get<decltype(MECACELL_K(p20))>();          \
			if (o.count(MECACELL_SK(p21)))                                                     \
				MECACELL_K(p21) = o[MECACELL_SK(p21)].get<decltype(MECACELL_K(p21))>();          \
			if (o.count(MECACELL_SK(p22)))                                                     \
				MECACELL_K(p22) = o[MECACELL_SK(p22)].get<decltype(MECACELL_K(p22))>();          \
			if (o.count(MECACELL_SK(p23)))                                                     \
				MECACELL_K(p23) = o[MECACELL_SK(p23)].get<decltype(MECACELL_K(p23))>();          \
			if (o.count(MECACELL_SK(p24)))                                                     \
				MECACELL_K(p24) = o[MECACELL_SK(p24)].get<decltype(MECACELL_K(p24))>();          \
			if (o.count(MECACELL_SK(p25)))                                                     \
				MECACELL_K(p25) = o[MECACELL_SK(p25)].get<decltype(MECACELL_K(p25))>();          \
			if (o.count(MECACELL_SK(p26)))                                                     \
				MECACELL_K(p26) = o[MECACELL_SK(p26)].get<decltype(MECACELL_K(p26))>();          \
			if (o.count(MECACELL_SK(p27)))                                                     \
				MECACELL_K(p27) = o[MECACELL_SK(p27)].get<decltype(MECACELL_K(p27))>();          \
			if (o.count(MECACELL_SK(p28)))                                                     \
				MECACELL_K(p28) = o[MECACELL_SK(p28)].get<decltype(MECACELL_K(p28))>();          \
			if (o.count(MECACELL_SK(p29)))                                                     \
				MECACELL_K(p29) = o[MECACELL_SK(p29)].get<decltype(MECACELL_K(p29))>();          \
			if (o.count(MECACELL_SK(p30)))                                                     \
				MECACELL_K(p30) = o[MECACELL_SK(p30)].get<decltype(MECACELL_K(p30))>();          \
			if (o.count(MECACELL_SK(p31)))                                                     \
				MECACELL_K(p31) = o[MECACELL_SK(p31)].get<decltype(MECACELL_K(p31))>();          \
		}                                                                                    \
		json to_json() const {                                                               \
			auto b = BType::to_json();                                                         \
			auto r = to_jsonL(*this);                                                          \
			if (!b.empty()) r.update(b);                                                       \
			return r;                                                                          \
		}                                                                                    \
		std::function<json(const Exportable&)> to_jsonL = [](const Exportable& s) {          \
			return s.saveJ(                                                                    \
			    MECACELL_SK(p0), s.MECACELL_K(p0), MECACELL_SK(p1), s.MECACELL_K(p1),          \
			    MECACELL_SK(p2), s.MECACELL_K(p2), MECACELL_SK(p3), s.MECACELL_K(p3),          \
			    MECACELL_SK(p4), s.MECACELL_K(p4), MECACELL_SK(p5), s.MECACELL_K(p5),          \
			    MECACELL_SK(p6), s.MECACELL_K(p6), MECACELL_SK(p7), s.MECACELL_K(p7),          \
			    MECACELL_SK(p8), s.MECACELL_K(p8), MECACELL_SK(p9), s.MECACELL_K(p9),          \
			    MECACELL_SK(p10), s.MECACELL_K(p10), MECACELL_SK(p11), s.MECACELL_K(p11),      \
			    MECACELL_SK(p12), s.MECACELL_K(p12), MECACELL_SK(p13), s.MECACELL_K(p13),      \
			    MECACELL_SK(p14), s.MECACELL_K(p14), MECACELL_SK(p15), s.MECACELL_K(p15),      \
			    MECACELL_SK(p16), s.MECACELL_K(p16), MECACELL_SK(p17), s.MECACELL_K(p17),      \
			    MECACELL_SK(p18), s.MECACELL_K(p18), MECACELL_SK(p19), s.MECACELL_K(p19),      \
			    MECACELL_SK(p20), s.MECACELL_K(p20), MECACELL_SK(p21), s.MECACELL_K(p21),      \
			    MECACELL_SK(p22), s.MECACELL_K(p22), MECACELL_SK(p23), s.MECACELL_K(p23),      \
			    MECACELL_SK(p24), s.MECACELL_K(p24), MECACELL_SK(p25), s.MECACELL_K(p25),      \
			    MECACELL_SK(p26), s.MECACELL_K(p26), MECACELL_SK(p27), s.MECACELL_K(p27),      \
			    MECACELL_SK(p28), s.MECACELL_K(p28), MECACELL_SK(p29), s.MECACELL_K(p29),      \
			    MECACELL_SK(p30), s.MECACELL_K(p30), MECACELL_SK(p31), s.MECACELL_K(p31));     \
		};                                                                                   \
		std::function<std::string(void)> save = [this]() { return to_json().dump(2); };      \
	};                                                                                     \
	Exportable getExport() const { return Exportable(*this); }

#define EXPORTABLE33(S, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, \
                     v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27,    \
                     v28, v29, v30, v31)                                                 \
	EXPORTABLE_INHERIT34(S, BaseExportable, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,   \
	                     v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23,  \
	                     v24, v25, v26, v27, v28, v29, v30, v31)

template <typename S, typename T> struct ExportableAlias {
	std::function<T(const S&)> getValue;
};

template <typename T> inline T* asPtr(T& obj) { return &obj; }
template <typename T> inline T* asPtr(T* obj) { return obj; }

template <typename T> struct BareType {
	using type = std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>;
};
// TODO : generalize to any std container, remove ref and pointers
template <typename A, typename B, typename S>
void ExportableConstruct(A& a, const B& b, const S&) {
	a = A(*asPtr(b));
}

template <typename A, typename S, typename T>
void ExportableConstruct(A& a, const ExportableAlias<S, T>& alias, const S& s) {
	a = alias.getValue(s);
}

template <typename NonEx, typename Ex, typename S>
void ExportableConstruct(std::vector<Ex>& a, const std::vector<NonEx>& b, const S&) {
	a = std::vector<Ex>();
	a.reserve(b.size());
	for (const auto& i : b) {
		if (asPtr(i) != nullptr) a.emplace_back(Ex(*asPtr(i)));
	}
}

template <typename C> struct ExportableType {
	template <typename T> static constexpr T getType(...) {
		std::cerr << "general type" << std::endl;
	}

	template <typename T> static constexpr typename T::Exportable getType(void*) {
		std::cerr << "exportable type" << std::endl;
	}

	template <class U, typename S, typename T>
	static constexpr T getType(ExportableAlias<S, T>*) {
		std::cerr << "alias type" << std::endl;
	}

	template <class U, template <class, class> class Cont, class T,
	          template <class> class Alloc>
	static constexpr Cont<typename BareType<T>::type::Exportable,
	                      Alloc<typename BareType<T>::type::Exportable>>
	    getType(Cont<T, Alloc<T>>*) {
		std::cerr << "containertype" << std::endl;
	}

	using type = decltype(
	    getType<typename BareType<C>::type>(std::declval<typename BareType<C>::type*>()));

	static void callType() {
		getType<typename BareType<C>::type>(
		    static_cast<typename BareType<C>::type*>(nullptr));
	}
};

struct BaseExportable {
	struct Exportable {
		using json = nlohmann::json;
		template <typename T> using KV = std::pair<std::string, T>;
		Exportable() {}
		template <typename T> Exportable(const T&) {}

		template <typename T, typename... Args>
		json saveJ(const std::string& name, const T& var, Args&&... args) const {
			json o;
			o[name] = var;
			o.update(saveJ(std::forward<Args>(args)...));
			return o;
		}

		template <typename T> json saveJ(const std::string& name, const T& var) const {
			json o;
			o[name] = var;
			return o;
		}
		json to_json() const {
			json o;
			return o;
		};
	};
};

// TODO : generalize to any std container
template <typename T> void from_json(const nlohmann::json& j, T& e) { e.from_json(j); }
template <typename T> void from_json(const nlohmann::json& j, std::vector<T>& e) {
	e = std::vector<T>();
	for (const auto& i : j) {
		T t;
		from_json(i, t);
		e.push_back(t);
	}
}
template <typename T> void to_json(nlohmann::json& j, const T& e) { j = e.to_json(); }
template <typename T> void to_json(nlohmann::json& j, const std::vector<T>& e) {
	j = nlohmann::json::array();
	for (const auto& i : e) {
		nlohmann::json o;
		to_json(o, i);
		j.push_back(o);
	}
}

#endif
