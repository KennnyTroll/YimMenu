#include "services/gui/gui_service.hpp"
#include "views/view.hpp"

#include "gui.hpp"
#include "pointers.hpp"
#include "services/model_preview/model_preview_service.hpp"

namespace big
{
	void view::navigation()
	{
		ImGui::SetNextWindowPos({10.f, 100.f * g.window.gui_scale}, ImGuiCond_Always);
		ImGui::SetNextWindowSize({300.f * g.window.gui_scale, 0.f}, ImGuiCond_Always);

		if (ImGui::Begin("navigation", 0, window_flags))
		{
			g_gui_service->reset_nav_size();
			for (std::pair<tabs, navigation_struct> nav_item : g_gui_service->get_navigation())
			{
				switch (nav_item.first)
				{
				case tabs::PLAYER:
				case tabs::DEBUG: continue;
				default: components::nav_item(nav_item, 0);
				}
			}

			if (ImGui::IsWindowFocused())
			{			
				if (ImGui::IsKeyReleased(ImGuiKey_GamepadDpadRight))
				{
					//LOG(INFO) << "->->-> DPAD_RIGHT : focused window item : " << g_gui->window_item_focused
					//          << " : name : " << tabs_char_name[g_gui->window_item_focused].tab_name;

					if (g_gui->window_item_focused != -1)
					{
						//LOG(INFO) << "->->-> DPAD_RIGHT : window_item_focused != -1";
										
						tabs tab_actuel = static_cast<tabs>(g_gui->window_item_focused);
						//LOG(INFO) << "->->-> DPAD_RIGHT : tab_actuel";

						g_gui_service->set_selected(tab_actuel);
						//LOG(INFO) << "->->-> DPAD_RIGHT : set_selected(tab_actuel)";

						g_gui->m_is_active_view_open = true;
						//LOG(INFO) << "->->-> DPAD_RIGHT : active_view_open = true";

						g_gui->window_FORCE_focuse_on_Nav = true;
						//LOG(INFO) << "->->-> DPAD_RIGHT : window_FORCE_focuse_on_Nav =  true";
					}
				}

				if (ImGui::IsKeyReleased(ImGuiKey_GamepadDpadLeft))
				{
					// player count does not include ourself that's why +1
					const auto player_count = g_player_service->players().size() + 1;
					if (!*g_pointers->m_gta.m_is_session_started && player_count < 2)
						return;

					g_gui->window_FORCE_focuse_on_Nav = false;
					//LOG(INFO) << "<-<-<- DPAD_LEFT : window_FORCE_focuse_on_Nav =  false";

					if (g_player_service->get_selected()->get_name() == "")
					{
						g_player_service->set_selected(g_player_service->get_self());
					}
					//LOG(INFO) << "<-<-<- DPAD_LEFT : get_selected() == " << g_player_service->get_selected()->get_name();

					g_gui_service->set_selected(tabs::PLAYER);
					//LOG(INFO) << "<-<-<- DPAD_LEFT : set_selected(tabs::PLAYER)";

					g_gui->m_is_active_view_open = true;
					//LOG(INFO) << "<-<-<- DPAD_LEFT : active_view_open = true";

					ImGui::SetWindowFocus("playerlist");
					//LOG(INFO) << "<-<-<- DPAD_LEFT : SetWindowFocus(playerlist)";
				}
			}
		}
		ImGui::End();
	}
}
