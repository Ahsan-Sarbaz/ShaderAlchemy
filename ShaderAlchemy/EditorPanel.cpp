#include "EditorPanel.h"
#include "application.h"

void EditorPanel::OnImGui()
{
	if (ImGui::Begin(name.c_str(), 0, undoIndexOnDisk != editor->GetUndoIndex() ? ImGuiWindowFlags_UnsavedDocument : 0)) {
		if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S))
		{
			auto shader = renderPass->GetShader();

			if (type == EditorPanelType::VertexShader)
			{
				auto source = editor->GetText();
				auto vs = new Shader(ShaderType::Vertex, source);
				shader->AttachShader(vs);
				shader->SetVertexSource(source);
			}
			else if (type == EditorPanelType::FragmentShader)
			{
				auto source = editor->GetText();
				auto fs = new Shader(ShaderType::Fragment, source);
				shader->AttachShader(fs);
				shader->SetFragmentSource(source);
			}

			char* infoLog;

			if (!shader->Link(&infoLog, nullptr))
			{
				std::vector<std::string> errors;
				char* token = strtok(infoLog, "\n");
				while (token != NULL)
				{
					if (std::regex_search(std::string(token), std::regex(R"(((ERROR: \d:\d*:) | (\s*:\s*error)))")))
						errors.emplace_back(std::string(token));
					token = strtok(NULL, "\n");
				}

				delete[] infoLog;

				std::map<int, std::string> errorMarkers;
				for (auto& error : errors)
				{
					std::string expression = R"((?::|\()\d*(?::|\)))";
					auto regexp = std::regex(expression);
					std::smatch match;
					std::regex_search(error, match, regexp);
					regexp = std::regex(R"(\d+)");
					auto line = match[0].str();
					std::smatch match2;
					std::regex_search(line, match2, regexp);
					line = match2[0].str();
					int num = std::stoi(line);
					auto newError = std::regex_replace(error, std::regex(expression), (":" + std::to_string(num) + ":"));

					errorMarkers.insert(std::make_pair<int, std::string>(int(num), std::string(newError)));
					Application::instance->console->AddLog("%s\n", newError.c_str());
				}

				editor->SetErrorMarkers(errorMarkers);
			}
			else
			{
				editor->SetErrorMarkers({});
			}

			undoIndexOnDisk = editor->GetUndoIndex();
		}
		editor->Render((name + "Editor").c_str());
	}
	ImGui::End();
}


