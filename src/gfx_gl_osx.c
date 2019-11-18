

#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
//#include <OpenGL/OpenGL.h>

//#define OLD_OPENGL	// or use a modern context

#define LOG_NO_DEBUG

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

extern char* ns_str(id);

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
	Class class_NSOpenGLPixelFormat = objc_getClass("NSOpenGLPixelFormat");
	id pixelFormatAlloc = ((id (*)(Class, SEL))objc_msgSend)(class_NSOpenGLPixelFormat, alloc);
	id glpixelformat = ((id (*)(id, SEL, const uint32_t*))objc_msgSend)(pixelFormatAlloc, initWithAttributes, pixelFormatAttributes);
	log_debug("glpixelformat = %s", ns_str(glpixelformat));

//	NSOpenGLView *view = [[NSOpenGLView alloc] initWithFrame:[[window contentView] bounds] pixelFormat:glpixelformat];
//	id gl_view_alloc = [NSOpenGLView alloc];
	Class class_NSOpenGLView = objc_getClass("NSOpenGLView");
	id gl_view_alloc = ((id (*)(Class, SEL))objc_msgSend)(class_NSOpenGLView, alloc);
//	CGRect window_bounds = [window contentView];
	SEL contentView = sel_registerName("contentView");
	id window_contentview = ((id (*)(id, SEL))objc_msgSend)(window, contentView);
//	CGRect window_bounds = [window_contentview bounds];
	SEL bounds = sel_registerName("bounds");
	CGRect window_bounds = ((CGRect (*)(id, SEL))objc_msgSend_stret)(window_contentview, bounds);
//	NSOpenGLView *view = [gl_view_alloc initWithFrame:window_bounds pixelFormat:glpixelformat];
	SEL initWithFramePixelFormat = sel_registerName("initWithFrame:pixelFormat:");
	id view = ((id (*)(id, SEL, CGRect, id))objc_msgSend)(gl_view_alloc, initWithFramePixelFormat, window_bounds, glpixelformat);
	log_debug("view = %s", ns_str(view));

//	[window setContentView:view];
	SEL setContentView = sel_registerName("setContentView:");
	((id (*)(id, SEL, id))objc_msgSend)(window, setContentView, view);

	ns_view = view;

//	[view setWantsBestResolutionOpenGLSurface:YES];   // enable retina resolutions
	SEL setWantsBestResolutionOpenGLSurface = sel_registerName("setWantsBestResolutionOpenGLSurface:");
	((id (*)(id, SEL, BOOL))objc_msgSend)(view, setWantsBestResolutionOpenGLSurface, YES);

//	CGLContextObj cglContext = [[view openGLContext] CGLContextObj];
	SEL sel_openGLContext = sel_registerName("openGLContext");
	openGLcontext = ((id (*)(id, SEL))objc_msgSend)(view, sel_openGLContext);
	log_debug("openGLcontext = %s", ns_str(openGLcontext));

	// do we need to do this?
//	SEL sel_update = sel_registerName("update");
//	((void (*)(id, SEL))objc_msgSend)(openGLcontext, sel_update);

	// we definitely need to do this
	SEL sel_makeCurrent = sel_registerName("makeCurrentContext");
	((void (*)(id, SEL))objc_msgSend)(openGLcontext, sel_makeCurrent);
}

SEL sel_flushBuffer;
void gfx_init(void)
{
	CVReturn ret;
	sel_flushBuffer = sel_registerName("flushBuffer");
	// Use a CVDisplayLink to toggle a mutex, to allow VSync
	pthread_mutex_lock(&mutex_vsync);
	ret = CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
	if(ret != kCVReturnSuccess){
		log_error("CVDisplayLinkCreateWithActiveCGDisplays() = %d", ret);
	}
	ret = CVDisplayLinkSetOutputCallback(_displayLink, &DisplayLinkCallback, NULL);
	if(ret != kCVReturnSuccess){
		log_error("CVDisplayLinkSetOutputCallback() = %d", ret);
	}
	ret = CVDisplayLinkStart(_displayLink);
	if(ret != kCVReturnSuccess){
		log_error("CVDisplayLinkStart() = %d", ret);
	}


	SEL sel_bounds = sel_registerName("bounds");
	SEL sel_convert = sel_registerName("convertRectToBacking:");

	CGRect r = ((CGRect (*)(id, SEL))objc_msgSend_stret)(ns_view, sel_bounds);
	CGRect ns_actual_rect = ((CGRect (*)(id, SEL, CGRect))objc_msgSend_stret)(ns_view, sel_convert, r);

	vid_width = ns_actual_rect.size.width;
	vid_height = ns_actual_rect.size.height;

}

void gfx_end(void)
{
	CVReturn ret;
	ret = CVDisplayLinkStop(_displayLink);
	if(ret != kCVReturnSuccess){
		log_error("CVDisplayLinkStop() = %d", ret);
	}
	
}

void gfx_resize(void)
{
	SEL sel_frame = sel_registerName("frame");
	SEL sel_convert = sel_registerName("convertRectToBacking:");

	CGRect r = ((CGRect (*)(id, SEL))objc_msgSend_stret)(ns_view, sel_frame);
	CGRect ns_actual_rect = ((CGRect (*)(id, SEL, CGRect))objc_msgSend_stret)(ns_view, sel_convert, r);

	vid_width = ns_actual_rect.size.width;
	vid_height = ns_actual_rect.size.height;

	glViewport(0, 0, vid_width, vid_height);
}

void gfx_swap(void)
{
#ifndef VSYNC_OFF
	pthread_mutex_lock(&mutex_vsync);
	pthread_mutex_unlock(&mutex_vsync);
#endif
	id ret = ((id (*)(id, SEL))objc_msgSend)(openGLcontext, sel_flushBuffer);
	if(ret){
		log_debug("ret = %s", ns_str(ret));
	}
}
