#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Lithe/Core/Clock.h"
#include "Lithe/Utils/Utils.h"

#include "Lithe/Events/Event.h"

class OrthographicCamera: public EventListener {

	public:
	
		OrthographicCamera(const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& target = glm::vec3(0.0f, 0.0f, -1.0f), const glm::vec3& up = glm::vec3(0, 1, 0));

		void update(Timestep ts);
		virtual void onEvent(Event& e) override;

		glm::mat4 view() const { return mView; }
		glm::mat4 projection() const { return mProjection; }
		glm::mat4 viewProjection() const { return mProjection * mView; }

		void setTarget(glm::vec3 target) { mTarget = target; mView = glm::lookAt(mPosition, mTarget, glm::vec3(0, 1, 0)); }
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
