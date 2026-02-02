#pragma once

#include "Event.hpp"
#include "Events/KeyEvents.hpp"
#include "Events/MouseEvents.hpp"

namespace Lithe {

class IInput {

public:

	virtual ~IInput() noexcept = default;

	[[nodiscard]] virtual bool isKeyUp(Key) const noexcept = 0;
	[[nodiscard]] virtual bool isKeyDown(Key) const noexcept = 0;

	[[nodiscard]] virtual bool isMouseUp(Button) const noexcept = 0;
	[[nodiscard]] virtual bool isMouseDown(Button) const noexcept = 0;

	[[nodiscard]] virtual MousePos position() const noexcept = 0;

};

}