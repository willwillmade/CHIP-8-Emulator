#if _MSC_VER >= 1910 && _MSC_VER <= 1916
// E1097 unknown attribute "no_init_all"
// https://developercommunity.visualstudio.com/t/vs-2017-1592-reports-unknown-attribute-no-init-all/387702
#define no_init_all deprecated
#endif

#include "winmain.h"

#include <commdlg.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cstdlib>
#include <string>

#include "resource.h"
#include "win_glcontext.h"

#include "wgl_loader.h"


using std::vector;
using std::fill;
using std::begin;
using std::wstring;
using std::cout;
using std::endl;
using std::cerr;

#pragma region Platform
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PTSTR pCmdLine, int nCmdShow)
{
	WNDCLASSEX wc;
	HWND hWnd;

	if (!hPrevInstance) {
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_OWNDC;
		wc.lpfnWndProc = wnd_proc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)::GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = className;
		wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

		if (!RegisterClassEx(&wc)) {
			MessageBox(NULL, _T("Window Registration Failed!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
			return 0;
		}
	}

	AppData app;
	SetProcessDPIAware();
	int clientW = Chip8::DISPLAY_W * 10, clientH = Chip8::DISPLAY_H * 10;
	RECT rc = { 0, 0, clientW, clientH };
	AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, TRUE, WS_EX_CLIENTEDGE);
	hWnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		className,
		className,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
		NULL, NULL, hInstance, &app);
	if (hWnd == NULL)
	{
		MessageBox(NULL, _T("Window Creation Failed!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	add_menu(hWnd);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	const int comp = 4;
	GLubyte pixels[Chip8::DISPLAY_H * Chip8::DISPLAY_W * comp];
	fill(begin(pixels), begin(pixels) + Chip8::DISPLAY_H * Chip8::DISPLAY_W * comp, 0);
	int pixelNum = 0;

	GLuint screenTextureID = 0;
	GLuint screenVBO = 0;

	HDC hDC = GetDC(hWnd);
	HGLRC glContext = create_gl_context(hWnd, hDC);
	wglMakeCurrent(hDC, glContext);
	bool glSupported = gladLoadGL();
	if (glSupported) {
		bool wglLoaded = setup_wgl_functions();
		if (!wglSwapIntervalEXT(0)) {
			DWORD error = GetLastError();
			// ERROR_INVALID_DATA or ERROR_DC_NOT_FOUND
			output_debug_string_f(_T("wglSwapIntervalEXT failed with arg(%d): %d\n"), 1, error);
		}
		setup_gl();

		GLfloat vertices[] = {
			0, 0, 0, 1,
			1, 0, 1, 1,
			1, 1, 1, 0,
			0, 1, 0, 0
		};
		GLfloat texCoords[] = {
			0.0f, 1.0f,
			1.0f, 1.0f,
			1.0f, 0.0f,
			0.0f, 0.0f
		};
		glGenBuffers(1, &screenVBO);
		glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glGenTextures(1, &screenTextureID);
		set_texture_paramaters(screenTextureID, GL_CLAMP_TO_EDGE, GL_NEAREST);
	}

	Chip8 chip8;

	LARGE_INTEGER startingTime, endingTime, elapsedMicroseconds;
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&startingTime);
	int fps = 60;
	bool running = true;
	MSG msg;
	while (running) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (glSupported && app.needResize) {
				app.needResize = false;
				OutputDebugString(_T("main loop resize\n"));
				resize_gl(app.width, app.height);
			}

			switch (msg.message)
			{
			case WM_CLOSE:
				// confirm close message or pass control to DefWindowProc
				DefWindowProc(hWnd, msg.message, msg.wParam, msg.lParam);
				break;
			case WM_QUIT:
				running = false;
				break;
			case WM_COMMAND: {
				switch (LOWORD(msg.wParam)) {
				case ID_FILE_LOAD_ROM: {
					TCHAR filepath[MAX_PATH] = { 0 };
					if (open_chip8_file(hWnd, filepath)) {
						chip8.load_rom(filepath);
					}
				}
				break;
				case ID_FILE_EXIT:
					PostMessage(hWnd, WM_CLOSE, 0, 0);
					break;
				case ID_SETTING_CONFIG: {
					DialogBox(
						hInstance,
						MAKEINTRESOURCE(IDD_CONFIG),
						NULL,
						config_proc
					);
				}
					break;
				}
			}
			 break;
			case WM_KEYDOWN:
				chip8_keydown_events(msg, chip8);
				break;
			case WM_KEYUP:
				chip8_keyup_events(msg, chip8);
				break;
			default:
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				break;
			}
		}

		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//update_pixels(pixels, pixelNum, comp);
		//draw_grid_pixels(50, 50, Chip8::DISPLAY_W, Chip8::DISPLAY_H, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		//project_to_screen(data->width*1.f, data->height*1.f, screenTextureID, pixels);

		//GLubyte grayscalePixels[Chip8::DISPLAY_H * Chip8::DISPLAY_W];
		//for (int i = 0; i < Chip8::DISPLAY_H * Chip8::DISPLAY_W; ++i) {
		//	grayscalePixels[i] = (rand() % 2) * 255;
		//}
		//chip8_map_to_screen(data->width*1.f, data->height*1.f, screenTextureID, &grayscalePixels[0]);

		if (chip8.is_ROM_opened()) {
			uint16_t code = chip8.fetch_code();
			chip8.execute_code(code);

			QueryPerformanceCounter(&endingTime);
			elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
			elapsedMicroseconds.QuadPart *= 1000000;
			elapsedMicroseconds.QuadPart /= frequency.QuadPart;
			if (elapsedMicroseconds.QuadPart > 1000000 / fps) {
				QueryPerformanceCounter(&startingTime);

				chip8.countdown();
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				chip8_map_to_screen(app.width*1.f, app.height*1.f, screenVBO, screenTextureID, chip8);
				glFinish();
			}
		}

		SwapBuffers(hDC);
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	AppData* data = (AppData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (msg == WM_CREATE) {
		CREATESTRUCT* create = reinterpret_cast<CREATESTRUCT*>(lParam);
		data = reinterpret_cast<AppData*>(create->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)data);
		return 0;
	}
	switch (msg)
	{
	case WM_SIZE:
		output_debug_string_f(_T("wnd_proc WM_SIZE: %d %d\n"), LOWORD(lParam), HIWORD(lParam));
		data->needResize = true;
		data->width = LOWORD(lParam);
		data->height = HIWORD(lParam);
		//SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)data);
		return DefWindowProc(hWnd, msg, wParam, lParam);
	case WM_SYSCOMMAND:
		if (wParam == SC_CLOSE) {
			PostMessage(hWnd, WM_CLOSE, wParam, lParam);
			return 0;
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

INT_PTR CALLBACK config_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR ptr = GetWindowLongPtr(hDlg, GWLP_USERDATA);
	AppData* app = (AppData*)ptr;
	switch (message)
	{
	case WM_INITDIALOG:
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		break;
	case WM_CLOSE:
		DestroyWindow(hDlg);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return (INT_PTR)FALSE;
	}
	return (INT_PTR)TRUE;
}

void output_debug_string_f(const TCHAR* format, ...)
{
	TCHAR buffer[1024] = {}; // A reasonably sized buffer for debug messages
	va_list args;

	va_start(args, format);
	int size = _vsntprintf_s(buffer, _countof(buffer), _TRUNCATE, format, args);
	va_end(args);

	OutputDebugString(buffer);
}

bool open_chip8_file(HWND hwnd, TCHAR* filepath)
{
	TCHAR fileDirectory[MAX_PATH] = { 0 };
	DWORD res = GetCurrentDirectory(MAX_PATH, fileDirectory);
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = filepath;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = _T("Chip8 Files\0*.ch8\0");
	ofn.nFilterIndex = 1; // 1-based
	ofn.lpstrInitialDir = fileDirectory;
	ofn.Flags = OFN_READONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn)) {
		cout << "Selected file: " << filepath << std::endl;
		return true;
	}
	else {
		// User canceled or an error occurred
		DWORD error = CommDlgExtendedError();
		if (error != 0) {
			cerr << "Error in GetOpenFileName: " << error << endl;
		}
		else {
			cout << "File selection canceled." << endl;
		}
		return false;
	}
}
#pragma endregion

