#pragma once

#include "IInput.hpp"
#include "ICamera.hpp"
#include "Clock.hpp"

#include <vector>

#include <entt/entt.hpp>

namespace Lithe {

class Scene {

	public:

		void update(Timestep, IInput&) const noexcept;

		entt::entity createEntity() noexcept {
			return mRegistry.create();
		}

		[[nodiscard]] glm::mat4 camera() const noexcept { return pActiveCamera->viewProjection(); }
		void setCamera(std::unique_ptr<ICamera> camera) noexcept {
			pActiveCamera = std::move(camera);
		}


	private:

		std::unique_ptr<ICamera> pActiveCamera;
		entt::registry mRegistry;

};

}