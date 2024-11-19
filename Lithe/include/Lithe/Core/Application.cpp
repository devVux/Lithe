#include "pch.h"
#include "Application.h"

#include "Lithe/Core/Log.h"





namespace Lithe {

	void Application::init() {
		LLGL::RenderSystemDescriptor rendererDesc;
		
		rendererDesc.moduleName = MODULE_NAME;
		
		mRenderer.init(rendererDesc);

	}

	void Application::run() {
		
		LLGL::Window& window = mRenderer.window();

		while (!window.HasQuit()) {

			mRenderer.draw();

			window.ProcessEvents();
		}


	}


}
