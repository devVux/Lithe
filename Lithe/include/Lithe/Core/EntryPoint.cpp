#include "pch.h"
#include "EntryPoint.h"

#include <LLGL/RenderSystem.h>
#include <LLGL/LLGL.h>
#include <LLGL/Window.h>

#define LLGL_BUILD_RENDERER_OPENGL

int main(int argc, char* argv[]) {

	try {

		LLGL::RenderSystemDescriptor rendererDesc;
		rendererDesc.moduleName = "Direct3D11";
		auto renderer = LLGL::RenderSystem::Load(rendererDesc);

		LLGL::WindowDescriptor windowDesc;
		windowDesc.title = "LLGL Window";
		windowDesc.size = { 720, 480 };
		windowDesc.centered = true;
		windowDesc.resizable = true;
		windowDesc.visible = true;

		auto window = LLGL::Window::Create(windowDesc);

		while (window->ProcessEvents()) {

	
		}



	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

}