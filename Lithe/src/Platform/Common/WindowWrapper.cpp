#include "pch.h"

#include "WindowWrapper.h"

#include <LLGL/Platform/NativeHandle.h>

namespace Lithe {

	bool WindowWrapper::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) {
		if (nativeHandle != nullptr && nativeHandleSize == sizeof(LLGL::NativeHandle)) {
			auto* handle = reinterpret_cast<LLGL::NativeHandle*>(nativeHandle);
			
			#ifdef __APPLE__
				handle->responder = pWindow->handle();
			#elifdef _WIN32
				handle->window = pWindow->handle();
			#else
				static_assert(false && "Yet to be implemented");
			#endif

			return true;
		}
		return false;
	}

	LLGL::Extent2D WindowWrapper::GetContentSize() const {
		return pWindow->size().to<LLGL::Extent2D>();
	}
	
	bool WindowWrapper::AdaptForVideoMode(LLGL::Extent2D* resolution, bool* fullscreen) {
		pWindow->resize(Extent<long>(resolution->width, resolution->height));
		return true;
	}

	LLGL::Display* WindowWrapper::FindResidentDisplay() const {
		return LLGL::Display::GetPrimary();
	}

}
