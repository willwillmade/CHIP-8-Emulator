#ifndef WGL_LOADER
#define WGL_LOADER

#if _MSC_VER >= 1910 && _MSC_VER <= 1916
#define no_init_all deprecated
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include <windows.h>
#include "glad/glad.h"
#include "wglext.h"

extern PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

bool setup_wgl_functions();

#endif // WGL_LOADER