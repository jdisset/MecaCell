#ifndef UNIQUE_VECTOR_HPP
#define UNIQUE_VECTOR_HPP
#include <vector>
template <typename T> class unique_vector {
	// Vector class that ensures no duplicate

	std::vector<T> vec;

 public:
	const std::vector<T> &getUnderlyingVector() const { return vec; }
	void clear() { vec = std::vector<T>(); }
	T &operator[](size_t i) { return vec[i]; }
	template <typename U> void insert(U &&u) {
		if (!isInVector(std::forward<U>(u), vec)) vec.push_back(u);
	}
	template <typename I> void insert(I b, I e) {
		while (b != e) {
			insert(*b);
			++b;
		}
	}
	size_t size() const { return vec.size(); }
	size_t count(const T &t) const { return isInVector(t, vec) ? 1 : 0; }
	void erase(const T &t) { eraseFromVector(t, vec); }
	using const_iterator = typename decltype(vec)::const_iterator;
	using iterator = typename decltype(vec)::iterator;
	const_iterator begin() const { return vec.begin(); }
	const_iterator end() const { return vec.end(); }
	iterator begin() { return vec.begin(); }
	iterator end() { return vec.end(); }
};

#endif
