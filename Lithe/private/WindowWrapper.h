#pragma once

#include "Window.h"

#include <LLGL/Surface.h>

namespace Lithe { 

	class WindowWrapper: public LLGL::Surface {

		public:

			WindowWrapper(Lithe::Window* window): pWindow(window) { }

			bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) final;
			[[nodiscard]] LLGL::Extent2D GetContentSize() const final;
			bool AdaptForVideoMode(LLGL::Extent2D* resolution, bool* fullscreen) final;
			[[nodiscard]] LLGL::Display* FindResidentDisplay() const final;

		private:

			Lithe::Window* pWindow;

	};

}
