#include "pch.h"
#include "Application.h"

#include "Lithe/Core/Log.h"
#include "Lithe/Events/Event.h"

#include <GLFW/glfw3.h>
#include <Utils/Utils.h>


namespace Lithe {

	void Application::init() {
		LLGL::RenderSystemDescriptor rendererDesc;
		rendererDesc.moduleName = MODULE_NAME;

		if (!glfwInit())
			Lithe::Log::ERR("Could not load GLFW");

		pWindow = makeShared<Lithe::Window>(pDispatcher, LLGL::Extent2D(1200, 800), "Main window");
		mRenderer.init(pWindow, rendererDesc);



		pDispatcher->subscribe<WindowEvents::WindowCloseEvent>([this](const WindowEvents::WindowCloseEvent& e) {
			stop();
		});
		

		pDispatcher->subscribe<MouseEvents::MouseButtonPressedEvent>([this](const MouseEvents::MouseButtonPressedEvent& e) {
			Lithe::Log::TRACE("{}", e.button);
		});
		
	}

	void Application::run() {
		
		mRunning = true;
		while (mRunning) {

			mRenderer.draw();

			pWindow->ProcessEvents();
		}


	}


}
