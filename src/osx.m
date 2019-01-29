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

#import <Cocoa/Cocoa.h>

#include <objc/message.h>

#include "log.h"
#include "global.h"

///////////////////////////////////////////////////////////////////////////////
//////// Public Interface to the rest of the program
///////////////////////////////////////////////////////////////////////////////

void gfx_resize(void);
void osx_view_init(void);
CVReturn DisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext);

int fullscreen = 0;
int fullscreen_toggle = 0;

static int gargc;
static char ** gargv;
NSWindow * window;

extern CVDisplayLinkRef _displayLink;


///////////////////////////////////////////////////////////////////////////////
//////// Mac OS X OpenGL window setup
///////////////////////////////////////////////////////////////////////////////

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
	id obj_processInfo = objc_msgSend(class_NSProcessInfo, sel_processInfo);
	id str_appname = objc_msgSend(obj_processInfo, sel_processName);

	id str_about = objc_msgSend(class_NSString, sel_stringWithUTF8String, "About ");
	id str_about_app = objc_msgSend(str_about, sel_stringByAppendingString, str_appname);
	id str_quit = objc_msgSend(class_NSString, sel_stringWithUTF8String, "Quit ");
	id str_quit_app = objc_msgSend(str_quit, sel_stringByAppendingString, str_appname);
	id str_hotkey_fullscreen = objc_msgSend(class_NSString, sel_stringWithUTF8String, "f");
	id str_hotkey_quit = objc_msgSend(class_NSString, sel_stringWithUTF8String, "q");
	id str_empty = objc_msgSend(class_NSString, sel_stringWithUTF8String, "");

	// prepare the menu objects
//	id menubar = [[NSMenu alloc] init];
	id menubar = objc_msgSend(class_NSMenu, sel_alloc);
	objc_msgSend(menubar, sel_init);
//	id menubaritem_app = [[NSMenuItem alloc] init];
	id menubaritem_app = objc_msgSend(class_NSMenuItem, sel_alloc);
	objc_msgSend(menubaritem_app, sel_init);
//	[menubar addItem:menubaritem_app];
	objc_msgSend(menubar, sel_addItem, menubaritem_app);
//	[NSApp setMainMenu:menubar];
	objc_msgSend(NSApp, sel_setMainMenu, menubar);
//	id menu_app = [[NSMenu alloc] init];
	id menu_app = objc_msgSend(class_NSMenu, sel_alloc);
	objc_msgSend(menu_app, sel_init);


	// add the menu items
//	id menuitem_about = [[NSMenuItem alloc] initWithTitle:str_about_app action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:str_empty];
	id menuitem_about = objc_msgSend(class_NSMenuItem, sel_alloc);	
	objc_msgSend(menuitem_about, sel_initWithTitle_action_keyEquivalent, str_about_app, sel_orderFrontStandardAboutPanel, str_empty);
//	[menu_app addItem:menuitem_about];
	objc_msgSend(menu_app, sel_addItem, menuitem_about);

//	[menu_app addItem:[NSMenuItem separatorItem]];
	id separator1 = objc_msgSend(class_NSMenuItem, sel_separatorItem);
	objc_msgSend(menu_app, sel_addItem, separator1);

//	id menuitem_fullscreen = [[NSMenuItem alloc] initWithTitle:str_empty action:@selector(toggleFullScreen:) keyEquivalent:@"f"];
	id menuitem_fullscreen = objc_msgSend(class_NSMenuItem, sel_alloc);	
	objc_msgSend(menuitem_fullscreen, sel_initWithTitle_action_keyEquivalent, str_empty, sel_toggleFullScreen, str_hotkey_fullscreen);
//	[menuitem_fullscreen setKeyEquivalentModifierMask:NSEventModifierFlagCommand | NSEventModifierFlagControl];
	objc_msgSend(menuitem_fullscreen, sel_setKeyEquivalentModifierMask, (1 << 20) | (1 << 18));
//	[menu_app addItem:menuitem_fullscreen];
	objc_msgSend(menu_app, sel_addItem, menuitem_fullscreen);

//	[menu_app addItem:[NSMenuItem separatorItem]];
	id separator2 = objc_msgSend(class_NSMenuItem, sel_separatorItem);
	objc_msgSend(menu_app, sel_addItem, separator2);

//	id menuitem_quit = [[NSMenuItem alloc] initWithTitle:str_quit_app action:@selector(terminate:) keyEquivalent:@"q"];
	id menuitem_quit = objc_msgSend(class_NSMenuItem, sel_alloc);	
	objc_msgSend(menuitem_quit, sel_initWithTitle_action_keyEquivalent, str_quit_app, sel_terminate, str_hotkey_quit);
//	[menu_app addItem:menuitem_quit];
	objc_msgSend(menu_app, sel_addItem, menuitem_quit);


//	[menubaritem_app setSubmenu:menu_app];
	objc_msgSend(menubaritem_app, sel_setSubmenu, menu_app);
}

@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation AppDelegate


- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
{
	if( ! killme )
		log_info("Shutdown on : App Close");
	killme = 1;

	return NSTerminateNow;
}

@end


@interface WindowDelegate: NSObject<NSWindowDelegate>
@end

