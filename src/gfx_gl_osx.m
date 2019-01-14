

#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>

#import <Cocoa/Cocoa.h>

#define MODERN_OPENGL		// or use a GL2 context


#include "global.h"
#include "log.h"

CVDisplayLinkRef _displayLink;
extern NSWindow * window;
static NSView * ns_view;

static CVReturn DisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
//	log_debug("View:DisplayLinkCallback");
	CGLError result;

//	NSOpenGLView *glview= (__bridge NSOpenGLView*) displayLinkContext;
	NSOpenGLContext *glcontext = [ns_view openGLContext];
	CGLContextObj context = [glcontext CGLContextObj];

//	log_warning("glv = %s", [[glview description] cStringUsingEncoding:NSASCIIStringEncoding]);
//	log_warning("glctx = %s", [[glcontext description] cStringUsingEncoding:NSASCIIStringEncoding]);
//	log_warning("pf = %s", [[pixelFormat description] cStringUsingEncoding:NSASCIIStringEncoding]);



	result = CGLLockContext(context);
	if(result != 0)
	{
		log_error("CGLLockContext() = %s", CGLErrorString(result));
	}
	[glcontext makeCurrentContext];
	main_loop();
	[glcontext flushBuffer];
	result = CGLUnlockContext(context);
	if(result != 0)
	{
		log_error("CGLLockContext() = %s", CGLErrorString(result));
	}

	if(killme != 0)
	{
//		[NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:0.0];
		[NSApp terminate:nil];
	}


	return kCVReturnSuccess;
}

void gfx_init(void)
{

	NSOpenGLPixelFormatAttribute pixelFormatAttributes[] =
	{
#ifdef MODERN_OPENGL
		NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
#else
		NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy,
#endif
		NSOpenGLPFAColorSize    , 24,
		NSOpenGLPFAAlphaSize    , 8,
		NSOpenGLPFADepthSize    , 24,
		NSOpenGLPFADoubleBuffer ,
		NSOpenGLPFAAccelerated  ,
		NSOpenGLPFANoRecovery   ,
		0
	};
	NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:pixelFormatAttributes];

	NSOpenGLView *view = [[NSOpenGLView alloc] initWithFrame:[[window contentView] bounds] pixelFormat:pixelFormat];
	[window setContentView:view];
	ns_view = view;

//	NSLog( [ [view description] cStringUsingEncoding:typeUTF8Text] );
//	log_warning("view = %s", [[view description] cStringUsingEncoding:typeUTF8Text]);

	log_warning("view = %s", [[view description] cStringUsingEncoding:NSUTF8StringEncoding]);


	GLint vsync = 0;

//	[self setWantsBestResolutionOpenGLSurface:YES];   // enable retina resolutions
//	sys_dpi = [self.window backingScaleFactor];
//	[[view openGLContext] setValues:&vsync forParameter:NSOpenGLCPSwapInterval];
	[[view openGLContext] setValues:&vsync forParameter:NSOpenGLContextParameterSwapInterval];

	// Use a CVDisplayLink to do the render loop
	CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
	CVDisplayLinkSetOutputCallback(_displayLink, &DisplayLinkCallback, NULL);

	log_warning("ds = %s", [[_displayLink description] cStringUsingEncoding:NSUTF8StringEncoding]);

	log_warning("ctx = %s", [[[view openGLContext] description] cStringUsingEncoding:NSUTF8StringEncoding]);
	log_warning("pf = %s", [[pixelFormat description] cStringUsingEncoding:NSUTF8StringEncoding]);

	CGLContextObj cglContext = [[view openGLContext] CGLContextObj];
	CGLPixelFormatObj cglPixelFormat = [pixelFormat CGLPixelFormatObj];



	CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(_displayLink, cglContext, cglPixelFormat);

//	CVDisplayLinkStart(_displayLink);
}


void gfx_end(void)
{
	CVDisplayLinkRelease(_displayLink);
}

extern int y_correction;  // to correct mouse position for title bar


void gfx_resize(void)
{
	glViewport(0, y_correction, vid_width, vid_height);
}


void gfx_swap(void)
{

}