#pragma region UI
void add_menu(HWND hwnd)
{
	HMENU menuBar = CreateMenu();

	HMENU fileMenu = CreateMenu();
	AppendMenu(fileMenu, MF_STRING, ID_FILE_LOAD_ROM, _T("Load ROM"));
	AppendMenu(fileMenu, MF_STRING, ID_FILE_EXIT, _T("Exit"));

	HMENU settingMenu = CreateMenu();
	AppendMenu(settingMenu, MF_STRING, ID_SETTING_CONFIG, _T("Config"));

	AppendMenu(menuBar, MF_POPUP, (UINT_PTR)fileMenu, _T("&File"));
	AppendMenu(menuBar, MF_POPUP, (UINT_PTR)settingMenu, _T("&Setting"));

	SetMenu(hwnd, menuBar);
}
#pragma endregion

#pragma region OpenGL 1.5
void setup_gl()
{
	const char* version = (const char*)glGetString(GL_VERSION);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glClearColor(0.f, 0.0f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void set_pixel_size(GLfloat w, GLfloat h)
{
	glPixelZoom(w, h);
}

void resize_gl(GLdouble w, GLdouble h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	set_pixel_size(GLfloat(w / Chip8::DISPLAY_W), GLfloat(h / Chip8::DISPLAY_H));
}

void draw_grid_pixels(int x, int y, int w, int h, GLenum format, GLenum type, const GLvoid* pixels)
{
	glRasterPos2i(x, y);
	glDrawPixels(w, h, format, type, pixels);
}

void set_texture_paramaters(GLuint textureID, GLint wrap, GLint filter)
{
	glBindTexture(GL_TEXTURE_2D, textureID);
	// GL_CLAMP_TO_EDGE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
	// GL_NEAREST
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
}

void draw_rgba2d(GLuint textureID, GLint srcWidth, GLint srcHeight, GLubyte* pixels)
{
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0,
		GL_RGBA,
		srcWidth, srcHeight, 0,
		GL_RGBA, GL_UNSIGNED_BYTE,
		pixels);

	static GLint vertices[] = {
		 0, 0,
		 1, 0,
		 1, 1,
		 0, 1
	};
	static GLfloat texCoords[] = {
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f
	};
	glVertexPointer(2, GL_INT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void draw_grayscale_2d(GLuint textureID, GLint srcWidth, GLint srcHeight, GLubyte* pixels)
{
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0,
		GL_LUMINANCE,
		srcWidth, srcHeight, 0,
		GL_LUMINANCE,
		GL_UNSIGNED_BYTE,
		pixels);
	static GLint vertices[] = {
		 0, 0,
		 1, 0,
		 1, 1,
		 0, 1
	};
	static GLfloat texCoords[] = {
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f
	};
	glVertexPointer(2, GL_INT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, texCoords);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void draw_grayscale_2d(GLuint vbo, GLuint textureID, GLsizei stride, GLint srcWidth, GLint srcHeight, GLubyte* pixels)
{
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0,
		GL_LUMINANCE,
		srcWidth, srcHeight, 0,
		GL_LUMINANCE,
		GL_UNSIGNED_BYTE,
		pixels);
	glBindBuffer(GL_VERTEX_ARRAY, vbo);
	glVertexPointer(2, GL_FLOAT, stride, (GLvoid*)0);
	glTexCoordPointer(2, GL_FLOAT, stride, (GLvoid*)(sizeof(GLfloat) * 2));

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_VERTEX_ARRAY, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
#pragma endregion

#pragma region App
void update_pixels(GLubyte* pixels, int& pixelNum, int comp)
{
	int r = (pixelNum / (Chip8::DISPLAY_W * comp)) % Chip8::DISPLAY_H;
	int c = pixelNum % (Chip8::DISPLAY_W * comp);
	int red = 0, green = 0, blue = 0, alpha = 255;
	if (pixelNum / comp % comp == 0) {
		red = 255;
		green = 0;
		blue = 0;
	}
	else if (pixelNum / comp % comp == 1) {
		red = 0;
		green = 255;
		blue = 0;
	}
	else if (pixelNum / comp % comp == 2) {
		red = 0;
		green = 0;
		blue = 255;
	}
	else if (pixelNum / comp % comp == 3) {
		red = 255;
		green = 255;
		blue = 255;
		alpha = 128;
	}
	red = rand() % 255;
	green = rand() % 255;
	green = rand() % 255;
	pixels[r * Chip8::DISPLAY_W * comp + c] = red;
	pixels[r * Chip8::DISPLAY_W * comp + c + 1] = green;
	pixels[r * Chip8::DISPLAY_W * comp + c + 2] = blue;
	if (comp == 4) {
		pixels[r * Chip8::DISPLAY_W * comp + c + 3] = alpha;
	}
	pixelNum += comp;
}

void project_to_screen(float xScale, float yScale, GLuint textureID, GLubyte* pixels)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glScalef(xScale, yScale, 1.f);
	draw_rgba2d(textureID, Chip8::DISPLAY_W, Chip8::DISPLAY_H, pixels);
	glPopMatrix();
}

void chip8_map_to_screen(float xScale, float yScale, GLuint textureID, Chip8& chip8)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glScalef(xScale, yScale, 1.f);
	draw_grayscale_2d(textureID, Chip8::DISPLAY_W, Chip8::DISPLAY_H, (GLubyte*)chip8.get_display_buffer());
	glPopMatrix();
}

void chip8_map_to_screen(float xScale, float yScale, GLuint vbo, GLuint textureID, Chip8& chip8)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glScalef(xScale, yScale, 1.f);
	draw_grayscale_2d(vbo, textureID, sizeof(GLfloat) * 4, Chip8::DISPLAY_W, Chip8::DISPLAY_H, (GLubyte*)chip8.get_display_buffer());
	glPopMatrix();
}

