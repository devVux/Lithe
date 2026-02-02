#pragma once

#include "ICamera.hpp"

namespace Lithe {

class OrthographicCamera: public ICamera {

	public:
	
		OrthographicCamera(const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& target = glm::vec3(0.0f, 0.0f, -1.0f), const glm::vec3& up = glm::vec3(0, 1, 0));

		virtual void update(Timestep, IInput&) noexcept override;

		[[nodiscard]] virtual glm::mat4 viewProjection() const noexcept override { return mProjection * mView; }

		void setTarget(glm::vec3 target) noexcept;
		[[nodiscard]] glm::vec3 target() const noexcept { return mTarget; }
		

	protected:

		glm::mat4 mProjection;
		glm::mat4 mView;

		glm::vec3 mPosition;
		glm::vec3 mTarget;

		float mSpeed;
		float mZoom;
		float mZoomSens { 0.1f };

};

}
