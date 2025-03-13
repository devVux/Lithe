#pragma once

#include <memory>


template <class T>
using SharedPtr = std::shared_ptr<T>;

template <class T, class D = std::default_delete<T>>
using UniquePtr = std::unique_ptr<T, D>;

template <class T, class... Args>
std::shared_ptr<T> makeShared(Args&&... args) {
	return std::make_shared<T>(std::forward<Args>(args)...);
}

template <class T, class... Args>
std::unique_ptr<T> makeUnique(Args&&... args) {
	return std::make_unique<T>(std::forward<Args>(args)...);
}


template<typename T, typename U = uint32_t>
struct Extent {
	union {
		struct { U width, height; };
		struct { U x, y; };
	};


	Extent(T w, T h): width(static_cast<U>(w)), height(static_cast<U>(h)) { }

	template<typename _T = T>
	Extent(const Extent<_T, U>& other) {
		width = static_cast<U>(other.width);
		height = static_cast<U>(other.height);
	}
};
