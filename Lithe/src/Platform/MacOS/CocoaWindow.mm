#include "pch.h"
#include "Window.h"
#include "Log.h"
#include "Event.h"
#include "EventDispatcher.h"

#include <Cocoa/Cocoa.h>

@interface LitheWindowDelegate : NSObject<NSWindowDelegate>
@property (nonatomic, assign) Lithe::Window* windowPtr;
@property (nonatomic, assign) BOOL windowShouldClose;
@end

@implementation LitheWindowDelegate
- (instancetype) init {
    if (self = [super init]) {
        _windowShouldClose = NO;
    }
    return self;
}

- (BOOL) windowShouldClose:(NSWindow*) sender {
    self.windowShouldClose = YES;
    if (self.windowPtr)
        self.windowPtr->dispatchEvent(NSWindowWillCloseNotification, 0);
    return YES;
}

- (void) windowWillClose:(NSNotification*) notification {
    self.windowShouldClose = YES;
}
@end

namespace Lithe {

Window::Window(EventDispatcher* pDispatcher, std::string title, Size size, Pos pos, bool centered) : 
    pDispatcher(pDispatcher), mCentered(centered) {
    
    @autoreleasepool {
        // Create app if needed
        if (NSApp == nil) {
            [NSApplication sharedApplication];
            [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
            [NSApp finishLaunching];
        }
        
        // Create window
        NSRect contentRect = NSMakeRect(pos.x, pos.y, size.width, size.height);
        NSUInteger styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | 
                               NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable;
        
        NSWindow* window = [[NSWindow alloc] initWithContentRect:contentRect
                                                       styleMask:styleMask
                                                         backing:NSBackingStoreBuffered
                                                           defer:NO];
        
        // Set title
        [window setTitle:[NSString stringWithUTF8String:title.c_str()]];
        
        // Create and set delegate
        LitheWindowDelegate* delegate = [[LitheWindowDelegate alloc] init];
        delegate.windowPtr = this;
        [window setDelegate:delegate];
        
        // Setup content view
        NSView* contentView = [[NSView alloc] initWithFrame:contentRect];
        [window setContentView:contentView];
        
        // Setup event tracking for mouse
        [contentView setWantsLayer:YES];
        
        // Center window if needed
        if (centered) {
            [window center];
        }
        
        // Set up event handlers
        NSTrackingAreaOptions trackingOptions = NSTrackingActiveAlways | NSTrackingInVisibleRect |
                                                NSTrackingMouseMoved | NSTrackingMouseEnteredAndExited;
        NSTrackingArea* trackingArea = [[NSTrackingArea alloc] initWithRect:[contentView bounds]
                                                                    options:trackingOptions
                                                                      owner:contentView
                                                                   userInfo:nil];
        [contentView addTrackingArea:trackingArea];
        
        pNativeHandle = (__bridge NativeWindowHandle)window;
        
        // Add mouse event monitor
        NSEventMask eventMask = NSEventMaskLeftMouseDown | NSEventMaskLeftMouseUp | 
                                NSEventMaskRightMouseDown | NSEventMaskRightMouseUp | 
                                NSEventMaskMouseMoved;
        
        [NSEvent addLocalMonitorForEventsMatchingMask:eventMask handler:^NSEvent * _Nullable(NSEvent * _Nonnull event) {
            NSWindow* targetWindow = [event window];
            if (targetWindow == (__bridge NSWindow*) pNativeHandle) {
                NSPoint locationInWindow = [event locationInWindow];
                NSPoint locationInView = [[targetWindow contentView] convertPoint:locationInWindow fromView:nil];
                
                switch ([event type]) {
                    case NSEventTypeMouseMoved:
                        pDispatcher->enqueue<MouseEvents::MouseMovedEvent>(locationInView.x, locationInView.y);
                        break;
                    case NSEventTypeLeftMouseDown:
                        pDispatcher->enqueue<MouseEvents::MouseButtonPressedEvent>(MouseButtons::BUTTON_1);
                        break;
                    case NSEventTypeLeftMouseUp:
                        pDispatcher->enqueue<MouseEvents::MouseButtonReleasedEvent>(MouseButtons::BUTTON_1);
                        break;
                    case NSEventTypeRightMouseDown:
                        pDispatcher->enqueue<MouseEvents::MouseButtonPressedEvent>(MouseButtons::BUTTON_2);
                        break;
                    case NSEventTypeRightMouseUp:
                        pDispatcher->enqueue<MouseEvents::MouseButtonReleasedEvent>(MouseButtons::BUTTON_2);
                        break;
                    default:
                        break;
                }
            }
            return event;
        }];
    }

	show();
}

Window::~Window() {
    if (pNativeHandle) {
        @autoreleasepool {
            NSWindow* window = (__bridge NSWindow*) pNativeHandle;
            [window setDelegate:nil];
            [window close];
        }
        pNativeHandle = nullptr;
    }
}

void Window::processEvents() const {
    @autoreleasepool {
        NSEvent* event;
        while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                          untilDate:[NSDate distantPast]
                                             inMode:NSDefaultRunLoopMode
                                            dequeue:YES])) {
            [NSApp sendEvent:event];
        }
    }
}

