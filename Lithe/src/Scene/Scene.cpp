#include "pch.h"
#include "Scene.h"

namespace Lithe {

	void Scene::update(const Time::Timestep ts) const {
		
		if (pActiveCamera)
			pActiveCamera->update(ts);

	}

}