#include "pch.h"
#include "WindowWrapper.h"

#include <LLGL/Platform/NativeHandle.h>

namespace Lithe {

	bool WindowWrapper::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) {
		if (nativeHandle != nullptr && nativeHandleSize == sizeof(LLGL::NativeHandle)) {
			auto* handle = reinterpret_cast<LLGL::NativeHandle*>(nativeHandle);
			handle->window = pWindow->handle();
			return true;
		}
		return false;
	}
	LLGL::Extent2D WindowWrapper::GetContentSize() const {
		Extent<uint32_t> size = pWindow->size();
		return { size.width, size.height };
	}

	bool WindowWrapper::AdaptForVideoMode(LLGL::Extent2D* resolution, bool* fullscreen) {
		pWindow->resize({
			static_cast<long>(resolution->width),
			static_cast<long>(resolution->height)
		});
		return true;
	}

	LLGL::Display* WindowWrapper::FindResidentDisplay() const {
		return LLGL::Display::GetPrimary();
	}

}