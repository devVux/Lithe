#pragma once

#include "ICamera.hpp"

namespace Lithe {

class PerspectiveCamera: public ICamera {

	public:

		PerspectiveCamera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up = glm::vec3(0, 1, 0));

		virtual void update(Timestep, IInput&) noexcept override;


		virtual glm::mat4 viewProjection() const noexcept override { return mProjection * mView; }

		glm::vec3 position() const { return mPosition; }
		glm::vec3 target() const { return mTarget; }

		void setPosition(glm::vec3 position) noexcept { mPosition = position; }
		void setTarget(glm::vec3 target) noexcept{ 
			target = glm::normalize(target);
			mTarget = target;
		}

		void setFov(float angle) noexcept {
			//setProjection(glm::pre(mFOV, (16.0f / 9.0f), -1.0f, 1.0f));
		}

		float fov() const noexcept { return mFOV; }

	private:

		glm::mat4 mProjection;
		glm::mat4 mView;

		glm::vec3 mPosition;
		glm::vec3 mTarget;

		float mFOV { 45.0f };
		float mZoom { 1.0f };

};


}
