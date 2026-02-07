#pragma once

#include "ICamera.hpp"
#include "EventDispatcher.hpp"

#include <glm/gtc/quaternion.hpp>

namespace Lithe {

class PerspectiveCamera: public ICamera {

	public:

		PerspectiveCamera(glm::vec3 position, glm::vec3 target);

		void update(Timestep, IInput&) noexcept override;

		glm::mat4 viewProjection() const noexcept override { return mProjection * mView; }

	private:

		glm::mat4 mProjection;
		glm::mat4 mView;

		glm::vec3 mPosition;

};


}
