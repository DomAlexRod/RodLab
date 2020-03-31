#pragma once
#include <experimental/filesystem>
#include <vector>
#include <SDL.h>
#include <SDL_opengl.h>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_sdl.h"
#include "ImGui/imgui_impl_opengl2.h"
#include "TextEditor.h"

// Portable helpers
int   Stricmp(const char* str1, const char* str2);
int   Strnicmp(const char* str1, const char* str2, int n);
char* Strdup(const char* str);
void  Strtrim(char* str);

class Console
{
private:
	int						             SDLwindowWidth;
	int						             SDLwindowHeight;
	char								 InputBuf[256];
	ImVector<char*>						 Items;
	std::vector<const char*>		     Commands;
	ImVector<char*>                      History;
	int								     HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
	ImGuiTextFilter					     Filter;
	bool								 AutoScroll;
	bool								 ScrollToBottom;
	bool								 isPlotOpen;
	bool								 isTextEditorOpen;
	TextEditor							 textEditor;
	std::experimental::filesystem::path  currentPath;
	
	//private functions
	static int TextEditCallbackStub(ImGuiInputTextCallbackData* data);
	int TextEditCallback(ImGuiInputTextCallbackData* data);

public:
	Console();
	~Console();
	void getSDLWindowSize();
	void clearLog();
	void addLog(const char* fmt, ...) IM_FMTARGS(2);
	void showPlot();
	void processCommand(const char* command_line);
	void ExecCommand(const char* command_line);
	void draw(const char* title);
};