void chip8_keydown_events(MSG& msg, Chip8& chip8)
{
	switch (msg.wParam)
	{
	case '1': chip8.on_key_down(0x1); break;
	case '2': chip8.on_key_down(0x2); break;
	case '3': chip8.on_key_down(0x3); break;
	case 'Q': chip8.on_key_down(0x4); break;
	case 'W': chip8.on_key_down(0x5); break;
	case 'E': chip8.on_key_down(0x6); break;
	case 'A': chip8.on_key_down(0x7); break;
	case 'S': chip8.on_key_down(0x8); break;
	case 'D': chip8.on_key_down(0x9); break;
	case 'Z': chip8.on_key_down(0xA); break;
	case 'X': chip8.on_key_down(0x0); break;
	case 'C': chip8.on_key_down(0xB); break;
	case '4': chip8.on_key_down(0xC); break;
	case 'R': chip8.on_key_down(0xD); break;
	case 'F': chip8.on_key_down(0xE); break;
	case 'V': chip8.on_key_down(0xF); break;
	default: break;
	}
}

void chip8_keyup_events(MSG& msg, Chip8& chip8)
{
	switch (msg.wParam)
	{
	case '1': chip8.on_key_up(0x1); break;
	case '2': chip8.on_key_up(0x2); break;
	case '3': chip8.on_key_up(0x3); break;
	case 'Q': chip8.on_key_up(0x4); break;
	case 'W': chip8.on_key_up(0x5); break;
	case 'E': chip8.on_key_up(0x6); break;
	case 'A': chip8.on_key_up(0x7); break;
	case 'S': chip8.on_key_up(0x8); break;
	case 'D': chip8.on_key_up(0x9); break;
	case 'Z': chip8.on_key_up(0xA); break;
	case 'X': chip8.on_key_up(0x0); break;
	case 'C': chip8.on_key_up(0xB); break;
	case '4': chip8.on_key_up(0xC); break;
	case 'R': chip8.on_key_up(0xD); break;
	case 'F': chip8.on_key_up(0xE); break;
	case 'V': chip8.on_key_up(0xF); break;
	default: break;
	}
}
#pragma endregion

