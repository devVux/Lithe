#include "pch.h"
#include "Application.h"

#include <LLGL/LLGL.h>
#include <LLGL/RenderSystemFlags.h>

#include "Lithe/Core/Log.h"
#include "Lithe/Core/Clock.h"
#include "Lithe/Events/Event.h"
#include "Lithe/Events/Input.h"
#include "Lithe/Events/WindowEvents.h"

#include "Lithe/Scene/OrthographicCamera.h"
#include "Lithe/Scene/Components.h"

#include <Utils/Utils.h>

#include <glm/ext/matrix_transform.hpp>


namespace Lithe {

	void Application::init() {

		pWindow = makeShared<Lithe::Window>(&mDispatcher, "Main window", Size(1200, 800));
		Input::setWindow(pWindow);
		mRenderer.init(pWindow);

		mDispatcher.subscribe<WindowEvents::WindowClosedEvent>([this](const WindowEvents::WindowClosedEvent& e) {
			stop();
		});


		auto scene = mSceneManager.create("intro");
		scene->addCamera(std::make_shared<OrthographicCamera>());

		auto e = scene->createEntity();
		e->addComponent<RenderableComponent>(glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(2.0f));
		e->addComponent<TransformComponent>(glm::mat4(1.0f));
		e->addComponent<SpriteComponent>(glm::vec4(0.6f, 1.0f, 0.6f, 1.0f));
		
		auto e2 = scene->createEntity();
		e2->addComponent<RenderableComponent>(glm::vec3(-5.0f, 1.0f, 0.0f), glm::vec3(1.0f));
		e2->addComponent<TransformComponent>(glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 3.0f, 1.0f)));
		//e2->addComponent<SpriteComponent>(glm::vec4(0.2f, 0.4f, 0.6f, 1.0f));

	}

	void Application::run() {
		
		Time::Clock clock;
		
		mRunning = true;
		while (mRunning) {
			Time::Timestep ts = clock.timeSinceLastUpdate();

			mSceneManager.update(ts);

			mRenderer.draw(*mSceneManager.active(), mSceneManager.activeCamera().get());

			pWindow->processEvents();
		}


	}


}
