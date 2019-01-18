

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

#define LOG_NO_DEBUG

#include "global.h"
#include "log.h"

CVDisplayLinkRef _displayLink;
extern NSWindow * window;
static NSView * ns_view;
void * pView = NULL;

@interface View : NSView
@end

@implementation View

-(BOOL) wantsUpdateLayer { return YES; }

+(Class) layerClass { return [CAMetalLayer class]; }

-(CALayer*) makeBackingLayer
{
//	log_trace("View:makeBackingLayer");
	CALayer *layer = [self.class.layerClass layer];
	CGSize viewScale = [self convertSizeToBacking: CGSizeMake(1.0, 1.0)];
	layer.contentsScale = MIN(viewScale.width, viewScale.height);
	return layer;
}

@end


CVReturn DisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
//	log_trace("View:DisplayLinkCallback");
	main_loop();

	if(killme != 0)
	{
		[NSApp terminate:nil];
	}

	return kCVReturnSuccess;
}

void osx_view_init(void)
{
	NSView * view = [[View alloc] init];
	[window setContentView:view];
	view.wantsLayer = YES;
	pView = [view layer];
	log_debug("view = %s", [[view description] cStringUsingEncoding:NSUTF8StringEncoding]);
}