bool Window::shouldClose() const {
    @autoreleasepool {
        NSWindow* window = (__bridge NSWindow*)pNativeHandle;
        LitheWindowDelegate* delegate = (LitheWindowDelegate*)[window delegate];
        return [delegate windowShouldClose];
    }
}

void Window::show() const {
    @autoreleasepool {
        NSWindow* window = (__bridge NSWindow*) pNativeHandle;
        [window makeKeyAndOrderFront:nil];
        [NSApp activateIgnoringOtherApps:YES];
    }
}

void Window::hide() const {
    @autoreleasepool {
        NSWindow* window = (__bridge NSWindow*) pNativeHandle;
        [window orderOut:nil];
    }
}

void Window::rename(const std::string& title) {
    @autoreleasepool {
        NSWindow* window = (__bridge NSWindow*) pNativeHandle;
        [window setTitle:[NSString stringWithUTF8String:title.c_str()]];
    }
}

std::string Window::title() const {
    @autoreleasepool {
        NSWindow* window = (__bridge NSWindow*) pNativeHandle;
        NSString* title = [window title];
        return [title UTF8String];
    }
}

void Window::resize(Size size) {
    @autoreleasepool {
        NSWindow* window = (__bridge NSWindow*) pNativeHandle;
        NSRect frame = [window frame];
        NSRect newFrame = [window frameRectForContentRect:NSMakeRect(frame.origin.x, 
                                                              frame.origin.y, 
                                                              size.width, 
                                                              size.height)];
        [window setFrame:newFrame display:YES];
    }
}

Window::Size Window::size() const {
    @autoreleasepool {
        NSWindow* window = (__bridge NSWindow*) pNativeHandle;
        NSRect contentRect = [[window contentView] frame];
        return {static_cast<long>(contentRect.size.width), static_cast<long>(contentRect.size.height)};
    }
}

void Window::move(Window::Pos pos) {
    @autoreleasepool {
        NSWindow* window = (__bridge NSWindow*) pNativeHandle;
        NSRect screenRect = [[NSScreen mainScreen] frame];
        NSRect frame = [window frame];
        
        // Convert to bottom-left coordinate system (Cocoa's default)
        float flippedY = screenRect.size.height - frame.size.height - pos.y;
        [window setFrameOrigin:NSMakePoint(pos.x, flippedY)];
    }
}

Window::Size Window::position() const {
    @autoreleasepool {
        NSWindow* window = (__bridge NSWindow*) pNativeHandle;
        NSRect screenRect = [[NSScreen mainScreen] frame];
        NSRect frame = [window frame];
        
        // Convert back to top-left coordinates to match Win32 behavior
        float flippedY = screenRect.size.height - frame.size.height - frame.origin.y;
        return {static_cast<long>(frame.origin.x), static_cast<long>(flippedY)};
    }
}

Window::Size Window::screenSize() const {
    @autoreleasepool {
        NSRect screenRect = [[NSScreen mainScreen] frame];
        return {static_cast<long>(screenRect.size.width), static_cast<long>(screenRect.size.height)};
    }
}

void Window::dispatchEvent(NSNotificationName notificationName, id sender) {
	if ([notificationName isEqualToString:NSWindowWillCloseNotification])
        pDispatcher->enqueue<WindowEvents::WindowCloseEvent>();
}

}
