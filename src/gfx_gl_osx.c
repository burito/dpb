

#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
//#include <OpenGL/OpenGL.h>

//#define OLD_OPENGL	// or use a modern context


#include "global.h"
#include "log.h"

#include <pthread.h>

#include <objc/objc.h>
#include <objc/message.h>
#include <CoreGraphics/CGBase.h>
#include <CoreGraphics/CGGeometry.h>
#include <OpenGL/CGLTypes.h>
#include <CoreVideo/CVReturn.h>
#include <CoreVideo/CVDisplayLink.h>

extern id NSApp;

extern id window;
static id ns_view;
static id openGLcontext;

pthread_mutex_t mutex_vsync = PTHREAD_MUTEX_INITIALIZER;
CVDisplayLinkRef _displayLink;

CVReturn DisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
	pthread_mutex_unlock(&mutex_vsync);
	usleep(1000);
	pthread_mutex_lock(&mutex_vsync);
	return kCVReturnSuccess;
}

void osx_view_init(void)
{
// /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/AppKit.framework/Headers/NSOpenGL.h
	uint32_t pixelFormatAttributes[] = {
#ifdef OLD_OPENGL
		99, 0x1000, //NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy,
#else
		99, 0x4100, //NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
//		99, 0x3200, //NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
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

//	[window setContentView:view];
	SEL setContentView = sel_registerName("setContentView:");
	objc_msgSend(window, setContentView, view);

	ns_view = view;

//	log_warning("view = %s", ns_str(view));

//	[view setWantsBestResolutionOpenGLSurface:YES];   // enable retina resolutions
	SEL setWantsBestResolutionOpenGLSurface = sel_registerName("setWantsBestResolutionOpenGLSurface:");
	objc_msgSend(view, setWantsBestResolutionOpenGLSurface, YES);

//	CGLContextObj cglContext = [[view openGLContext] CGLContextObj];
	SEL sel_openGLContext = sel_registerName("openGLContext");
	openGLcontext = objc_msgSend(view, sel_openGLContext);

}

SEL sel_flushBuffer;
void gfx_init(void)
{
	sel_flushBuffer = sel_registerName("flushBuffer");
	// Use a CVDisplayLink to toggle a mutex, to allow VSync
	pthread_mutex_lock(&mutex_vsync);
	CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
	CVDisplayLinkSetOutputCallback(_displayLink, &DisplayLinkCallback, NULL);
	CVDisplayLinkStart(_displayLink);
}

void gfx_end(void)
{
	CVDisplayLinkStop(_displayLink);
}

void gfx_resize(void)
{
	glViewport(0, 0, vid_width, vid_height);
}

void gfx_swap(void)
{
#ifndef VSYNC_OFF
	pthread_mutex_lock(&mutex_vsync);
	pthread_mutex_unlock(&mutex_vsync);
#endif
	objc_msgSend(openGLcontext, sel_flushBuffer);
}
