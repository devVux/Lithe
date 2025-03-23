#pragma once

#include "Clock.h"
#include <glm/glm.hpp>

namespace Lithe {

	class Camera {

		public:

			virtual ~Camera() = default;

			virtual void update(const Time::Timestep ts) = 0;

			virtual glm::mat4 view() const = 0;
			virtual glm::mat4 projection() const = 0;
			virtual glm::mat4 viewProjection() const = 0;


	};

}
