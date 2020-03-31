#include "TextEditor.h"
#include <fstream>
#include <experimental/filesystem>
#include <cstdio>
#include <iostream>

#define B_KEY 5
#define CTRL_KEY 224

namespace fs = std::experimental::filesystem;

std::string TextEditor::processResult(std::string cmd)
{
	std::string data;
	FILE* stream;
	const int max_buffer = 256;
	char buffer[max_buffer];
	cmd.append(" 2>&1");

	stream = _popen(cmd.c_str(), "r");
	if (stream) {
		while (!feof(stream))
			if (fgets(buffer, max_buffer, stream) != NULL) data.append(buffer);
		_pclose(stream);
	}
	return data;
}

void TextEditor::saveAsMenuItem()
{
	ImGui::OpenPopup("save");
	if (ImGui::BeginPopupModal("save", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static char fileBuffer[64] = ""; 
		ImGui::InputText("#save", fileBuffer, 64);

		if (ImGui::Button("OK", ImVec2(120, 0))) 
		{ 
			fs::path currentPath = fs::current_path();
			std::ofstream saveFile;
			std::string fileName = currentPath.string() + "\\" + fileBuffer; 
			saveFile.open(fileName);
			saveFile << text;
			saveFile.close();
			ImGui::CloseCurrentPopup(); 
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
	std::cout << "saving";
}

void TextEditor::runPython()
{
	fs::path currentPath = fs::current_path();
	std::string fileName = currentPath.string() + "\\temp.py";
	std::ofstream tempFile;
	tempFile.open(fileName);
	tempFile << text;
	tempFile.close();
	std::string commandString = "cd " + currentPath.string() + "& python temp.py";
	std::string resultS = processResult(commandString).c_str();
	strcpy(result, resultS.c_str());
	remove(fileName.c_str());
}

void TextEditor::showMenuBar()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save As"))
			{
				saveAsMenuItem();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Dock On Right"))
			{

			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Run"))
		{
			if (ImGui::MenuItem("Run With Python", "Ctrl+b"))
			{
				runPython();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

void TextEditor::draw(bool* isOpen)
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_MenuBar;
	ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(100, 0, 0, 100));
	ImGui::SetNextWindowBgAlpha(240.f);

	if (!ImGui::Begin("Text Editor", isOpen, window_flags))
	{
		ImGui::End();
		return;
	}
	
	ImGui::PopStyleColor();
	showMenuBar();

	if (ImGui::IsKeyPressed(CTRL_KEY) && ImGui::IsKeyPressed(B_KEY))
	{
		runPython();
	}

	ImGuiInputTextFlags textFlags = ImGuiInputTextFlags_AllowTabInput;
	ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text), ImVec2(ImGui::GetWindowWidth() - 18, 2*ImGui::GetWindowHeight() / 3), textFlags);
	ImGui::Separator();
	
	ImGuiInputTextFlags resultFlags = textFlags |= ImGuiInputTextFlags_ReadOnly;
	ImGui::InputTextMultiline("##result", result, IM_ARRAYSIZE(result), ImVec2(ImGui::GetWindowWidth() - 18,  ImGui::GetWindowHeight() / 4.2), resultFlags);

	
	ImGui::End();
}