/*#if _MSC_VER >= 1910 && _MSC_VER <= 1916
// E1097 unknown attribute "no_init_all"
// https://developercommunity.visualstudio.com/t/vs-2017-1592-reports-unknown-attribute-no-init-all/387702
#define no_init_all deprecated
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include <Windows.h>
#include <tchar.h>
#include <commdlg.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cstdlib>
#include <string>

#include "resource.h"
#include "win_glcontext.h"
#include "glad/glad.h"
#include "wgl_loader.h"
#include "chip8.h"

using std::vector;
using std::fill;
using std::begin;
using std::wstring;
using std::cout;
using std::endl;
using std::cerr;

#pragma region Platform
typedef struct PlatformData {
	HWND hWnd;
	HDC hDC;
	HGLRC glContext;
} PlatformData;
static void output_debug_string_f(const TCHAR* format, ...);
static INT_PTR CALLBACK window_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void clearGLContext(PlatformData& data);
#pragma endregion

#pragma region App
typedef struct AppData {
	PlatformData platform;
	int viewW, viewH;
	int windowW, windowH;
	bool needResize;
} AppData;
static void main_loop(AppData& app);
#pragma endregion

#pragma region OpenGL 1.5
static void setup_gl();
#pragma endregion

#pragma region Platform
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PTSTR pCmdLine, int nCmdShow)
{
	SetProcessDPIAware();
	AppData app;
	HWND hWnd = CreateDialogParam(
		hInstance,
		MAKEINTRESOURCE(IDD_WINDOW),
		NULL,
		window_proc,
		(LPARAM)&app
	);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	HDC hDC = GetDC(hWnd);
	HGLRC glContext = create_gl_context(hWnd, hDC);
	wglMakeCurrent(hDC, glContext);
	bool glSupported = gladLoadGL();
	if (glSupported) {
		bool wglLoaded = setup_wgl_functions();
		if (!wglSwapIntervalEXT(0)) {
			DWORD error = GetLastError();
			// ERROR_INVALID_DATA or ERROR_DC_NOT_FOUND
			output_debug_string_f(_T("wglSwapIntervalEXT failed with arg(%d): %d\n"), 1, error);
		}
		setup_gl();
	}

	app.platform.hWnd = hWnd;
	app.platform.hDC = hDC;
	app.platform.glContext = glContext;

	bool running = true;
	MSG msg;
	while (running) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			switch (msg.message)
			{
			case WM_QUIT:
				running = false;
				break;
			default:
				// IsDialogMessage: able to deal with tab/enter/esc button presses
				if (!IsDialogMessage(hWnd, &msg)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				break;
			}
		}
		if (running) {
			main_loop(app);
		}
	}

	return (int)msg.wParam;
}

void output_debug_string_f(const TCHAR* format, ...)
{
	TCHAR buffer[1024] = {}; // A reasonably sized buffer for debug messages
	va_list args;

	va_start(args, format);
	int size = _vsntprintf_s(buffer, _countof(buffer), _TRUNCATE, format, args);
	va_end(args);

	OutputDebugString(buffer);
}

INT_PTR CALLBACK window_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR ptr = GetWindowLongPtr(hDlg, GWLP_USERDATA);
	AppData* app = (AppData*)ptr;
	switch (message)
	{
	case WM_INITDIALOG:
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		break;
	case WM_CLOSE:
		DestroyWindow(hDlg);
		break;
	case WM_DESTROY:
		clearGLContext(app->platform);
		PostQuitMessage(0);
		break;
	case WM_SIZE: {
		HWND g_hWndRenderArea = GetDlgItem(hDlg, IDC_RENDER_AREA);
		LONG g_RenderWidth;
		LONG g_RenderHeight;
		if (g_hWndRenderArea)
		{
			RECT rect;
			GetClientRect(g_hWndRenderArea, &rect);

			g_RenderWidth = rect.right - rect.left;
			g_RenderHeight = rect.bottom - rect.top;

			// InitOpenGL(g_hWndRenderArea);

			SetClassLongPtr(g_hWndRenderArea, GCLP_HBRBACKGROUND, (LONG_PTR)GetStockObject(BLACK_BRUSH));
			InvalidateRect(g_hWndRenderArea, NULL, TRUE);
		}
		output_debug_string_f(_T("wnd_proc WM_SIZE: %d %d, %d %d\n"), LOWORD(lParam), HIWORD(lParam), g_RenderWidth, g_RenderHeight);
		app->needResize = true;
		app->windowW = LOWORD(lParam);
		app->windowH = HIWORD(lParam);
	}
		return (INT_PTR)FALSE;
	default:
		return (INT_PTR)FALSE;
	}
	return (INT_PTR)TRUE;
}

void clearGLContext(PlatformData& data)
{
	wglMakeCurrent(NULL, NULL);
	ReleaseDC(data.hWnd, data.hDC);
	wglDeleteContext(data.glContext);
}
#pragma endregion

#pragma region App
void main_loop(AppData& app)
{

}
#pragma endregion

#pragma region OpenGL 1.5
void setup_gl()
{
	const char* version = (const char*)glGetString(GL_VERSION);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
#pragma endregion*/
