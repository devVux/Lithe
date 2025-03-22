#pragma once

#include "Camera.h"

namespace Lithe {

	class PerspectiveCamera: public Camera {

		public:

			PerspectiveCamera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up = glm::vec3(0, 1, 0));

			virtual void update(const Time::Timestep ts) override;


			virtual glm::mat4 view() const override { return mView; }
			virtual glm::mat4 projection() const override { return mProjection; }
			virtual glm::mat4 viewProjection() const override { return mProjection * mView; }

			void setView(glm::mat4 view) { mView = view; }
			void setProjection(glm::mat4 projection) { mProjection = projection; }

			glm::vec3 position() const { return mPosition; }
			glm::vec3 target() const { return mTarget; }

			void setPosition(glm::vec3 position) { mPosition = position; }
			void setTarget(glm::vec3 target) { 
				target = glm::normalize(target);
				mTarget = target;
			}

			void setFov(float angle) {
				//setProjection(glm::pre(mFOV, (16.0f / 9.0f), -1.0f, 1.0f));
			}

			float fov() const { return mFOV; }

		private:

			glm::mat4 mProjection;
			glm::mat4 mView;

			glm::vec3 mPosition;
			glm::vec3 mTarget;

			float mSpeed;
			float mFOV;

	};


}
