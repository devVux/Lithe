#include "Scene.hpp"

#include <cassert>

namespace Lithe {

void Scene::update(Timestep ts, IInput& input) const noexcept {

	assert(pActiveCamera);
	pActiveCamera->update(ts, input);

}

}