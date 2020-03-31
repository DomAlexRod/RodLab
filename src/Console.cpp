#include "Console.h"
#include <iostream>
#include <string>

namespace fs = std::experimental::filesystem;

int   Stricmp(const char* str1, const char* str2) { int d; while ((d = tolower(*str2) - tolower(*str1)) == 0 && *str1) { str1++; str2++; } return d; }
int   Strnicmp(const char* str1, const char* str2, int n) { int d = 0; while (n > 0 && (d = tolower(*str2) - tolower(*str1)) == 0 && *str1) { str1++; str2++; n--; } return d; }
char* Strdup(const char* str) { size_t len = strlen(str) + 1; void* buf = malloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)str, len); }
void  Strtrim(char* str) { char* str_end = str + strlen(str); while (str_end > str && str_end[-1] == ' ') str_end--; *str_end = 0; }

Console::Console()
{
	Commands.push_back("help");
	Commands.push_back("history");
	Commands.push_back("clear");
	Commands.push_back("classify");
	Commands.push_back("plot");
	Commands.push_back("destroy");
	Commands.push_back("pwd");
	Commands.push_back("cd");
	clearLog();
	memset(InputBuf, 0, sizeof(InputBuf));
	HistoryPos = -1;
	AutoScroll = true;
	ScrollToBottom = true;
	isPlotOpen = false;
	isTextEditorOpen = false;
	addLog("Enter Commands...");
	currentPath = fs::current_path();
	TextEditor textEditor;

}
Console::~Console()
{
	clearLog();
	for (int i = 0; i < History.Size; i++)
		free(History[i]);
}

void Console::getSDLWindowSize()
{
	SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &SDLwindowWidth, &SDLwindowHeight);
}

void Console::clearLog()
{
	for (int i = 0; i < Items.Size; i++)
		free(Items[i]);
	Items.clear();
	ScrollToBottom = true;
}

void Console::addLog(const char* fmt, ...) IM_FMTARGS(2)
{
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
	buf[IM_ARRAYSIZE(buf) - 1] = 0;
	va_end(args);
	Items.push_back(Strdup(buf));
	if (AutoScroll)
		ScrollToBottom = true;
}

void Console::showPlot()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoSavedSettings;

	ImGui::SetNextWindowPos(ImVec2(5*SDLwindowWidth/8, 18));
	ImGui::SetNextWindowSize(ImVec2(3*SDLwindowWidth/8, SDLwindowHeight - 18));

	if (!ImGui::Begin("Plot Widget", &isPlotOpen, window_flags))
	{
		ImGui::End();
		return;
	}

	static float x[90] = { 0 };

	for (int i = 0; i < 90; i++)
	{
		x[i] = cosf(i * 0.1);
	}

	//Title, data, data length, data offset, over lay text, min, max , size
	ImGui::PlotLines("Result", x, IM_ARRAYSIZE(x), 0, "y = cos(x)", -1.0f, 1.0f, ImVec2(0, 150));

	for (int i = 0; i < 90; i++)
	{
		x[i] = sinf(i * 0.1);
	}
	ImGui::PlotLines("Result", x, IM_ARRAYSIZE(x), 0, "y = sin(x)", -1.0f, 1.0f, ImVec2(0, 150));

	static bool animate = true;
	ImGui::Checkbox("Animate", &animate);

	static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
	ImGui::PlotLines("Frame Times", arr, IM_ARRAYSIZE(arr));

	// Create a dummy array of contiguous float values to plot
	// Tip: If your float aren't contiguous but part of a structure, you can pass a pointer to your first float and the sizeof() of your structure in the Stride parameter.
	static float values[90] = { 0 };
	static int values_offset = 0;
	static double refresh_time = 0.0;
	if (!animate || refresh_time == 0.0)
		refresh_time = ImGui::GetTime();
	while (refresh_time < ImGui::GetTime()) // Create dummy data at fixed 60 hz rate for the demo
	{
		static float phase = 0.0f;
		values[values_offset] = cosf(phase);
		values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
		phase += 0.10f * values_offset;
		refresh_time += 1.0f / 60.0f;
	}
	ImGui::PlotLines("Lines", values, IM_ARRAYSIZE(values), values_offset, "avg 0.0", -1.0f, 1.0f, ImVec2(0, 80));
	ImGui::PlotHistogram("Histogram", arr, IM_ARRAYSIZE(arr), 0, NULL, 0.0f, 1.0f, ImVec2(0, 80));

	// Use functions to generate output
	// FIXME: This is rather awkward because current plot API only pass in indices. We probably want an API passing floats and user provide sample rate/count.
	struct Funcs
	{
		static float Sin(void*, int i) { return sinf(i * 0.1f); }
		static float Saw(void*, int i) { return (i & 1) ? 1.0f : -1.0f; }
	};
	static int func_type = 0, display_count = 70;
	ImGui::Separator();
	ImGui::Combo("func", &func_type, "Sin\0Saw\0");
	ImGui::SameLine();
	ImGui::SliderInt("Sample count", &display_count, 1, 400);
	float (*func)(void*, int) = (func_type == 0) ? Funcs::Sin : Funcs::Saw;
	ImGui::PlotLines("Lines", func, NULL, display_count, 0, NULL, -1.0f, 1.0f, ImVec2(0, 80));
	ImGui::PlotHistogram("Histogram", func, NULL, display_count, 0, NULL, -1.0f, 1.0f, ImVec2(0, 80));
	ImGui::Separator();

	// Animate a simple progress bar
	static float progress = 0.0f, progress_dir = 1.0f;
	if (animate)
	{
		progress += progress_dir * 0.4f * ImGui::GetIO().DeltaTime;
		if (progress >= +1.1f) { progress = +1.1f; progress_dir *= -1.0f; }
		if (progress <= -0.1f) { progress = -0.1f; progress_dir *= -1.0f; }
	}

	// Typically we would use ImVec2(-1.0f,0.0f) to use all available width, or ImVec2(width,0.0f) for a specified width. ImVec2(0.0f,0.0f) uses ItemWidth.
	ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::Text("Progress Bar");

	float progress_saturated = (progress < 0.0f) ? 0.0f : (progress > 1.0f) ? 1.0f : progress;
	char buf[32];
	sprintf(buf, "%d/%d", (int)(progress_saturated * 1753), 1753);
	ImGui::ProgressBar(progress, ImVec2(0.f, 0.f), buf);


	ImGui::End();
}

