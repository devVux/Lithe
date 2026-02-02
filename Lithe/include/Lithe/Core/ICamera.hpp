#pragma once

#include "IInput.hpp"

#include <glm/glm.hpp>

namespace Lithe {

class ICamera {

	public:

		virtual ~ICamera() = default;

		virtual void update(float, IInput&) noexcept = 0;

		[[nodiscard]] virtual glm::mat4 viewProjection() const noexcept = 0;

};

}
