#pragma once

#include <EventDispatcher.hpp>

namespace Lithe {

using NativeHandle = void*;

class ISurface {

  public:

	ISurface()							 = default;
	ISurface(const ISurface&)			 = delete;
	ISurface(ISurface&&)				 = delete;
	ISurface& operator=(const ISurface&) = delete;
	ISurface& operator=(ISurface&&)		 = delete;
	virtual ~ISurface() noexcept		 = default;

	// Support for native windows (e.g. Win32, Cocoa, X11, ecc...) via injected `EventDispatcher`
	// If you have a reactive windowing system, just subclass and pass the dispatcher
	// to an init function or constructor
	virtual void update(EventDispatcher&) const noexcept = 0;

	[[nodiscard]] virtual NativeHandle handle() const noexcept = 0;

	// For linux only
	[[nodiscard]] virtual NativeHandle display() const noexcept { return nullptr; }
};

} // namespace Lithe