//called by ExecCommand To determine response
void Console::processCommand(const char* command_line)
{
	if (Stricmp(command_line, "clear") == 0)
	{
		clearLog();
	}
	else if (Stricmp(command_line, "help") == 0)
	{
		addLog("Commands:");
		for (int i = 0; i < Commands.size(); i++)
			addLog("- %s", Commands[i]);
	}
	else if (Stricmp(command_line, "history") == 0)
	{
		int first = History.Size - 10;
		for (int i = first > 0 ? first : 0; i < History.Size; i++)
			addLog("%3d: %s\n", i, History[i]);
	}
	else if (Stricmp(command_line, "plot") == 0)
	{
		isPlotOpen = true;
	}
	else if (Stricmp(command_line, "editor") == 0) 
	{
		isTextEditorOpen = true;
	}
	else if (Stricmp(command_line, "destroy") == 0)
	{
		addLog("[error] Cannot destory Console.");
	}
	else if (Stricmp(command_line, "pwd") == 0)
	{
		addLog(currentPath.string().c_str());
	}
	else if (Stricmp(command_line, "cd ..") == 0 )
	{
		currentPath = currentPath.parent_path();
		addLog(currentPath.string().c_str());
	}
	else if (Strnicmp(command_line, "cd ", 3) == 0)
	{
		//get child path from command
		std::string childPath = std::string(command_line).substr(3);
		fs::path combinedPath = fs::path(currentPath.string() + "\\" + childPath);

		if (fs::exists(combinedPath))
		{
			currentPath = combinedPath;
			addLog(currentPath.string().c_str());
		}
		else
		{
			addLog("[error] Cannot find given path");
		}
	}
	else if (Stricmp(command_line, "ls") == 0)
	{
		for (auto& item : fs::directory_iterator(currentPath))
		{
			addLog(item.path().string().c_str());
		}
	}
	else if (Strnicmp(command_line, "system(", 7) == 0)
	{
		//grab command between brackets
		std::string functionInput = std::string(command_line).substr(7);
		//To remove closing bracket
		functionInput.pop_back();

		addLog(functionInput.c_str());
		system(functionInput.c_str());
	}
	else
	{
		addLog("Unknown command: '%s'\n", command_line);
	}
}

void Console::ExecCommand(const char* command_line)
{
	addLog(">>> %s\n", command_line);

	// Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
	HistoryPos = -1;
	for (int i = History.Size - 1; i >= 0; i--)
		if (Stricmp(History[i], command_line) == 0)
		{
			free(History[i]);
			History.erase(History.begin() + i);
			break;
		}
	History.push_back(Strdup(command_line));

	//delegate processing of command
	processCommand(command_line);

	// On command input, we scroll to bottom even if AutoScroll==false
	ScrollToBottom = true;
}

