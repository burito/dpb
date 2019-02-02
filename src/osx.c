/*
Copyright (c) 2015 Daniel Burke

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
*/

#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <limits.h>
#include <objc/objc.h>
#include <objc/message.h>
#include <objc/NSObjCRuntime.h>

#include <CoreGraphics/CGBase.h>
#include <CoreGraphics/CGGeometry.h>

#include <CoreVideo/CVReturn.h>
#include <CoreVideo/CVDisplayLink.h>

#include "log.h"
#include "global.h"

///////////////////////////////////////////////////////////////////////////////
//////// Public Interface to the rest of the program
///////////////////////////////////////////////////////////////////////////////

void gfx_resize(void);
void osx_view_init(void);

int fullscreen = 0;
int fullscreen_toggle = 0;

extern id NSApp;
extern id const NSDefaultRunLoopMode;
id window;


///////////////////////////////////////////////////////////////////////////////
//////// Mac OS X OpenGL window setup
///////////////////////////////////////////////////////////////////////////////

pthread_mutex_t mutex_vsync = PTHREAD_MUTEX_INITIALIZER;
CVDisplayLinkRef _displayLink;

CVReturn DisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
	pthread_mutex_unlock(&mutex_vsync);
	usleep(1000);
	pthread_mutex_lock(&mutex_vsync);
	return kCVReturnSuccess;
}

char* ns_str(id item)
{
	SEL sel_description = sel_registerName("description");
	SEL sel_cStringUsingEncoding = sel_registerName("cStringUsingEncoding:");
	// NSUTF8StringEncoding = 4
	id desc = objc_msgSend(item, sel_description);
	return (char*)objc_msgSend(desc, sel_cStringUsingEncoding, 4);
}

