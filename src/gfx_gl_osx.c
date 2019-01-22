

#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#include <OpenGL/OpenGL.h>

//#define OLD_OPENGL	// or use a modern context


#include "global.h"
#include "log.h"


#include <objc/objc.h>
#include <objc/message.h>
#include <CoreGraphics/CGBase.h>
#include <CoreGraphics/CGGeometry.h>
#include <OpenGL/CGLTypes.h>
#include <CoreVideo/CVReturn.h>
#include <CoreVideo/CVDisplayLink.h>

extern id NSApp;

CVDisplayLinkRef _displayLink;
extern id window;
static id ns_view;


char* ns_str(id item)
{
	SEL sel_description = sel_registerName("description");
	SEL sel_cStringUsingEncoding = sel_registerName("cStringUsingEncoding:");
	// NSUTF8StringEncoding = 4
	id desc = objc_msgSend(item, sel_description);
	return (char*)objc_msgSend(desc, sel_cStringUsingEncoding, 4);
}


CVReturn DisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
//	log_debug("View:DisplayLinkCallback");
	CGLError result;

//	NSOpenGLContext *glcontext = [ns_view openGLContext];
	SEL openGLContext = sel_registerName("openGLContext");
	id glcontext = objc_msgSend(ns_view, openGLContext);

//	CGLContextObj context = [glcontext CGLContextObj];
	SEL sel_CGLContextObj = sel_registerName("CGLContextObj");
	CGLContextObj context = (CGLContextObj)objc_msgSend(glcontext, sel_CGLContextObj);

//	log_warning("pf = %s", ns_str(pixelFormat));

	result = CGLLockContext(context);
	if(result != 0)
	{
		log_error("CGLLockContext() = %s", CGLErrorString(result));
	}

//	[glcontext makeCurrentContext];
	SEL makeCurrentContext = sel_registerName("makeCurrentContext");
	objc_msgSend(glcontext, makeCurrentContext);
	
	main_loop();

//	[glcontext flushBuffer];
	SEL flushBuffer = sel_registerName("flushBuffer");
	objc_msgSend(glcontext, flushBuffer);


	result = CGLUnlockContext(context);
	if(result != 0)
	{
		log_error("CGLLockContext() = %s", CGLErrorString(result));
	}

	if(killme != 0)
	{
//		[NSApp terminate:nil];
		SEL terminate = sel_registerName("terminate:");
		objc_msgSend(NSApp, terminate, 0);
	}

	return kCVReturnSuccess;
}

void osx_view_init(void)
{


// /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/AppKit.framework/Headers/NSOpenGL.h
	uint32_t pixelFormatAttributes[] = {
#ifndef OLD_OPENGL
		99, 0x4100, //NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
//		99, 0x3200, //NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
#else
		99, 0x1000, //NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy,
#endif
		8, 24,	//NSOpenGLPFAColorSize    , 24,
		11, 8,	//NSOpenGLPFAAlphaSize    , 8,
		12, 24,	//NSOpenGLPFADepthSize    , 24,
		5,	//NSOpenGLPFADoubleBuffer ,
		73,	//NSOpenGLPFAAccelerated  ,
		72,	//NSOpenGLPFANoRecovery   ,
		0
	};

//	NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:pixelFormatAttributes];
	SEL alloc = sel_registerName("alloc");
	SEL initWithAttributes = sel_registerName("initWithAttributes:");
	Class NSOpenGLPixelFormatClass = objc_getClass("NSOpenGLPixelFormat");
	id pixelFormatAlloc = objc_msgSend((id)NSOpenGLPixelFormatClass, alloc);
	id glpixelformat = objc_msgSend(pixelFormatAlloc, initWithAttributes, pixelFormatAttributes);

//	NSOpenGLView *view = [[NSOpenGLView alloc] initWithFrame:[[window contentView] bounds] pixelFormat:glpixelformat];
//	id gl_view_alloc = [NSOpenGLView alloc];
	Class NSOpenGLViewClass = objc_getClass("NSOpenGLView");
	id gl_view_alloc = objc_msgSend((id)NSOpenGLViewClass, alloc);

//	CGRect window_bounds = [window contentView];
	SEL contentView = sel_registerName("contentView");
	id window_contentview = objc_msgSend(window, contentView);

//	CGRect window_bounds = [window_contentview bounds];
	SEL bounds = sel_registerName("bounds");
	CGRect window_bounds = ((CGRect (*)(id, SEL))objc_msgSend_stret)(window_contentview, bounds);

//	NSOpenGLView *view = [gl_view_alloc initWithFrame:window_bounds pixelFormat:glpixelformat];
	SEL initWithFramePixelFormat = sel_registerName("initWithFrame:pixelFormat:");
	id view = objc_msgSend(gl_view_alloc, initWithFramePixelFormat, window_bounds, glpixelformat);


/*	SEL initWithFramePixelFormat = sel_registerName("initWithFrame:pixelFormat:");
	SEL contentView = sel_registerName("contentView");
	SEL bounds = sel_registerName("bounds");
	id gl_view_alloc = objc_msgSend(NSOpenGLViewClass, alloc);
	id window_contentview = objc_msgSend(window, contentView);
	CGRect window_bounds = objc_msgSend(window_contentview, bounds);
	id view = objc_msgSend(gl_view_alloc, initWithFramePixelFormat, window_bounds, glpixelformat);
*/


//	[window setContentView:view];
	SEL setContentView = sel_registerName("setContentView:");
	objc_msgSend(window, setContentView, view);

	ns_view = view;

//	log_warning("view = %s", ns_str(view));
	GLint vsync = 0;

//	[view setWantsBestResolutionOpenGLSurface:YES];   // enable retina resolutions
	SEL setWantsBestResolutionOpenGLSurface = sel_registerName("setWantsBestResolutionOpenGLSurface:");
	objc_msgSend(view, setWantsBestResolutionOpenGLSurface, YES);

//	sys_dpi = [window backingScaleFactor];
//	[[view openGLContext] setValues:&vsync forParameter:NSOpenGLCPSwapInterval];
//	[[view openGLContext] setValues:&vsync forParameter:NSOpenGLContextParameterSwapInterval];


//	CGLContextObj cglContext = [[view openGLContext] CGLContextObj];
	SEL openGLContext = sel_registerName("openGLContext");
	SEL sel_CGLContextObj = sel_registerName("CGLContextObj");
	id view_glcontext = objc_msgSend(view, openGLContext);
	CGLContextObj cglContext = (CGLContextObj)objc_msgSend(view_glcontext, sel_CGLContextObj);
//	CGLPixelFormatObj cglPixelFormat = [glpixelformat CGLPixelFormatObj];
	SEL sel_CGLPixelFormatObj = sel_registerName("CGLPixelFormatObj");
	CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)objc_msgSend(glpixelformat, sel_CGLPixelFormatObj);


	CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(_displayLink, cglContext, cglPixelFormat);

//	CVDisplayLinkStart(_displayLink);
}


void gfx_init(void)
{
}

void gfx_end(void)
{
}

extern int y_correction;  // to correct mouse position for title bar


void gfx_resize(void)
{
	glViewport(0, y_correction, vid_width, vid_height);
}


void gfx_swap(void)
{
}
