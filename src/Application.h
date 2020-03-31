#pragma once
#include "Console.h"
#include "MainMenuBar.h"

class Application
{
private:
	//vars
	MainMenuBar mainMenuBar;
	Console console;

	//funcs
	void showFilesWindow();

public:
	Application();
	void run();
};