@implementation WindowDelegate

- (BOOL)windowShouldClose:(NSWindow *)sender;
{
	log_info("Shutdown on : Window close");
	killme = 1;
	return true;
}


- (void)windowDidResize:(NSNotification *)notification;
{
//	log_debug("WindowDelegate:windowDidResize");
	NSWindow *window = [notification object];
	sys_dpi = [window backingScaleFactor];
	CGSize box = [window contentView].frame.size;
	vid_width = box.width * sys_dpi;
	vid_height = box.height * sys_dpi;
	gfx_resize();
}

@end


@interface MyApp : NSApplication
{
}

@end


static void mouse_move(NSEvent * theEvent)
{
	NSWindow * currentWindow = [NSApp keyWindow];
	NSRect adjustFrame = [[currentWindow contentView] frame];
	NSPoint p = [currentWindow mouseLocationOutsideOfEventStream];
	NSRect r = {p.x, p.y, 0, 0};
	r = [[currentWindow contentView] convertRectToBacking:r];
	mouse_x = r.origin.x;
	mouse_y = vid_height-r.origin.y;
//	log_info("win = (%d, %d) mouse = (%d, %d)", vid_width, vid_height, mouse_x, mouse_y);
	return;
	// TODO: this is broken
	mouse_x = theEvent.locationInWindow.x * sys_dpi;
	mouse_y = vid_height - theEvent.locationInWindow.y * sys_dpi;
//	log_debug("mouse = (%d, %d) dpi = %f", mouse_x, mouse_y, sys_dpi);
	mickey_x -= theEvent.deltaX * sys_dpi;
	mickey_y -= theEvent.deltaY * sys_dpi;
}

@implementation MyApp


-(void)sendEvent:(NSEvent *)theEvent
{
// https://developer.apple.com/library/mac/documentation/Cocoa/Reference/ApplicationKit/Classes/NSEvent_Class/#//apple_ref/c/tdef/NSEventType
	int bit=0;
	switch(theEvent.type) {
	case NSEventTypeLeftMouseDown:
		bit = 1;
	case NSEventTypeLeftMouseUp:
		mouse[0] = bit;
		mouse_move(theEvent);
		break;
	case NSEventTypeRightMouseDown:
		bit = 1;
	case NSEventTypeRightMouseUp:
		mouse[1] = bit;
		mouse_move(theEvent);
		break;
	case NSEventTypeOtherMouseDown:
		bit = 1;
	case NSEventTypeOtherMouseUp:
		switch(theEvent.buttonNumber) {
		case 2: mouse[2] = bit; break;
		case 3: mouse[3] = bit; break;
		case 4: mouse[4] = bit; break;
		case 5: mouse[5] = bit; break;
		case 6: mouse[6] = bit; break;
		case 7: mouse[7] = bit; break;
		default: log_debug("Unexpected Mouse Button %d", (int)theEvent.buttonNumber); break;
		}
		mouse_move(theEvent);
		break;

	case NSEventTypeMouseMoved:
	case NSEventTypeLeftMouseDragged:
	case NSEventTypeRightMouseDragged:
	case NSEventTypeOtherMouseDragged:
		mouse_move(theEvent);
		break;
	case NSEventTypeScrollWheel:
		break;

	case NSEventTypeMouseEntered:
	case NSEventTypeMouseExited:
		mouse[0] = 0;
		break;

	default:
		break;
	}
	[super sendEvent:theEvent];
}

@end


int main(int argc, char * argv[])
{
	log_init();
	log_info("Platform    : MacOS");

	memset(keys, 0, KEYMAX);
	gargc = argc;
	gargv = argv;

	id myapp = [MyApp sharedApplication];
	AppDelegate * appd = [[AppDelegate alloc] init];
	[myapp setDelegate:appd];
	sys_menu_init();


	NSRect contentSize = NSMakeRect(10.0, 500.0, 640.0, 360.0);
	NSUInteger windowStyleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskResizable | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
	window = [[NSWindow alloc] initWithContentRect:contentSize
		styleMask:windowStyleMask
		backing:NSBackingStoreBuffered
		defer:NO];
	WindowDelegate * wdg = [[WindowDelegate alloc] init];
	[window setDelegate: wdg];

	window.backgroundColor = [NSColor whiteColor];

	Class class_NSString = objc_getClass("NSString");
	SEL sel_stringWithUTF8String = sel_registerName("stringWithUTF8String:");
	id ns_binary_name = objc_msgSend(class_NSString, sel_stringWithUTF8String, binary_name);

	[window setTitle: ns_binary_name];

	[window makeKeyAndOrderFront:window];
	[NSApp activateIgnoringOtherApps:YES];	// brings app to the front
	CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
	CVDisplayLinkSetOutputCallback(_displayLink, &DisplayLinkCallback, NULL);
	osx_view_init();
	if( main_init(argc, argv) )
	{
		killme = 1;
		log_info("Shutdown on : Init Failed");
	}
	CVDisplayLinkStart(_displayLink);
	sys_dpi = [window backingScaleFactor];

	[myapp run];
	[myapp setDelegate:nil];
	CVDisplayLinkRelease(_displayLink);

	main_end();
	return 0;
}
