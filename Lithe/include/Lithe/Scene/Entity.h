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

		template <class T, class... Args>
		T& getOrCreate(Args&&... args) {
			if (auto* component = mHandle.try_get<T>())
				return *component;

			return mHandle.emplace<T>(std::forward<Args>(args)...);
		}

		template <class T>
		T& get() {
			return mHandle.get<T>();
		}

		template <class T>
		bool has() {
			return mHandle.try_get<T>();
		}

		entt::entity id() const { return mHandle.entity(); }

		//bool operator<=>(const Entity& other) const { return mHandle == other.mHandle; }

	private:

		entt::handle mHandle;

};
