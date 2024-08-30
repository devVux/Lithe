#include "pch.h"
#include "EntryPoint.h"

#include <LLGL/RenderSystem.h>
#include <LLGL/LLGL.h>
#include <LLGL/Window.h>
#include <LLGL/WindowFlags.h>

#define LLGL_BUILD_RENDERER_OPENGL

int main(int argc, char* argv[]) {

	try {

		LLGL::RenderSystemDescriptor rendererDesc;
		rendererDesc.moduleName = "Direct3D11";
		auto renderer = LLGL::RenderSystem::Load(rendererDesc);

		LLGL::WindowDescriptor windowDesc;
		windowDesc.title = "LLGL Window";
		windowDesc.size = { 720, 480 };
		windowDesc.flags = LLGL::WindowFlags::Visible | LLGL::WindowFlags::Centered | LLGL::WindowFlags::Resizable;
	

		auto window = LLGL::Window::Create(windowDesc);
		while (!window->HasQuit()) {
			std::cout << " nigga\n";
			window->ProcessEvents();
	
		}



	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

}