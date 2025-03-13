#pragma once

#include "Window.h"

#include <LLGL/LLGL.h>
#include <LLGL/Surface.h>

namespace Lithe { 

	class WindowWrapper: public LLGL::Surface {

		public:

			WindowWrapper(Lithe::Window* window): pWindow(window) { }

			bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) final override;
			LLGL::Extent2D GetContentSize() const final override;
			bool AdaptForVideoMode(LLGL::Extent2D* resolution, bool* fullscreen) final override;
			LLGL::Display* FindResidentDisplay() const final override;

		private:

			Lithe::Window* pWindow;

	};

}