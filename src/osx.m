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


@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation AppDelegate


- (void)applicationWillFinishLaunching:(NSNotification *)aNotification
{
	// Create the menu that goes on the Apple Bar
	NSMenu * mainMenu = [[NSMenu alloc] initWithTitle:@"MainMenu"];
	NSMenuItem * menuTitle;
	NSMenu * aMenu;

	menuTitle = [mainMenu addItemWithTitle:@"Apple" action:NULL keyEquivalent:@""];
	aMenu = [[NSMenu alloc] initWithTitle:@"Apple"];
	[NSApp performSelector:@selector(setAppleMenu:) withObject:aMenu];

	// generate contents of menu

	Class class_NSString = objc_getClass("NSString");
	SEL sel_stringWithUTF8String = sel_registerName("stringWithUTF8String:");
	id ns_binary_name = objc_msgSend(class_NSString, sel_stringWithUTF8String, binary_name);


	NSMenuItem * menuItem;
	menuItem = [aMenu addItemWithTitle:[NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"About", nil), ns_binary_name]
				    action:@selector(orderFrontStandardAboutPanel:)
			     keyEquivalent:@""];
	[menuItem setTarget:NSApp];
	[aMenu addItem:[NSMenuItem separatorItem]];

	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Fullscreen", nil)
				    action:@selector(toggleFullScreen:)
			     keyEquivalent:@"f"];
	[menuItem setKeyEquivalentModifierMask:NSEventModifierFlagCommand | NSEventModifierFlagControl];
	menuItem.target = nil;

	[aMenu addItem:[NSMenuItem separatorItem]];

	menuItem = [aMenu addItemWithTitle:[NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"Quit", nil), ns_binary_name]
				    action:@selector(terminate:)
			     keyEquivalent:@"q"];
	[menuItem setTarget:NSApp];

	// attach generated menu to menuitem
	[mainMenu setSubmenu:aMenu forItem:menuTitle];
	[NSApp setMainMenu:mainMenu];
}


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
	mouse_x = theEvent.locationInWindow.x * sys_dpi;
	mouse_y = vid_height - theEvent.locationInWindow.y * sys_dpi;
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

	CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
	CVDisplayLinkSetOutputCallback(_displayLink, &DisplayLinkCallback, NULL);
	osx_view_init();
	if( main_init(argc, argv) )
	{
		killme = 1;
		log_info("Shutdown on : Init Failed");
	}
	CVDisplayLinkStart(_displayLink);

	[myapp run];
	[myapp setDelegate:nil];
	CVDisplayLinkRelease(_displayLink);

	main_end();
	return 0;
}