void sys_menu_init(void)
{
	// get selectors and classes
	SEL sel_alloc = sel_registerName("alloc");
	SEL sel_init = sel_registerName("init");
	Class class_NSMenu = objc_getClass("NSMenu");
	SEL sel_setMainMenu = sel_registerName("setMainMenu:");
	Class class_NSMenuItem = objc_getClass("NSMenuItem");
	SEL sel_initWithTitle_action_keyEquivalent = sel_registerName("initWithTitle:action:keyEquivalent:");
	SEL sel_addItem = sel_registerName("addItem:");
	SEL sel_setSubmenu = sel_registerName("setSubmenu:");
	SEL sel_separatorItem = sel_registerName("separatorItem");
	SEL sel_setKeyEquivalentModifierMask = sel_registerName("setKeyEquivalentModifierMask:");

	Class class_NSString = objc_getClass("NSString");
	SEL sel_stringWithUTF8String = sel_registerName("stringWithUTF8String:");
	SEL sel_stringByAppendingString = sel_registerName("stringByAppendingString:");

	Class class_NSProcessInfo = objc_getClass("NSProcessInfo");
	SEL sel_processInfo = sel_registerName("processInfo");
	SEL sel_processName = sel_registerName("processName");

	SEL sel_orderFrontStandardAboutPanel = sel_registerName("orderFrontStandardAboutPanel:");
	SEL sel_toggleFullScreen = sel_registerName("toggleFullScreen:");
	SEL sel_terminate = sel_registerName("terminate:");

	// create the needed strings
//	id str_appname = [[NSProcessInfo processInfo] processName];
	id obj_processInfo = objc_msgSend((id)class_NSProcessInfo, sel_processInfo);
	id str_appname = objc_msgSend(obj_processInfo, sel_processName);

	id str_about = objc_msgSend((id)class_NSString, sel_stringWithUTF8String, "About ");
	id str_about_app = objc_msgSend(str_about, sel_stringByAppendingString, str_appname);
	id str_quit = objc_msgSend((id)class_NSString, sel_stringWithUTF8String, "Quit ");
	id str_quit_app = objc_msgSend(str_quit, sel_stringByAppendingString, str_appname);
	id str_hotkey_fullscreen = objc_msgSend((id)class_NSString, sel_stringWithUTF8String, "f");
	id str_hotkey_quit = objc_msgSend((id)class_NSString, sel_stringWithUTF8String, "q");
	id str_empty = objc_msgSend((id)class_NSString, sel_stringWithUTF8String, "");

	// prepare the menu objects
//	id menubar = [[NSMenu alloc] init];
	id menubar = objc_msgSend((id)class_NSMenu, sel_alloc);
	objc_msgSend(menubar, sel_init);
//	id menubaritem_app = [[NSMenuItem alloc] init];
	id menubaritem_app = objc_msgSend((id)class_NSMenuItem, sel_alloc);
	objc_msgSend(menubaritem_app, sel_init);
//	[menubar addItem:menubaritem_app];
	objc_msgSend(menubar, sel_addItem, menubaritem_app);
//	[NSApp setMainMenu:menubar];
	objc_msgSend(NSApp, sel_setMainMenu, menubar);
//	id menu_app = [[NSMenu alloc] init];
	id menu_app = objc_msgSend((id)class_NSMenu, sel_alloc);
	objc_msgSend(menu_app, sel_init);


	// add the menu items
//	id menuitem_about = [[NSMenuItem alloc] initWithTitle:str_about_app action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:str_empty];
	id menuitem_about = objc_msgSend((id)class_NSMenuItem, sel_alloc);	
	objc_msgSend(menuitem_about, sel_initWithTitle_action_keyEquivalent, str_about_app, sel_orderFrontStandardAboutPanel, str_empty);
//	[menu_app addItem:menuitem_about];
	objc_msgSend(menu_app, sel_addItem, menuitem_about);

//	[menu_app addItem:[NSMenuItem separatorItem]];
	id separator1 = objc_msgSend((id)class_NSMenuItem, sel_separatorItem);
	objc_msgSend(menu_app, sel_addItem, separator1);

//	id menuitem_fullscreen = [[NSMenuItem alloc] initWithTitle:str_empty action:@selector(toggleFullScreen:) keyEquivalent:@"f"];
	id menuitem_fullscreen = objc_msgSend((id)class_NSMenuItem, sel_alloc);	
	objc_msgSend(menuitem_fullscreen, sel_initWithTitle_action_keyEquivalent, str_empty, sel_toggleFullScreen, str_hotkey_fullscreen);
//	[menuitem_fullscreen setKeyEquivalentModifierMask:NSEventModifierFlagCommand | NSEventModifierFlagControl];
	objc_msgSend(menuitem_fullscreen, sel_setKeyEquivalentModifierMask, (1 << 20) | (1 << 18));
//	[menu_app addItem:menuitem_fullscreen];
	objc_msgSend(menu_app, sel_addItem, menuitem_fullscreen);

//	[menu_app addItem:[NSMenuItem separatorItem]];
	id separator2 = objc_msgSend((id)class_NSMenuItem, sel_separatorItem);
	objc_msgSend(menu_app, sel_addItem, separator2);

//	id menuitem_quit = [[NSMenuItem alloc] initWithTitle:str_quit_app action:@selector(terminate:) keyEquivalent:@"q"];
	id menuitem_quit = objc_msgSend((id)class_NSMenuItem, sel_alloc);	
	objc_msgSend(menuitem_quit, sel_initWithTitle_action_keyEquivalent, str_quit_app, sel_terminate, str_hotkey_quit);
//	[menu_app addItem:menuitem_quit];
	objc_msgSend(menu_app, sel_addItem, menuitem_quit);


//	[menubaritem_app setSubmenu:menu_app];
	objc_msgSend(menubaritem_app, sel_setSubmenu, menu_app);
}


NSUInteger applicationShouldTerminate(id self, SEL _sel, id sender)
{
//	log_debug("AppDelegate:applicationsShouldTerminate");
	if( ! killme )
		log_info("Shutdown on : App Close");
	killme = 1;

	return 1; // = NSTerminateNow
}

_Bool windowShouldClose(id self, SEL _sel, id notification)
{
//	log_debug("WindowDelegate:windowShouldClose");
	log_info("Shutdown on : Window close");
	killme = 1;
	return true;
}


