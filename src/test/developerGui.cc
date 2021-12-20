#include "developerGui.h"

#ifdef EGGINE_DEVELOPER_MODE
#include <stdarg.h>

#include "chunkContainer.h"
#include "../engine/engine.h"

int developerPrint(const char* buffer, ...) {
	if(engine->developerGui == nullptr) {
		return 0;
	}
	
	char output[1024];
	va_list argptr;
	va_start(argptr, buffer);
	vsnprintf(output, 1024, buffer, argptr);
	va_end(argptr);

	engine->developerGui->console.push_back(ConsoleEntry {
		level: 0,
		contents: string(output),
	});
	
	return 0;
}

int developerWarning(const char* buffer, ...) {
	if(engine->developerGui == nullptr) {
		return 0;
	}
	
	char output[1024];
	va_list argptr;
	va_start(argptr, buffer);
	vsnprintf(output, 1024, buffer, argptr);
	va_end(argptr);

	engine->developerGui->console.push_back(ConsoleEntry {
		level: 1,
		contents: string(output),
	});
	
	return 0;
}

int developerError(const char* buffer, ...) {
	if(engine->developerGui == nullptr) {
		return 0;
	}
	
	char output[1024];
	va_list argptr;
	va_start(argptr, buffer);
	vsnprintf(output, 1024, buffer, argptr);
	va_end(argptr);

	engine->developerGui->console.push_back(ConsoleEntry {
		level: 2,
		contents: string(output),
	});
	
	return 0;
}

int vDeveloperPrint(const char* buffer, va_list args) {
	if(engine->developerGui == nullptr) {
		return 0;
	}
	
	char output[1024];
	vsnprintf(output, 1024, buffer, args);
	engine->developerGui->console.push_back(ConsoleEntry {
		level: 0,
		contents: string(output),
	});

	return 0;
}

int vDeveloperWarning(const char* buffer, va_list args) {
	if(engine->developerGui == nullptr) {
		return 0;
	}
	
	char output[1024];
	vsnprintf(output, 1024, buffer, args);
	engine->developerGui->console.push_back(ConsoleEntry {
		level: 1,
		contents: string(output),
	});

	return 0;
}

int vDeveloperError(const char* buffer, va_list args) {
	if(engine->developerGui == nullptr) {
		return 0;
	}
	
	char output[1024];
	vsnprintf(output, 1024, buffer, args);
	engine->developerGui->console.push_back(ConsoleEntry {
		level: 2,
		contents: string(output),
	});

	return 0;
}

DeveloperGui::DeveloperGui() {
	esSetPrintFunction(engine->eggscript, developerPrint, developerWarning, developerError);
	esSetVPrintFunction(engine->eggscript, vDeveloperPrint, vDeveloperWarning, vDeveloperError);
}

int DeveloperGui::consoleCallback(ImGuiInputTextCallbackData* data) {
	switch(data->EventFlag) {
		case ImGuiInputTextFlags_CallbackHistory: {
			int oldHistoryPosition = this->historyPosition;
			
			if(data->EventKey == ImGuiKey_UpArrow) {
				this->historyPosition++;
				if(this->historyPosition >= this->history.size()) {
					this->historyPosition = this->history.size();
				}
			}
			else if(data->EventKey == ImGuiKey_DownArrow) {
				this->historyPosition--;
				if(this->historyPosition < 0) {
					this->historyPosition = 0;
				}
			}
			
			if(oldHistoryPosition == 0) { // save incomplete command
				this->incompleteCommand = string(data->Buf);
			}

			if(this->historyPosition == 0) {
				data->DeleteChars(0, data->BufTextLen);
				data->InsertChars(0, this->incompleteCommand.c_str());
			}
			else {
				size_t index = this->history.size() - this->historyPosition;
				data->DeleteChars(0, data->BufTextLen);
				data->InsertChars(0, this->history[index].c_str());
			}

			break;
		}
	}
	return 0;
}

void DeveloperGui::render() {
	// tile selection
	{
		ImGui::SetNextWindowSize(ImVec2(-1, -1));
		ImGui::Begin("Tile Selection", NULL, ImGuiWindowFlags_NoResize);

		float usedWidth = 0.f, margin = 10.f, size = 50.f, scrollbar = 0.f;
		unsigned int amount = 10;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(margin, margin));
		ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, scrollbar);
		ImGui::BeginChild("scrolling", ImVec2(amount * size + (amount - 1) * margin + margin * 2.0f + scrollbar, 600.f), true, 0);
		ImGui::PopStyleVar(2);

		float width = ImGui::GetContentRegionAvailWidth();
		int count = 0;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(margin, margin));
		for(render::Texture* texture: this->spritesheetImages) {
			if(usedWidth + size < width && count != 0) {
				ImGui::SameLine();
				usedWidth += size + margin;
			}
			else if(usedWidth + size >= width) {
				usedWidth = 0.f;
			}
			ImGui::PushID(count);
			if(ImGui::ImageButton((void*)texture->texture, ImVec2(size, size * 2), ImVec2(0, 0), ImVec2(1, 1), 0)) {
				esEntry arguments[1];
				esCreateNumberAt(arguments, count);
				esDeleteEntry(esCallFunction(engine->eggscript, "onDevTextureSwitch", 1, arguments));
			}
			ImGui::PopID();

			count++;
		}
		ImGui::PopStyleVar(1);
		ImGui::EndChild();

		ImGui::BeginChild("save", ImVec2(0, 20), false);
		if(ImGui::Button("Save map", ImVec2(150, 0))) {
			engine->chunkContainer->map.save("test.map");
		}
		ImGui::EndChild();

		ImGui::End();
	}

	// eggscript shell
	{
		ImGui::SetNextWindowSize(ImVec2(-1, -1));
		ImGui::Begin("eggscript shell", NULL, ImGuiWindowFlags_NoResize);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.f, 10.f));
		ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 10.f);
		ImGui::BeginChild("scrolling", ImVec2(800.f, 400.f), true, ImGuiWindowFlags_HorizontalScrollbar);
		ImGui::PopStyleVar(2);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 2));
		for(unsigned int i = 0; i < this->console.size(); i++) {
			if(this->console[i].level == 1) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.f / 255.f, 238.f / 255.f, 94.f / 255.f, 1.0f));
			}
			else if(this->console[i].level == 2) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.f / 255.f, 79.f / 255.f, 111.f / 255.f, 1.0f));
			}
			
			ImGui::TextUnformatted(this->console[i].contents.c_str());

			if(this->console[i].level != 0) {
				ImGui::PopStyleColor();
			}
		}
		ImGui::PopStyleVar();
		ImGui::EndChild();
		
		char buffer[1024];
		memset(buffer, 0, sizeof(buffer));
		ImGui::PushItemWidth(800.f);
		if(
			ImGui::InputText(
				"##frog",
				buffer,
				IM_ARRAYSIZE(buffer),
				ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory,
				[](ImGuiInputTextCallbackData* data) { return ((DeveloperGui*)data->UserData)->consoleCallback(data); },
				this
			)
		) {
			this->console.push_back(ConsoleEntry {
				level: 0,
				contents: "> " + string(buffer),
			});
			esEval(engine->eggscript, buffer);
			ImGui::SetKeyboardFocusHere(-1);

			this->history.push_back(string(buffer));
			this->historyPosition = 0;
		}
		ImGui::PopItemWidth();

		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void DeveloperGui::prerender() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

#endif
