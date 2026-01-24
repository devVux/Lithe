#pragma once

#include "EventDispatcher.hpp"

#include <GLFW/glfw3.h>
#include <ISurface.hpp>
#include <cstdint>
#include <expected>
#include <string>
#include <variant>

class Window: public Lithe::ISurface {

public:

	Window()						 = default;
	Window(const Window&)			 = delete;
	Window(Window&&)				 = delete;
	Window& operator=(const Window&) = delete;
	Window& operator=(Window&&)		 = delete;

	~Window() noexcept override;

	std::expected<std::monostate, std::string> init(Lithe::EventDispatcher&, uint32_t, uint32_t, std::string) noexcept;

	void update(Lithe::EventDispatcher&) const noexcept override;

	[[nodiscard]] Lithe::NativeHandle handle() const noexcept override;

	[[nodiscard]] Lithe::NativeHandle display() const noexcept override;

private:

	GLFWwindow* pWindow;
};
