#include "view_debug.hpp"

#include "services/gui/gui_service.hpp"

#include "gui.hpp"

namespace big
{
	void debug::main()
	{
		if (strcmp(g_gui_service->get_selected()->name, "GUI_TAB_DEBUG"))
			return;

		if (ImGui::Begin("DEBUG_WINDOW"_T.data()))
		{
			if (ImGui::IsWindowFocused())
			{
				g_gui->window_focused = 3;
			}

			ImGui::BeginTabBar("debug_tabbar");
			misc();
			logs();
			globals();
			locals();
			script_events();
			scripts();
			threads();
			if (ImGui::BeginTabItem("GUI_TAB_ANIMATIONS"_T.data()))
			{
				animations();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::End();
	}
}