#include "views/view.hpp"

#include "lua/lua_manager.hpp"
#include "pointers.hpp"
#include "services/gui/gui_service.hpp"
#include "services/translation_service/translation_service.hpp"

#include "gui.hpp"

namespace big
{
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
		ImGui::End();
	}
}
