#pragma once
#include "ImGui/imgui.h"
#include <string>

class TextEditor
{
private:
	char text[1024 * 10];
	char result[1024];
	std::string processResult(std::string cmd);
	void saveAsMenuItem();
	void runPython();
	void showMenuBar();

public:
	void draw(bool* isOpen);
};
