#include "PerspectiveCamera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>

namespace Lithe {

PerspectiveCamera::PerspectiveCamera(glm::vec3 position, glm::vec3 target): 
 	mPosition(position),
	mProjection(glm::perspectiveRH_ZO(glm::radians(60.0f), (16.0f / 9.0f), 0.01f, 100.0f)),
	mView(glm::lookAtRH(position, target, glm::vec3(0.0f, 1.0f, 0.0f)))
	{ 
	
	
	}

static float angle = 0.0f;

void PerspectiveCamera::update(Timestep ts, IInput& input) noexcept {
	static constexpr float moveSpeed = 15.0f;
	static constexpr float rotationSpeed = 0.75f;

	static constexpr auto worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

	float velocity = moveSpeed * ts;

	if (input.isKeyDown(Key::Q))
		angle += glm::radians(-rotationSpeed);
	if (input.isKeyDown(Key::E))
		angle += glm::radians(rotationSpeed);

	glm::vec3 front;
	front.x = sin(angle);
	front.y = 0.0f;
	front.z = -cos(angle);
	front = glm::normalize(front);

	glm::vec3 right = glm::normalize(glm::cross(front, worldUp));

	if (input.isKeyDown(Key::W))
		mPosition += front * velocity;
	if (input.isKeyDown(Key::S))
		mPosition += -front * velocity;
	if (input.isKeyDown(Key::A))
		mPosition += -right * velocity;
	if (input.isKeyDown(Key::D))
		mPosition += right * velocity;

	glm::vec3 target = mPosition + front;

	mView = glm::lookAtRH(mPosition, target, worldUp);
}

}