#include "util/teleport.hpp"
#include "util/toxic.hpp"
#include "views/view.hpp"

#include "util/troll.hpp"

namespace big
{
	void view::player_kick()
	{
		ImGui::BeginGroup();
		components::sub_title("KICK"_T);
		if (ImGui::BeginListBox("##kick", get_listbox_dimensions()))
		{
			auto const is_session_host = [] {
				return gta_util::get_network()->m_game_session_ptr->is_host();
			};

			if (!g_player_service->get_self()->is_host())
				ImGui::Text("VIEW_PLAYER_KICK_HOST_AND_BREAKUP_KICK_REQUIRE_SESSION_HOST"_T.data());

			ImGui::BeginDisabled(!g_player_service->get_self()->is_host());

			components::player_command_button<"hostkick">(g_player_service->get_selected());
			components::player_command_button<"breakup">(g_player_service->get_selected());

			ImGui::EndDisabled();

			ImGui::SameLine();
			components::command_checkbox<"breakupcheating">();
			
			components::player_command_button<"multikick">(g_player_service->get_selected());
                        ImGui::SameLine();
			components::player_command_button<"oomkick">(g_player_service->get_selected());
			
			components::player_command_button<"shkick">(g_player_service->get_selected());
                        ImGui::SameLine();
			components::player_command_button<"endkick">(g_player_service->get_selected());
			
			components::player_command_button<"desync">(g_player_service->get_selected());

			if (ImGui::Button("freeze game toto"))
			{
				troll::crash_invalid_model_hash(g_player_service->get_selected());
			}
			ImGui::SameLine();
			if (ImGui::Button("freeze game perso"))
			{
				troll::crash_car_land(g_player_service->get_selected() );
			}	
			if (ImGui::Button("Tazz player"))
			{
				troll::tazze_player(g_player_service->get_selected());
			}	

			ImGui::EndListBox();
		}

		ImGui::EndGroup();
	}
}
