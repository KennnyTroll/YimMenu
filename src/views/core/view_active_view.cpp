#include "views/view.hpp"

#include "lua/lua_manager.hpp"
#include "pointers.hpp"
#include "services/gui/gui_service.hpp"
#include "services/translation_service/translation_service.hpp"

#include "gui.hpp"


namespace big
{	
	static auto last_time         = std::chrono::steady_clock::now();

	void view::active_view()
	{
		if (!g_gui->m_is_active_view_open || g_gui_service->get_selected_tab().empty())
		{			
			return;
		}
		const auto selected = g_gui_service->get_selected();

		if (selected->func == nullptr &&
			(g_lua_manager && !g_lua_manager->has_gui_to_draw(selected->hash)))
		{
			return;
		}

		constexpr float alpha = 1.f;

		ImGui::SetNextWindowPos({(300.f + 20.f) * g.window.gui_scale, 100.f * g.window.gui_scale}, ImGuiCond_Always);
		ImGui::SetNextWindowSize({0.f, 0.f});
		ImGui::SetNextWindowSizeConstraints({300.f, 100.f},
		    {(float)*g_pointers->m_gta.m_resolution_x - 270.f, (float)*g_pointers->m_gta.m_resolution_y - 110.f});

		if (ImGui::Begin("main", nullptr, window_flags))
		{
			if (ImGui::IsWindowFocused())
			{
				if (g_gui->window_FORCE_focuse_on_Nav == true)
				{
					g_gui->window_FORCE_focuse_on_Nav = false;
					//LOG(INFO) << "->->-> ACTIVE_VIEW : FORCE_focuse_on_Nav : false";

					ImGui::PopStyleVar();
					ImGui::End();	
					
					ImGui::SetWindowFocus("navigation");
					//LOG(INFO) << "->->-> ACTIVE_VIEW : FORCE_focuse --> SetWindowFocus( navigation )";
					
					LOG(INFO) << "->->-> FORCE_focuse navigation : Test ??";

					return;
				}
			}

			const char* key = nullptr;
			if (key = g_translation_service.get_translation(selected->hash).data(); !key)
				key = selected->name;

			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
			components::title(key);
			ImGui::Separator();

			if (selected->func)
			{
				selected->func();

				const auto has_lua_gui_to_draw = g_lua_manager && g_lua_manager->has_gui_to_draw(selected->hash);
				if (has_lua_gui_to_draw)
					ImGui::Separator();
			}

			if (g_lua_manager)
				g_lua_manager->draw_gui(selected->hash);

			ImGui::PopStyleVar();
		}

		if (ImGui::IsWindowFocused())
		{
			const auto time_now           = std::chrono::steady_clock::now();
			const auto elapsed_time_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_now - last_time);

			if (g_gui_service->get_selected_tab().at(0) == tabs::PLAYER)
			{
				if (ImGui::IsKeyDown(ImGuiKey_GamepadR1) && ImGui::IsKeyDown(ImGuiKey_GamepadL1) && elapsed_time_in_ms >= 500ms)
				{
					g_gui->m_is_active_view_open = !g_gui->m_is_active_view_open;
					//LOG(INFO) << "<-<-<- DPAD_LEFT : active_view_open = " << g_gui->m_is_active_view_open;
					last_time = time_now;
				}
				else
				if (ImGui::IsKeyPressed(ImGuiKey_GamepadL1))
				{
					const auto player_count = g_player_service->players().size() + 1;

					if (player_count <= 1)
					{
						g_player_service->set_selected(g_player_service->get_self());
					}
					else
					{
						int selected_player_index = 0;
						bool founded              = false;
						for (const auto& [_, player] : g_player_service->players())
						{
							selected_player_index += 1;
							if (g_player_service->get_selected() == player)
							{
								founded = true;
								//LOG(INFO) << "A-A-A- DPAD_UP : ACTUAL selected() == " << g_player_service->get_selected()->get_name();
								break;
							}
						}

						if (!founded) // set last
						{
							//LOG(INFO) << "A-A-A- DPAD_UP : ACTUAL selected() == SELF --> LAST";

							int next_target_player_index = player_count - 1;
							selected_player_index        = 0;
							for (const auto& [_, player] : g_player_service->players())
							{
								selected_player_index += 1;
								if (selected_player_index == next_target_player_index)
								{
									g_player_service->set_selected(player);
									break;
								}
							}
						}
						else if (selected_player_index == 1) // set first get_self
						{
							//LOG(INFO) << "A-A-A- DPAD_UP : SET selected() == SELF";
							g_player_service->set_selected(g_player_service->get_self());
						}
						else
						{
							int next_target_player_index = selected_player_index - 1;
							selected_player_index        = 0;
							for (const auto& [_, player] : g_player_service->players())
							{
								selected_player_index += 1;
								if (selected_player_index == next_target_player_index)
								{
									//LOG(INFO) << "A-A-A- DPAD_UP : SET selected() == " << player->get_name();
									g_player_service->set_selected(player);
									break;
								}
							}
						}
					}
				}
				else
				if (ImGui::IsKeyPressed(ImGuiKey_GamepadR1))
				{
					const auto player_count = g_player_service->players().size() + 1;

					if (player_count <= 1)
					{
						g_player_service->set_selected(g_player_service->get_self());
					}
					else
					{
						int selected_player_index = 0;
						bool founded              = false;
						for (const auto& [_, player] : g_player_service->players())
						{
							selected_player_index += 1;
							if (g_player_service->get_selected() == player)
							{
								founded = true;
								//LOG(INFO) << "V-V-V- DPAD_DOWN : ACTUAL selected() == " << g_player_service->get_selected()->get_name();
								break;
							}
						}

						if (!founded) //
						{
							//LOG(INFO) << "V-V-V- DPAD_DOWN : ACTUAL selected() == SELF";

							for (const auto& [_, player] : g_player_service->players())
							{
								//LOG(INFO) << "V-V-V- DPAD_DOWN : SET selected() == " << player->get_name();
								g_player_service->set_selected(player);
								break;
							}
						}
						else if ((player_count - 1) < selected_player_index + 1) // set first
						{
							//LOG(INFO) << "V-V-V- DPAD_DOWN : SET selected() == SELF";
							g_player_service->set_selected(g_player_service->get_self());
						}
						else
						{
							int next_target_player_index = selected_player_index + 1;
							selected_player_index        = 0;
							for (const auto& [_, player] : g_player_service->players())
							{
								selected_player_index += 1;
								if (selected_player_index == next_target_player_index)
								{
									//LOG(INFO) << "V-V-V- DPAD_DOWN : SET selected() == " << player->get_name();
									g_player_service->set_selected(player);
									break;
								}
							}
						}
					}
				}
			}
		}

		ImGui::End();


	}
}
