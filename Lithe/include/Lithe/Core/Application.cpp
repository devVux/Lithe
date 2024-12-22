#include "pch.h"
#include "Application.h"

#include "Lithe/Core/Log.h"
#include "Lithe/Core/Clock.h"
#include "Lithe/Events/Event.h"
#include "Lithe/Events/Input.h"

#include "Lithe/Scene/OrthographicCamera.h"

#include <GLFW/glfw3.h>
#include <Utils/Utils.h>


namespace Lithe {

	void Application::init() {
		LLGL::RenderSystemDescriptor rendererDesc;
		rendererDesc.moduleName = MODULE_NAME;

		if (!glfwInit())
			Lithe::Log::ERR("Could not load GLFW");

		pWindow = makeShared<Lithe::Window>(mDispatcher, LLGL::Extent2D(1200, 800), "Main window");
		mRenderer.init(pWindow, rendererDesc);

		Input::setInput(pWindow.get());

		
		mDispatcher.subscribe<WindowEvents::WindowCloseEvent>([this](const WindowEvents::WindowCloseEvent& e) {
			stop();
		});


		auto scene = mSceneManager.create("intro");
		scene->add(std::make_shared<OrthographicCamera>());


	}

	void Application::run() {
		
		Time::Clock clock;
		
		mRunning = true;
		while (mRunning) {
			Time::Timestep ts = clock.timeSinceLastUpdate();

			mSceneManager.update(ts);

			mRenderer.draw(mSceneManager.activeCamera().get());

			pWindow->ProcessEvents();
		}


	}


}
