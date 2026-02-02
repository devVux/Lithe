#pragma once

#include <IInput.hpp>

#include <GLFW/glfw3.h>

using namespace Lithe;

class MyInput : public IInput {
public:
	MyInput(GLFWwindow* w) : window(w) {}

	[[nodiscard]] bool isKeyUp(Key code) const noexcept override {
		return glfwGetKey(window, static_cast<int>(code)) == GLFW_RELEASE;
	}

	[[nodiscard]] bool isKeyDown(Key code) const noexcept override {
		return glfwGetKey(window, static_cast<int>(code)) == GLFW_PRESS;
	}

	[[nodiscard]] bool isMouseUp(Button button) const noexcept override {
		return glfwGetMouseButton(window, static_cast<int>(button)) == GLFW_RELEASE;
	}

	[[nodiscard]] bool isMouseDown(Button button) const noexcept override {
		return glfwGetMouseButton(window, static_cast<int>(button)) == GLFW_RELEASE;
	}

	[[nodiscard]] MousePos position() const noexcept override {
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		return { static_cast<uint32_t>(x), static_cast<uint32_t>(y) };
	}

private:
	GLFWwindow* window;
	int mScrollX{ 0 };
	int mScrollY{ 0 };
};

