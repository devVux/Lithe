#pragma once

#include "EventDispatcher.hpp"

#include <ISurface.hpp>
#include <cstdint>
#include <string>

class MyWindow: public Lithe::ISurface {

public:

	MyWindow()							 = default;
	MyWindow(const MyWindow&)			 = delete;
	MyWindow(MyWindow&&)				 = delete;
	MyWindow& operator=(const MyWindow&) = delete;
	MyWindow& operator=(MyWindow&&)		 = delete;

	~MyWindow() noexcept override;

	bool init(Lithe::EventDispatcher&, uint32_t, uint32_t, std::string) noexcept;

	void update(Lithe::EventDispatcher&) const noexcept override;

	[[nodiscard]] Lithe::NativeHandle native() const noexcept override;
	[[nodiscard]] Lithe::Size size() const noexcept override;

private:

	struct GLFWwindow* pWindow;
};
