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


template<typename T = int32_t>
struct Extent {
	union {
		struct { T width, height; };
		struct { T x, y; };
	};


	Extent(T w, T h): width(w), height(h) { }

	template<typename U = T>
	Extent(const Extent<U>& other) {
		width = static_cast<T>(other.width);
		height = static_cast<T>(other.height);
	}
};
