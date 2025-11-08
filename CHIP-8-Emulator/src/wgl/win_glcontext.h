#ifndef WIN_GLCONTEXT_H
#define WIN_GLCONTEXT_H

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

HGLRC create_gl_context(HWND hwnd, HDC hDC);

#endif // WIN_GLCONTEXT_H