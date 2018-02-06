#ifndef ORDERED_HASH_MAP_HPP
#define ORDERED_HASH_MAP_HPP
#include <unordered_map>
#include <vector>
namespace MecaCell {
template <typename K, typename V> struct ordered_hash_map {
	// deterministic ordered hash_map
	std::unordered_map<K, size_t> um;
	std::vector<std::pair<K, V>> vec;

	V &operator[](const K &k) {
		if (!um.count(k)) {
			um[k] = vec.size();
			vec.push_back({k, V{}});
		}
		return vec[um[k]].second;
	}

	bool count(const K &k) { return um.count(k); }
	size_t size() const { return vec.size(); }
	V &at(const K &k) { return vec[um.at(k)].second; }
	void erase(const K &k) {
		if (um.count(k)) {
			const size_t id = um[k];
			if (id < vec.size() - 1) {
				vec[id] = std::move(vec.back());
				um[vec[id].first] = id;
			}
			vec.pop_back();
			um.erase(k);
		}
	}
	using const_iterator = typename decltype(vec)::const_iterator;
	using iterator = typename decltype(vec)::iterator;
	const_iterator begin() const { return vec.begin(); }
	const_iterator end() const { return vec.end(); }
	iterator begin() { return vec.begin(); }
	iterator end() { return vec.end(); }
};
}  // namespace MecaCell
#endif
