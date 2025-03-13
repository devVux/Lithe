#pragma once

#include "Lithe/Core/Log.h"
#include "Lithe/Core/Clock.h"
#include "Lithe/Scene/Entity.h"
#include "Lithe/Scene/Camera.h"

#include <memory>

namespace Lithe {

	class Scene {

		public:

			void update(const Time::Timestep ts) const;

			std::shared_ptr<Entity> createEntity() {
				std::shared_ptr<Entity> e = std::make_shared<Entity>(mRegistry, mRegistry.create());
				mEntities.push_back(e);
				return e;
			}

			template <class T>
			T* get(const Entity& entity) {
				return mRegistry.try_get<T>(entity.id());
			}


			void addCamera(std::shared_ptr<Camera> camera) {
				if (camera) {
					mCameras.push_back(camera);
					pActiveCamera = camera;
				}
			}

			std::shared_ptr<Camera> camera() const { return pActiveCamera; }

			
			auto begin() { return mEntities.begin(); }
			auto end() { return mEntities.end(); }
			auto begin() const { return mEntities.begin(); }
			auto end() const { return mEntities.end(); }

		private:

			std::vector<std::shared_ptr<Camera>> mCameras;
			std::shared_ptr<Camera> pActiveCamera { nullptr };


			std::vector<std::shared_ptr<Entity>> mEntities;
			entt::registry mRegistry;

	};
			

	class SceneManager {

		using ScenePtr = std::shared_ptr<Scene>;

		public:

			ScenePtr create(const std::string& name) {
				ScenePtr scene = std::make_shared<Scene>();
				mScenes[name] = scene;
				pActiveScene = scene;
				return scene;
			}

			void remove(const std::string& name) {
				auto it = mScenes.find(name);
				if (it != mScenes.end()) {
					if (pActiveScene == it->second)
						pActiveScene = nullptr; 
					mScenes.erase(it);
				} 
				else
					Log::WARN("{} scene not found for removal", name);
			}

			void update(Time::Timestep ts) const { 
				if (pActiveScene)
					pActiveScene->update(ts);
			}


			ScenePtr active(const std::string& name) {
				const auto& it = mScenes.find(name);
				if (it != mScenes.end()) {
					pActiveScene = it->second;
					return pActiveScene;
				}

				Log::WARN("{} scene not found", name);
				pActiveScene = nullptr;
				return nullptr;
			}

			ScenePtr active() const { return pActiveScene; }

			std::shared_ptr<Camera> activeCamera() const { return pActiveScene == nullptr ? nullptr : pActiveScene->camera(); }

		private:

			std::unordered_map<std::string, ScenePtr> mScenes;
			ScenePtr pActiveScene { nullptr };

	};

}