void Console::draw(const char* title)
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoSavedSettings;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	
	//get the context window size for scaling
	getSDLWindowSize();

	ImGui::SetNextWindowPos(ImVec2(SDLwindowWidth/4, 18) );
	ImGui::SetNextWindowSize(ImVec2(3*SDLwindowWidth/8, SDLwindowHeight-18));

	if (!ImGui::Begin(title, NULL, window_flags))
	{
		ImGui::End();
		return;
	}

	ImGui::TextWrapped("A more elaborate implementation may want to store entries along with extra data such as timestamp, emitter, etc.");
	ImGui::TextWrapped("Enter 'HELP' for help, press TAB to use text completion.");

	if (ImGui::SmallButton("Add Dummy Text")) { addLog("%d some text", Items.Size); addLog("some more text"); addLog("display very important message here!"); } ImGui::SameLine();
	if (ImGui::SmallButton("Add Dummy Error")) { addLog("[error] something went wrong"); } ImGui::SameLine();
	if (ImGui::SmallButton("Clear")) { clearLog(); } ImGui::SameLine();
	bool copy_to_clipboard = ImGui::SmallButton("Copy"); ImGui::SameLine();
	if (ImGui::SmallButton("Scroll to bottom")) ScrollToBottom = true;

	ImGui::Separator();

	// Options menu
	if (ImGui::BeginPopup("Options"))
	{
		if (ImGui::Checkbox("Auto-scroll", &AutoScroll))
			if (AutoScroll)
				ScrollToBottom = true;
		ImGui::EndPopup();
	}

	// Options, Filter
	if (ImGui::Button("Options"))
		ImGui::OpenPopup("Options");
	ImGui::SameLine();
	Filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
	ImGui::Separator();

	const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing(); // 1 separator, 1 input text
	ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar); // Leave room for 1 separator + 1 InputText
	if (ImGui::BeginPopupContextWindow())
	{
		if (ImGui::Selectable("Clear")) clearLog();
		ImGui::EndPopup();
	}

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
	if (copy_to_clipboard)
		ImGui::LogToClipboard();
	for (int i = 0; i < Items.Size; i++)
	{
		const char* item = Items[i];
		if (!Filter.PassFilter(item))
			continue;

		// Normally you would store more information in your item (e.g. make Items[] an array of structure, store color/type etc.)
		bool pop_color = false;
		if (strstr(item, "[error]")) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f)); pop_color = true; }
		else if (strncmp(item, "# ", 2) == 0) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.6f, 1.0f)); pop_color = true; }
		ImGui::TextUnformatted(item);
		if (pop_color)
			ImGui::PopStyleColor();
	}
	if (copy_to_clipboard)
		ImGui::LogFinish();
	if (ScrollToBottom)
		ImGui::SetScrollHereY(1.0f);
	ScrollToBottom = false;
	ImGui::PopStyleVar();
	ImGui::EndChild();
	ImGui::Separator();

	// Command-line
	bool reclaim_focus = false;
	if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory, &TextEditCallbackStub, (void*)this))
	{
		char* s = InputBuf;
		Strtrim(s);
		if (s[0])
		{
			ExecCommand(s);
		}
		strcpy(s, "");
		reclaim_focus = true;
	}

	// Auto-focus on window apparition
	ImGui::SetItemDefaultFocus();
	if (reclaim_focus)
		ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

	ImGui::End();

	//Display console-managed windows
	if (isPlotOpen)       { showPlot(); }
	if (isTextEditorOpen) { textEditor.draw(&isTextEditorOpen); }
}

int Console::TextEditCallbackStub(ImGuiInputTextCallbackData * data) 
{
	Console* console = (Console*)data->UserData;
	return console->TextEditCallback(data);
}

int Console::TextEditCallback(ImGuiInputTextCallbackData * data)
{
	//AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
	switch (data->EventFlag)
	{
	case ImGuiInputTextFlags_CallbackCompletion:
	{
		// Example of TEXT COMPLETION

		// Locate beginning of current word
		const char* word_end = data->Buf + data->CursorPos;
		const char* word_start = word_end;
		while (word_start > data->Buf)
		{
			const char c = word_start[-1];
			if (c == ' ' || c == '\t' || c == ',' || c == ';')
				break;
			word_start--;
		}

		// Build a list of candidates
		ImVector<const char*> candidates;
		for (int i = 0; i < Commands.size(); i++)
			if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
				candidates.push_back(Commands[i]);

		if (candidates.Size == 0)
		{
			// No match
			addLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
		}
		else if (candidates.Size == 1)
		{
			// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
			data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
			data->InsertChars(data->CursorPos, candidates[0]);
			data->InsertChars(data->CursorPos, " ");
		}
		else
		{
			// Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
			int match_len = (int)(word_end - word_start);
			for (;;)
			{
				int c = 0;
				bool all_candidates_matches = true;
				for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
					if (i == 0)
						c = tolower(candidates[i][match_len]);
					else if (c == 0 || c != tolower(candidates[i][match_len]))
						all_candidates_matches = false;
				if (!all_candidates_matches)
					break;
				match_len++;
			}

			if (match_len > 0)
			{
				data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
				data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
			}

			// List matches
			addLog("Possible matches:\n");
			for (int i = 0; i < candidates.Size; i++)
				addLog("- %s\n", candidates[i]);
		}

		break;
	}
	case ImGuiInputTextFlags_CallbackHistory:
	{
		// Example of HISTORY
		const int prev_history_pos = HistoryPos;
		if (data->EventKey == ImGuiKey_UpArrow)
		{
			if (HistoryPos == -1)
				HistoryPos = History.Size - 1;
			else if (HistoryPos > 0)
				HistoryPos--;
		}
		else if (data->EventKey == ImGuiKey_DownArrow)
		{
			if (HistoryPos != -1)
				if (++HistoryPos >= History.Size)
					HistoryPos = -1;
		}

		// A better implementation would preserve the data on the current input line along with cursor position.
		if (prev_history_pos != HistoryPos)
		{
			const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
			data->DeleteChars(0, data->BufTextLen);
			data->InsertChars(0, history_str);
		}
	}
	}
	return 0;
}
