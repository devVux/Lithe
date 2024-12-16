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

