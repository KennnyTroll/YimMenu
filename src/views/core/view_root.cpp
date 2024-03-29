#include "views/view.hpp"
#include "lua/lua_manager.hpp"
#include "imgui_internal.h"

namespace big
{
	void view::root()
	{
		view::heading();
		view::navigation();
		view::players();
		view::active_view();

		debug::main();

		if (g_lua_manager)
			g_lua_manager->draw_independent_gui();

		if (g.window.demo) // It is not the YimMenu way.
			ImGui::ShowDemoWindow(&g.window.demo);










		//struct funcs
		//{
		//	static bool IsLegacyNativeDupe(ImGuiKey key)
		//	{
		//		return key < 512 && ImGui::GetIO().KeyMap[key] != -1;
		//	}
		//}; // Hide Native<>ImGuiKey duplicates when both exists in the array
          
		//ImGui::Text("Keys down:");
		//for (ImGuiKey key = ImGuiKey_KeysData_OFFSET; key < ImGuiKey_COUNT; key = (ImGuiKey)(key + 1))
		//{
		//	if (funcs::IsLegacyNativeDupe(key) || !ImGui::IsKeyDown(key))
		//		continue;
		//	ImGui::SameLine();
		//	ImGui::Text(ImGui::IsNamedKey(key) ? "\"%s\"" : "\"%s\" %d", ImGui::GetKeyName(key), key);
		//	ImGui::SameLine();
		//	ImGui::Text("(%.02f)", ImGui::GetKeyData(key)->DownDuration);
		//}
		//ImGui::Text("Keys pressed:");
		//for (ImGuiKey key = ImGuiKey_KeysData_OFFSET; key < ImGuiKey_COUNT; key = (ImGuiKey)(key + 1))
		//{
		//	if (funcs::IsLegacyNativeDupe(key) || !ImGui::IsKeyPressed(key))
		//		continue;
		//	ImGui::SameLine();
		//	ImGui::Text(ImGui::IsNamedKey(key) ? "\"%s\"" : "\"%s\" %d", ImGui::GetKeyName(key), key);
		//}
		//ImGui::Text("Keys released:");
		//for (ImGuiKey key = ImGuiKey_KeysData_OFFSET; key < ImGuiKey_COUNT; key = (ImGuiKey)(key + 1))
		//{
		//	if (funcs::IsLegacyNativeDupe(key) || !ImGui::IsKeyReleased(key))
		//		continue;
		//	g_gui_service->set_selected(navItem.first);
		//	ImGui::SameLine();
		//	ImGui::Text(ImGui::IsNamedKey(key) ? "\"%s\"" : "\"%s\" %d", ImGui::GetKeyName(key), key);
		//}
		
		
         



	}
}
