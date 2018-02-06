#ifndef ORDERED_PAIR
#define ORDERED_PAIR
#include <cassert>
#include <unordered_map>
#include <utility>
namespace MecaCell {
template <typename T> struct ordered_pair {
	T first, second;
	bool operator==(const ordered_pair &other) const {
		return (first->id == other.first->id && second->id == other.second->id);
	}
	template <unsigned int i> T &get() { return i == 0 ? first : second; }
	friend inline std::ostream &operator<<(std::ostream &out, const ordered_pair &v) {
		out << "{" << v.first << ", " << v.second << "}";
		return out;
	}
};
template <typename T> inline ordered_pair<T *> make_ordered_cell_pair(T *a, T *b) {
	if (a->id < b->id) return {a, b};
	return {b, a};
}
template <typename T> inline ordered_pair<T> make_ordered_pair(const T &a, const T &b) {
	if (a.id < b.id) return {a, b};
	return {b, a};
}
}  // namespace MecaCell
namespace std {
template <typename T> struct hash<MecaCell::ordered_pair<T>> {
	size_t operator()(const MecaCell::ordered_pair<T> &x) const {
		// assert(x.first->id < x.second->id);
		// return hash<size_t>()(x.first->id) ^ hash<size_t>()(x.second->id);
		return x.first->id * 10 ^ 9 + x.second->id;
	}
};
}  // namespace std
#endif
