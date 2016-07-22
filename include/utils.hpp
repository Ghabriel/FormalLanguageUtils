#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdlib>
#include <iostream>
#include <type_traits>
#include <unordered_map>

#define TRACE(x) std::cout << (#x) << " = " << (x) << std::endl
#define TRACE_L(x,y) std::cout << (x) << " = " << (y) << std::endl
#define TRACE_IT(x) \
    {\
        unsigned counter = 0; \
        for (auto& elem : (x)) { \
            std::cout << (#x) << "[" << std::to_string(counter++) << "] = " << elem << std::endl; \
        }\
    }
#define ECHO(x) std::cout << (x) << std::endl

namespace utils {
    using Index = unsigned;

	template<typename T1, typename T2>
	class bimap {
	public:
		void reserve(std::size_t size) {
			ltr.reserve(size);
			rtl.reserve(size);
		}

		void insert(const T1& first, const T2& second) {
			ltr[first] = second;
			rtl[second] = first;
		}

		unsigned count(const T1& value) const {
			return ltr.count(value);
		}

		unsigned count(const T2& value) const {
			return rtl.count(value);
		}

		T2& operator[](const T1& value) {
			return ltr.at(value);
		}

		T1& operator[](const T2& value) {
			return rtl.at(value);
		}

		const T2& operator[](const T1& value) const {
			return ltr.at(value);
		}

		const T1& operator[](const T2& value) const {
			return rtl.at(value);
		}

		std::size_t size() const {
			return ltr.size();
		}

		typename std::unordered_map<T1, T2>::iterator begin() {
			return ltr.begin();
		}

		typename std::unordered_map<T1, T2>::const_iterator begin() const {
			return ltr.cbegin();
		}

		typename std::unordered_map<T1, T2>::iterator end() {
			return ltr.end();
		}

		typename std::unordered_map<T1, T2>::const_iterator end() const {
			return ltr.cend();
		}

	private:
		std::unordered_map<T1, T2> ltr;
		std::unordered_map<T2, T1> rtl;
	};
}

#endif