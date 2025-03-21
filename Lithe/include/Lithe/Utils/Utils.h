#pragma once

#include <fmt/core.h>
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


template<typename T = uint32_t>
struct Extent {
	union {
		struct { T width, height; };
		struct { T x, y; };
	};


	Extent(T w, T h): width(w), height(h) { }
	Extent(const Extent& other) = default;

	template<typename _U, typename _T = T>
	static _U to(const Extent<_T>& extent) {
		return _U(
			static_cast<_T>(extent.width),
			static_cast<_T>(extent.height)
		);
	}

	template<typename _U, typename _T = T>
	_U to() const {
		return Extent::to<_U, _T>(*this);
	}
		
};

using Size = Extent<long>;
using Pos = Size;
using MousePos = Extent<double>;
