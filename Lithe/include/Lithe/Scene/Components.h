#pragma once

#include <glm/glm.hpp>
#include <box2d/b2_body.h>

struct MeshComponent {
	glm::vec2 vertices[4];
};

struct TransformComponent {

	glm::mat4 transform;

	TransformComponent(const glm::mat4& t = glm::mat4(1.0f)): transform(t) { }

	operator const glm::mat4&() const { return transform; }

};

struct SpriteComponent {

	glm::vec4 color;

	SpriteComponent(const glm::vec4& c = glm::vec4(1.0f)): color(c) { }

	operator const glm::vec4&() const { return color; }

};

struct PhysicsComponent {

	b2Body* body;

	PhysicsComponent(b2Body* b): body(b) { }

};
