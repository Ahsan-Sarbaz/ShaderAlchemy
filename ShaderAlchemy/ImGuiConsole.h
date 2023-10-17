#pragma once
#include <imgui.h>

struct ImGuiConsole
{
	ImGuiTextBuffer     Buf;
	ImGuiTextFilter     Filter;
	ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
	bool                AutoScroll;  // Keep scrolling if already at the bottom.

	ImGuiConsole();

	void Clear();

	void AddLog(const char* fmt, ...) IM_FMTARGS(2);

	void Draw(const char* title, bool* p_open = NULL);
};
