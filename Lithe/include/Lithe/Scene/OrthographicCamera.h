#pragma once

#include "Camera.h"

namespace Lithe {

	class OrthographicCamera: public Camera {

		public:
	
			OrthographicCamera(const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& target = glm::vec3(0.0f, 0.0f, -1.0f), const glm::vec3& up = glm::vec3(0, 1, 0));

			virtual void update(const Time::Timestep ts) override;

			virtual glm::mat4 view() const override { return mView; }
			virtual glm::mat4 projection() const override { return mProjection; }
			virtual glm::mat4 viewProjection() const override { return mProjection * mView; }

			void setTarget(glm::vec3 target);
			glm::vec3 target() const { return mTarget; }
		
			void setView(glm::mat4 view) { mView = view; }
			void setProjection(glm::mat4 projection) { mProjection = projection; }

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
