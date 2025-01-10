#pragma once

#include <glm/glm.hpp>
#include <box2d/b2_body.h>

struct MeshComponent {
	glm::vec2 vertices[4];
};

struct RenderableComponent {
	bool visible;
	glm::vec3 position;
	glm::vec3 size;

	RenderableComponent(const glm::vec3& p = glm::vec3(0.0f), const glm::vec3& s = glm::vec3(1.0f), bool v = true): visible(v), position(p), size(s) { }

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
