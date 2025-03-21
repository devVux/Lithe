#include "pch.h"
#include "PerspectiveCamera.h"

#include "Lithe/Events/Input.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Lithe {

	PerspectiveCamera::PerspectiveCamera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up): 
		mPosition(position), mTarget(target), mSpeed(1.0f), mFOV(30.0f),
		mProjection(glm::perspective(mFOV, (16.0f / 9.0f), -100.0f, 1.0f)),
		mView(glm::lookAt(position, target, up)) {


	}

	void PerspectiveCamera::update(const Time::Timestep ts) {

		using Lithe::Input;

		{
			if (Input::isKeyDown(Key::W))
				mView = glm::translate(mView, glm::vec3(0, 0, mSpeed * ts));
			if (Input::isKeyDown(Key::S))
				mView = glm::translate(mView, glm::vec3(0, 0, -mSpeed * ts));

			if (Input::isKeyDown(Key::A))
				mView = glm::translate(mView, glm::vec3(mSpeed * ts, 0, 0));
			if (Input::isKeyDown(Key::D))
				mView = glm::translate(mView, glm::vec3(-mSpeed * ts, 0, 0));

			if (Input::isKeyDown(Key::Q))
				mView = glm::rotate(mView, mSpeed * (float) ts, glm::vec3(0, mSpeed * ts, 0));
			if (Input::isKeyDown(Key::E))
				mView = glm::rotate(mView, mSpeed * (float) ts, glm::vec3(0, -mSpeed * ts, 0));
	
		}

	}


}

