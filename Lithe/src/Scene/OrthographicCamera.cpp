#include "pch.h"
#include "OrthographicCamera.h"

#include "Input.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Lithe {

	OrthographicCamera::OrthographicCamera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up): 
			mPosition(position), mTarget(target), mSpeed(10.0f), mZoom(1.0f) {

		mProjection = glm::ortho(-16.0f, 16.0f, -9.0f, 9.0f, -1.0f, 1.0f);
		mView = glm::lookAt(mPosition, mTarget, up);

	}

	void OrthographicCamera::update(const Time::Timestep ts) {

		using Lithe::Input;

		if (Input::isKeyDown(Key::W))
			mView = glm::translate(mView, glm::vec3(0, -mSpeed * ts, 0));
		if (Input::isKeyDown(Key::S))
			mView = glm::translate(mView, glm::vec3(0, mSpeed * ts, 0));

		if (Input::isKeyDown(Key::A))
			mView = glm::translate(mView, glm::vec3(mSpeed * ts, 0, 0));
		if (Input::isKeyDown(Key::D))
			mView = glm::translate(mView, glm::vec3(-mSpeed * ts, 0, 0));

		if (Input::isKeyDown(Key::Q))
			mView = glm::rotate(mView, mSpeed * (float) ts, glm::vec3(0, 0, mSpeed * ts));
		if (Input::isKeyDown(Key::E))
			mView = glm::rotate(mView, mSpeed * (float) ts, glm::vec3(0, 0, -mSpeed * ts));


	}

	void OrthographicCamera::setTarget(glm::vec3 target) { 
		mTarget = target; 
		mView = glm::lookAt(mPosition, mTarget, glm::vec3(0, 1, 0)); 
	}

	//void OrthographicCamera::onEvent(Event& e) {
	//	EventDispatcher::staticDispatch<MouseScrolledEvent>(e, [this](MouseScrolledEvent& e) {
	//		mZoom -= e.yOffset() * mZoomSens;
	//		mZoom = std::max(mZoom, 0.1f);
	//		mProjection = glm::ortho(-16.0f * mZoom, 16.0f * mZoom, -9.0f * mZoom, 9.0f * mZoom, -mZoom, mZoom);
	//	});
	//}

}
