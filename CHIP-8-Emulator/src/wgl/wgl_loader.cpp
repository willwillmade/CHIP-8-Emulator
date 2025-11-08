#include "wgl_loader.h"

PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = nullptr;

bool setup_wgl_functions()
{
	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	return wglSwapIntervalEXT != nullptr;
}