#include "pch.h"
#include "Application.h"

#include "Lithe/Core/Log.h"
#include "Lithe/Core/Clock.h"
#include "Lithe/Events/Event.h"
#include "Lithe/Events/Input.h"

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

		{
			mDispatcher.subscribe<WindowEvents::WindowCloseEvent>([this](const WindowEvents::WindowCloseEvent& e) {
				stop();
			});
			mDispatcher.subscribe<MouseEvents::MouseButtonPressedEvent>([this](const MouseEvents::MouseButtonPressedEvent& e) {
				Lithe::Log::TRACE("{}", Input::isKeyDown(Keys::A));
			});
		}


		mSceneManager.create("intro");

	}

	void Application::run() {
		
		Time::Clock clock;
		

		mRunning = true;
		while (mRunning) {

			mRenderer.draw();

			Time::Timestep delta = clock.timeSinceLastUpdate();

			mSceneManager.update(delta);

			pWindow->ProcessEvents();
		}


	}


}
