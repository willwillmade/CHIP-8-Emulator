#ifndef WIN_MAIN_H

#include "chip8.h"
#include <tchar.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include <Windows.h>
#include "glad/glad.h"

#pragma region Platform
const TCHAR className[] = _T("CHIP-8 Emulator");

static void output_debug_string_f(const TCHAR* format, ...);
static bool open_chip8_file(HWND hwnd, TCHAR* filepath);

static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK config_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

#pragma region UI
#define ID_FILE_LOAD_ROM					512
#define ID_FILE_EXIT						WM_DESTROY
#define ID_SETTING_CONFIG					1024

void add_menu(HWND hwnd);
#pragma endregion // UI
#pragma endregion // Platform

#pragma region OpenGL 1.5
static void setup_gl();
static void set_pixel_size(GLfloat w, GLfloat h);
static void resize_gl(GLdouble w, GLdouble h);
static void draw_grid_pixels(int x, int y, int w, int h, GLenum format, GLenum type, const GLvoid* pixels);
static void set_texture_paramaters(GLuint textureID, GLint wrap, GLint filter);
static void draw_rgba2d(GLuint textureID, GLint srcWidth, GLint srcHeight, GLubyte* pixels);
static void draw_grayscale_2d(GLuint textureID, GLint srcWidth, GLint srcHeight, GLubyte* pixels);
static void draw_grayscale_2d(GLuint vbo, GLuint textureID, GLsizei stride, GLint srcWidth, GLint srcHeight, GLubyte* pixels);
#pragma endregion

#pragma region App
struct ConfigData {
	int fps;
	bool fadingPixel;
	Chip8Quirks quirks;
};

struct AppData {
	bool needResize;
	int width, height;
	ConfigData config;
};

static void update_pixels(GLubyte* pixels, int& pixelNum, int comp);
static void project_to_screen(float xScale, float yScale, GLuint textureID, GLubyte* pixels);

static void chip8_map_to_screen(float xScale, float yScale, GLuint textureID, Chip8& chip8);
static void chip8_map_to_screen(float xScale, float yScale, GLuint vbo, GLuint textureID, Chip8& chip8);
// https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
static void chip8_keydown_events(MSG& msg, Chip8& chip8);
static void chip8_keyup_events(MSG& msg, Chip8& chip8);
#pragma endregion

#endif // WIN_MAIN_H