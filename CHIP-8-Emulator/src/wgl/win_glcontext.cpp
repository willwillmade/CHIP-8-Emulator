#include "win_glcontext.h"
#include <tchar.h>
#include <glad/glad.h>

HGLRC create_gl_context(HWND hWnd, HDC hDC)
{
	PIXELFORMATDESCRIPTOR pfd = {};
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = pfd.cRedShift = pfd.cGreenBits = pfd.cGreenShift = pfd.cBlueBits = pfd.cBlueShift = pfd.cAlphaBits = pfd.cAlphaShift = 0;
	pfd.cAccumBits = pfd.cAccumRedBits = pfd.cAccumGreenBits = pfd.cAccumBlueBits = pfd.cAccumAlphaBits = 0;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.cAuxBuffers = 0;
	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.bReserved = 0;
	pfd.dwLayerMask = pfd.dwVisibleMask = pfd.dwDamageMask = 0;

	//HDC hDC = GetDC(hWnd);

	int pf = ChoosePixelFormat(hDC, &pfd);
	bool skip = !pf;
	if (skip) {
		MessageBox(NULL, _T("Cannot find a suitable pixel format."), _T("Error"), MB_OK);
	}
	skip = !skip && !SetPixelFormat(hDC, pf, &pfd);
	if (skip) {
		MessageBox(NULL, _T("Cannot find set format specified."), _T("Error"), MB_OK);
	}

	HGLRC glContext = wglCreateContext(hDC);
	return glContext;
}