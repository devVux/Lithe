#include "pch.h"

#include "PerspectiveCamera.h"
#include "Lithe/Core/Input.h"

PerspectiveCamera::PerspectiveCamera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up): 
	mPosition(position), mTarget(target), mSpeed(1.0f), mFOV(30.0f), mCoord({ 640, 360 }),
	mProjection(glm::perspective(mFOV, (16.0f / 9.0f), -100.0f, 1.0f)),
	mView(glm::lookAt(position, target, up)) {


}

void PerspectiveCamera::update(Timestep ts) {

	{
		if (Input::isKeyDown(Keys::W))
			mView = glm::translate(mView, glm::vec3(0, 0, mSpeed * ts));
		if (Input::isKeyDown(Keys::S))
			mView = glm::translate(mView, glm::vec3(0, 0, -mSpeed * ts));

		if (Input::isKeyDown(Keys::A))
			mView = glm::translate(mView, glm::vec3(mSpeed * ts, 0, 0));
		if (Input::isKeyDown(Keys::D))
			mView = glm::translate(mView, glm::vec3(-mSpeed * ts, 0, 0));

		if (Input::isKeyDown(Keys::Q))
			mView = glm::rotate(mView, mSpeed * (float) ts, glm::vec3(0, mSpeed * ts, 0));
		if (Input::isKeyDown(Keys::E))
			mView = glm::rotate(mView, mSpeed * (float) ts, glm::vec3(0, -mSpeed * ts, 0));
	
	}

}

