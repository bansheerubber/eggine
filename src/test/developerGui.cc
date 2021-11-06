#include "developerGui.h"

#ifdef EGGINE_DEVELOPER_MODE
#include "../engine/engine.h"

void DeveloperGui::render() {
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
			esCallFunction(engine->eggscript, "onDevTextureSwitch", 1, arguments);
		}
		ImGui::PopID();

		count++;
	}
	ImGui::PopStyleVar(1);
	ImGui::EndChild();
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void DeveloperGui::prerender() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

#endif
