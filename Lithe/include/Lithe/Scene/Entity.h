#pragma once

#include <entt/entt.hpp>

class Entity {

	public:

		Entity() = default;
		Entity(entt::registry& registry, entt::entity entityID): mHandle(registry, entityID) { }

		template <class T, class... Args>
		void addComponent(Args&&... args) {
			mHandle.emplace<T>(std::forward<Args>(args)...);
		}

		template <class T>
		bool hasComponent() {
			return mHandle.try_get<T>();
		}

		entt::entity id() const { return mHandle.entity(); }

		bool operator<=>(const Entity& other) const { return mHandle == other.mHandle; }

	private:

		entt::handle mHandle;

};
