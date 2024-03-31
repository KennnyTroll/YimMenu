#include "fiber_pool.hpp"
#include "views/view.hpp"
#include "script_mgr.hpp"

#include "gui.hpp"
#include "imgui_internal.h"

namespace big
{
	struct funcs
	{
		static bool IsLegacyNativeDupe(ImGuiKey key)
		{
			return key < 512 && ImGui::GetIO().KeyMap[key] != -1;
		}
	};

	void view::heading()
	{
		if (!g.settings.view_imputs)
		{
			ImGui::SetNextWindowSize({300.f * g.window.gui_scale, 80.f * g.window.gui_scale});
			ImGui::SetNextWindowPos({10.f, 10.f});
		}
		else
		{
			//ImGui::SetNextWindowSize({300.f * g.window.gui_scale, 80.f * g.window.gui_scale});
			ImGui::SetNextWindowPos({1200.f, 10.f});
		}
		
		if (ImGui::Begin("menu_heading", nullptr, window_flags | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::BeginGroup();
			ImGui::Text("HEADING_WELCOME"_T.data());

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.172f, 0.380f, 0.909f, 1.f));
			ImGui::Text(g_local_player == nullptr || g_local_player->m_player_info == nullptr ?
			        "UNKNOWN_USERNAME"_T.data() :
			        g_local_player->m_player_info->m_net_player_data.m_name);
			ImGui::PopStyleColor();
			ImGui::EndGroup();
			ImGui::SameLine();
			ImGui::SetCursorPos(
			    {(300.f * g.window.gui_scale) - ImGui::CalcTextSize("UNLOAD"_T.data()).x - ImGui::GetStyle().ItemSpacing.x,
			        ImGui::GetStyle().WindowPadding.y / 2 + ImGui::GetStyle().ItemSpacing.y + (ImGui::CalcTextSize("W").y / 2)});
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.69f, 0.29f, 0.29f, 1.00f));
			if (components::nav_button("UNLOAD"_T))
			{
				// allow to unload in the main title screen.
				if (g_script_mgr.can_tick())
				{
					// empty the pool, we want the that job below run no matter what for clean up purposes.
					g_fiber_pool->reset();
					g_fiber_pool->queue_job([] {
						g_lua_manager->trigger_event<menu_event::MenuUnloaded>();
						for (auto& command : g_looped_commands)
							if (command->is_enabled())
								command->on_disable();

						g_running = false;
					});
				}
				else
				{
					g_lua_manager->trigger_event<menu_event::MenuUnloaded>();
					g_running = false;
				}
			}
			ImGui::PopStyleColor();

			if (g.settings.view_imputs)
			{				
				ImGui::Text("Keys down:");
				for (ImGuiKey key = ImGuiKey_KeysData_OFFSET; key < ImGuiKey_COUNT; key = (ImGuiKey)(key + 1))
				{
					if (funcs::IsLegacyNativeDupe(key) || !ImGui::IsKeyDown(key))
						continue;
					ImGui::SameLine();
					ImGui::Text(ImGui::IsNamedKey(key) ? "\"%s\"" : "\"%s\" %d", ImGui::GetKeyName(key), key);
					ImGui::SameLine();
					ImGui::Text("(%.02f)", ImGui::GetKeyData(key)->DownDuration);
				}

				ImGui::Text("Keys pressed:");
				for (ImGuiKey key = ImGuiKey_KeysData_OFFSET; key < ImGuiKey_COUNT; key = (ImGuiKey)(key + 1))
				{
					if (funcs::IsLegacyNativeDupe(key) || !ImGui::IsKeyPressed(key))
						continue;
					ImGui::SameLine();
					ImGui::Text(ImGui::IsNamedKey(key) ? "\"%s\"" : "\"%s\" %d", ImGui::GetKeyName(key), key);
				}

				ImGui::Text("Keys released:");
				for (ImGuiKey key = ImGuiKey_KeysData_OFFSET; key < ImGuiKey_COUNT; key = (ImGuiKey)(key + 1))
				{
					if (funcs::IsLegacyNativeDupe(key) || !ImGui::IsKeyReleased(key))
						continue;
					ImGui::SameLine();
					ImGui::Text(ImGui::IsNamedKey(key) ? "\"%s\"" : "\"%s\" %d", ImGui::GetKeyName(key), key);
				}
			}
         
		}
		ImGui::End();
	}
}