void windowDidResize(id self, SEL _sel, id nofitication)
{
//	log_debug("WindowDelegate:windowDidResize");
	SEL sel_backingScaleFactor = sel_registerName("backingScaleFactor");
	SEL sel_contentView = sel_registerName("contentView");
//	sys_dpi = [window backingScaleFactor];
	sys_dpi = ((CGFloat (*)(id, SEL))objc_msgSend)(window, sel_backingScaleFactor);
//	CGSize box = [window contentView].frame.size;
	id view = objc_msgSend(window, sel_contentView);
	SEL sel_frame = sel_registerName("frame");
	CGRect r = ((CGRect (*)(id, SEL))objc_msgSend_stret)(view, sel_frame);

	vid_width = r.size.width * sys_dpi;
	vid_height = r.size.height * sys_dpi;
	gfx_resize();
}


SEL sel_type;
SEL sel_buttonNumber;
SEL sel_keyCode;
SEL sel_modifierFlags;

void handle_event(id event)
{
// https://developer.apple.com/library/mac/documentation/Cocoa/Reference/ApplicationKit/Classes/NSEvent_Class/#//apple_ref/c/tdef/NSEventType

	NSUInteger event_type = ((NSUInteger (*)(id, SEL))objc_msgSend)(event, sel_type);
// event codes are defined in...
// /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/AppKit.framework/Headers/NSEvent.h
	int bit=0;
	switch(event_type) {
	case 1: // NSEventTypeLeftMouseDown:
		bit = 1;
	case 2: // NSEventTypeLeftMouseUp:
		mouse[0] = bit;
//		mouse_move(theEvent);
		break;
	case 3: // NSEventTypeRightMouseDown:
		bit = 1;
	case 4: // NSEventTypeRightMouseUp:
		mouse[1] = bit;
//		mouse_move(theEvent);
		break;
	case 25: // NSEventTypeOtherMouseDown:
		bit = 1;
	case 26: // NSEventTypeOtherMouseUp:
		{
			NSUInteger button_number = ((NSUInteger (*)(id, SEL))objc_msgSend)(event, sel_buttonNumber);

			switch(button_number) {
			case 2: mouse[2] = bit; break;
			case 3: mouse[3] = bit; break;
			case 4: mouse[4] = bit; break;
			case 5: mouse[5] = bit; break;
			case 6: mouse[6] = bit; break;
			case 7: mouse[7] = bit; break;
			default: log_debug("Unexpected Mouse Button %d", button_number); break;
			}
//			mouse_move(theEvent);
		}
		break;

	case 5: // NSEventTypeMouseMoved:
	case 6: // NSEventTypeLeftMouseDragged:
	case 7: // NSEventTypeRightMouseDragged:
	case 27: // NSEventTypeOtherMouseDragged:
//		mouse_move(theEvent);
		break;
	case 22: //NSEventTypeScrollWheel:
		break;

	case 8: // NSEventTypeMouseEntered:
	case 9: // NSEventTypeMouseExited:
		mouse[0] = 0;
		break;

	case 10: // NSEventTypeKeyDown:
		bit=1;
	case 11: // NSEventTypeKeyUp:
		{
			NSUInteger key_code = ((NSUInteger (*)(id, SEL))objc_msgSend)(event, sel_keyCode);
			keys[key_code] = bit;
		}
		break;

	case 12: // NSEventTypeFlagsChanged:
		{
			NSUInteger modifier_flags = ((NSUInteger (*)(id, SEL))objc_msgSend)(event, sel_modifierFlags);
			for(int i = 0; i<24; i++)
			{
				int bit = !!(modifier_flags & (1 << i));
				switch(i) {
				case   0: keys[KEY_LCONTROL] = bit; break;
				case   1: keys[KEY_LSHIFT] = bit; break;
				case   2: keys[KEY_RSHIFT] = bit; break;
				case   3: keys[KEY_LLOGO] = bit; break;
				case   4: keys[KEY_RLOGO] = bit; break;
				case   5: keys[KEY_LALT] = bit; break;
				case   6: keys[KEY_RALT] = bit; break;
				case   8: break; // Always on?
				case  13: keys[KEY_RCONTROL] = bit; break;
				case  16: keys[KEY_CAPSLOCK] = bit; break;
				case  17: break; // AllShift
				case  18: break; // AllCtrl
				case  19: break; // AllAlt
				case  20: break; // AllLogo
				case  23: keys[KEY_FN] = bit; break;
				default: break;
				}
			}
		}
		break;

	default:
		break;
	}
}


