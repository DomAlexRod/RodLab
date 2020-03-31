// Copyright(C) 2019 Mappin Technologies LTD - All Rights Reserved

#include "Application.h"
#include <string>

namespace fs = std::experimental::filesystem;

Application::Application()
{
}

void Application::showFilesWindow()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoSavedSettings;
	window_flags |= ImGuiWindowFlags_NoTitleBar;

	int height, width;
	SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &width, &height);
	ImGui::SetNextWindowPos(ImVec2(0, 18));
	ImGui::SetNextWindowSize(ImVec2(width/4, height - 20));

	//Begin left hand window
	ImGui::Begin("Files", NULL, window_flags);
	
	fs::path openingPath = fs::current_path() / fs::path("src");
	std::vector<std::string> files;
	static std::string currentFile = "main.cpp";

	for (auto& item : fs::directory_iterator(openingPath))
	{
		files.push_back(item.path().filename().string());
	}

	if (ImGui::BeginCombo("##start List", currentFile.c_str(), ImGuiSelectableFlags_DontClosePopups ))
	{
		for (int n = 0; n <files.size(); n++)
		{
			bool is_selected = (currentFile == files[n]);
			if (ImGui::Selectable(files[n].c_str(), is_selected))
			{
				currentFile = files[n];
			}
			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	
	//end navigation window
	ImGui::End();
}

void Application::run()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

	// Setup window

	SDL_DisplayMode current;
	SDL_GetCurrentDisplayMode(0, &current);
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Window * window = SDL_CreateWindow("RodLab", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	SDL_GL_SetSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO & io = ImGui::GetIO();
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowBorderSize = 1;

	// Setup Platform/Renderer bindings
	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL2_Init();

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// Main loop
	bool done = false;
	while (!done)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT)
				done = true;
		}

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL2_NewFrame();
		ImGui_ImplSDL2_NewFrame(window);
		ImGui::NewFrame();

		//Add all windows here
		mainMenuBar.show();
		showFilesWindow();
		console.draw("RodLab Console");

		// Rendering
		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(window);
	}

	// Cleanup
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}




