

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

#include "global.h"
#include "log.h"

CVDisplayLinkRef _displayLink;
extern NSWindow * window;
static NSView * ns_view;
void * pView = NULL;

@interface View : NSView
@end

@implementation View

-(id) init
{
	log_debug("View:init");
	return [super init];
}

-(BOOL) wantsUpdateLayer {
	log_debug("View:wantsUpdateLayer");
	return YES; }

+(Class) layerClass { 
	log_debug("View:layerClass");
	return [CAMetalLayer class]; }

-(CALayer*) makeBackingLayer
{
	log_debug("View:makeBackingLayer");
	CALayer *layer = [self.class.layerClass layer];
	CGSize viewScale = [self convertSizeToBacking: CGSizeMake(1.0, 1.0)];
	layer.contentsScale = MIN(viewScale.width, viewScale.height);
	return layer;
}

@end



static CVReturn DisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
//	log_debug("View:DisplayLinkCallback");
//	log_warning("pf = %s", [[pixelFormat description] cStringUsingEncoding:NSASCIIStringEncoding]);
	main_loop();

	if(killme != 0)
	{
//		[NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:0.0];
		[NSApp terminate:nil];
	}

	return kCVReturnSuccess;
}

void gfx_init(void)
{
	NSView * view = [[View alloc] init];
	[window setContentView:view];
	view.wantsLayer = YES;
	pView = [view layer];

//	NSLog( [ [view description] cStringUsingEncoding:typeUTF8Text] );
//	log_warning("view = %s", [[view description] cStringUsingEncoding:typeUTF8Text]);

	log_warning("view = %s", [[view description] cStringUsingEncoding:NSUTF8StringEncoding]);

	CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
	CVDisplayLinkSetOutputCallback(_displayLink, &DisplayLinkCallback, NULL);
}


void gfx_end(void)
{
	CVDisplayLinkRelease(_displayLink);
}
