#pragma once

#ifdef _WIN32
	#include <wtypes.h>
	using NativeWindowHandle = struct HWND__*;
#elif defined(__APPLE__)
	#ifdef __OBJC__
	@class NSResponder;
	@class NSString;
	@class NSObject;
	#else
	class NSResponder;
	class NSString;
	struct objc_object;
	#endif

	using NativeWindowHandle = NSResponder*;
	using NSNotificationName = NSString*;
	using id = struct objc_object*;
#else
	using NativeWindowHandle = void*; // Fallback or Linux-specific type
#endif