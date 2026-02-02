#include "PerspectiveCamera.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace Lithe {

PerspectiveCamera::PerspectiveCamera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up): 
	mPosition(position), mTarget(target),
	mProjection(glm::perspective(mFOV, (16.0f / 9.0f), -100.0f, 1.0f)),
	mView(glm::lookAt(position, target, up)) {


}

void PerspectiveCamera::update(Timestep ts, IInput& input) noexcept {

	static constexpr auto mSpeed = 1.0f;
	static constexpr auto scrollSpeed = 1.0f;
	static constexpr auto zoomSpeed = 1.0f;

	if (input.isKeyDown(Key::W))
		mView = glm::translate(mView, glm::vec3(0, 0, mSpeed * ts));
	if (input.isKeyDown(Key::S))
		mView = glm::translate(mView, glm::vec3(0, 0, -mSpeed * ts));

	if (input.isKeyDown(Key::A))
		mView = glm::translate(mView, glm::vec3(mSpeed * ts, 0, 0));
	if (input.isKeyDown(Key::D))
		mView = glm::translate(mView, glm::vec3(-mSpeed * ts, 0, 0));

	if (input.isKeyDown(Key::Q))
		mView = glm::rotate(mView, mSpeed * (float) ts, glm::vec3(0, mSpeed * ts, 0));
	if (input.isKeyDown(Key::E))
		mView = glm::rotate(mView, mSpeed * (float) ts, glm::vec3(0, -mSpeed * ts, 0));

}


}