int main(int argc, char * argv[])
{
	log_init();
	log_info("Platform    : MacOS");

	memset(keys, 0, KEYMAX);

	SEL sel_alloc = sel_registerName("alloc");
	SEL sel_init = sel_registerName("init");

//	[[NSAutoreleasePool alloc] init];
	Class class_NSAutoreleasePool = objc_getClass("NSAutoreleasePool");
	id pool = objc_msgSend((id)class_NSAutoreleasePool, sel_alloc);
	objc_msgSend(pool, sel_init);
	
//	[NSApplication sharedApplication];
	Class class_NSApplication = objc_getClass("NSApplication");
	SEL sel_sharedApplication = sel_registerName("sharedApplication");
	objc_msgSend((id)class_NSApplication, sel_sharedApplication);

//	AppDelegate * appd = [[AppDelegate alloc] init];
	Class class_NSObject = objc_getClass("NSObject");
	Class class_AppDelegate = objc_allocateClassPair(class_NSObject, "AppDelegate", 0);
	Protocol *protocol_NSApplicationDelegate = objc_getProtocol("NSApplicationDelegate");
	int result = class_addProtocol(class_AppDelegate, protocol_NSApplicationDelegate);
	if(!result)
	{
		log_error("AppDelegate creation");
		return 1;
	}
	SEL sel_applicationShouldTerminate = sel_registerName("applicationShouldTerminate:");
	result = class_addMethod(class_AppDelegate, sel_applicationShouldTerminate, (IMP)applicationShouldTerminate, "L@:@");
	if(!result)
	{
		log_error("AppDelegate:applicationShouldTerminate creation");
		return 1;
	}

	id app_delegate = objc_msgSend((id)class_AppDelegate, sel_alloc);
	objc_msgSend(app_delegate, sel_init);

//	[NSApp setDelegate:appd];
	SEL sel_setDelegate = sel_registerName("setDelegate:");
	objc_msgSend(NSApp, sel_setDelegate, app_delegate);


	// Need to do this because we're not using [NSApp run];
//	[NSApp finishLaunching];
	SEL sel_finishLaunching = sel_registerName("finishLaunching");
	objc_msgSend(NSApp, sel_finishLaunching);

	sys_menu_init();

//	NSRect contentSize = NSMakeRect(10.0, 500.0, 640.0, 360.0);
//	NSUInteger windowStyleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskResizable | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
//	window = [[NSWindow alloc] initWithContentRect:contentSize styleMask:windowStyleMask backing:NSBackingStoreBuffered defer:NO];
	CGRect content_size = { {10, 500}, {640, 360} };
	uint32_t style_mask = 15; // (1<<0) | (1<<3) | (1<<1) | (1<<2)
	Class class_NSWindow = objc_getClass("NSWindow");
	window = objc_msgSend((id)class_NSWindow, sel_alloc);
	SEL sel_initWithContentRect_styleMask_backing_defer = sel_registerName("initWithContentRect:styleMask:backing:defer:");
	objc_msgSend(window, sel_initWithContentRect_styleMask_backing_defer, content_size, style_mask, 2, NO); // 2 = NSBackingStoreBuffered

//	WindowDelegate * wdg = [[WindowDelegate alloc] init];
//	[window setDelegate: wdg];
	Class class_WindowDelegate = objc_allocateClassPair(class_NSObject, "AppDelegate", 0);
	Protocol *protocol_NSWindowDelegate = objc_getProtocol("NSWindowDelegate");
	result = class_addProtocol(class_WindowDelegate, protocol_NSWindowDelegate);
	if(!result)
	{
		log_error("WindowDelegate creation");
		return 1;
	}
	SEL sel_windowShouldClose = sel_registerName("windowShouldClose:");
	result = class_addMethod(class_WindowDelegate, sel_windowShouldClose, (IMP)windowShouldClose, "B@:");
	if(!result)
	{
		log_error("WindowDelegate:windowShouldClose creation");
		return 1;
	}
	SEL sel_windowDidResize = sel_registerName("windowDidResize:");
	result = class_addMethod(class_WindowDelegate, sel_windowDidResize, (IMP)windowDidResize, "v@:");
	if(!result)
	{
		log_error("WindowDelegate:windowDidResize creation");
		return 1;
	}

	id window_delegate = objc_msgSend((id)class_WindowDelegate, sel_alloc);
	objc_msgSend(window_delegate, sel_init);
	objc_msgSend(window, sel_setDelegate, window_delegate);

//	id str_appname = [[NSProcessInfo processInfo] processName];
	Class class_NSProcessInfo = objc_getClass("NSProcessInfo");
	SEL sel_processInfo = sel_registerName("processInfo");
	SEL sel_processName = sel_registerName("processName");
	id obj_processInfo = objc_msgSend((id)class_NSProcessInfo, sel_processInfo);
	id str_appname = objc_msgSend(obj_processInfo, sel_processName);

//	[window setTitle: str_appname];
	SEL sel_setTitle = sel_registerName("setTitle:");
	objc_msgSend(window, sel_setTitle, str_appname);

//	[window makeKeyAndOrderFront:window];
	SEL sel_makeKeyAndOrderFront = sel_registerName("makeKeyAndOrderFront:");
	objc_msgSend(window, sel_makeKeyAndOrderFront, window);

	// brings app to the front
//	[NSApp activateIgnoringOtherApps:YES];
	SEL sel_activateIgnoringOtherApps = sel_registerName("activateIgnoringOtherApps:");
	objc_msgSend(NSApp, sel_activateIgnoringOtherApps, YES);

	// Use a CVDisplayLink to toggle a mutex, to allow VSync
	pthread_mutex_lock(&mutex_vsync);
	CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
	CVDisplayLinkSetOutputCallback(_displayLink, &DisplayLinkCallback, NULL);
	CVDisplayLinkStart(_displayLink);

	osx_view_init();
	if( main_init(argc, argv) )
	{
		killme = 1;
		log_info("Shutdown on : Init Failed");
	}

//	[NSApp run];

	SEL sel_nextEventMatchingMask_untilDate_inMode_dequeue = sel_registerName("nextEventMatchingMask:untilDate:inMode:dequeue:");
	Class class_NSDate = objc_getClass("NSDate");
	SEL sel_distantPast = sel_registerName("distantPast");
	SEL sel_sendEvent = sel_registerName("sendEvent:");
	id distant_past = objc_msgSend((id)class_NSDate, sel_distantPast);

	// used in handle_event()
	sel_type = sel_registerName("type");
	sel_buttonNumber = sel_registerName("buttonNumber");
	sel_keyCode = sel_registerName("keyCode");
	sel_modifierFlags = sel_registerName("modifierFlags");

	SEL sel_mouseLocationOutsideofEventStream = sel_registerName("mouseLocationOutsideOfEventStream");
	SEL sel_convertRectToBacking = sel_registerName("convertRectToBacking:");
	SEL sel_backingScaleFactor = sel_registerName("backingScaleFactor");

	while(!killme)
	{
		while(1)
		{
			id event = objc_msgSend(NSApp, sel_nextEventMatchingMask_untilDate_inMode_dequeue, NSUIntegerMax, distant_past, NSDefaultRunLoopMode, YES);
			if(!event)break;
			handle_event(event);
			objc_msgSend(NSApp, sel_sendEvent, event);
		}
		
		// get the current mouse position
//		NSPoint p = [window mouseLocationOutsideOfEventStream];
		CGPoint p = ((CGPoint (*)(id, SEL))objc_msgSend)(window, sel_mouseLocationOutsideofEventStream);
		CGRect r = {p.x, p.y, 0, 0};
//		r = [window convertRectToBacking:r];
		r = ((CGRect (*)(id, SEL, CGRect))objc_msgSend_stret)(window, sel_convertRectToBacking, r);

		// get the current scaling setting, incase it has been changed
		sys_dpi = ((CGFloat (*)(id, SEL))objc_msgSend)(window, sel_backingScaleFactor);

		// convert the mouse position to what is expected
		p.x = r.origin.x;
		p.y = vid_height - r.origin.y;
		mickey_x -= p.x - mouse_x;
		mickey_y -= p.y - mouse_y;
		mouse_x = p.x;
		mouse_y = p.y;


		main_loop();
	}
	CVDisplayLinkStop(_displayLink);

	main_end();

//	[NSApp setDelegate:nil];
	objc_msgSend(NSApp, sel_setDelegate, nil);
	return 0;
}
