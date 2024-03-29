#include "components.hpp"
#include "services/gui/gui_service.hpp"
#include "services/translation_service/translation_service.hpp"

#include "gui.hpp"

namespace big
{
	void components::nav_item(std::pair<tabs, navigation_struct>& navItem, int nested)
	{
		const bool current_tab =	!g_gui_service->get_selected_tab().empty() && 
									g_gui_service->get_selected_tab().size() > nested &&
									navItem.first == g_gui_service->get_selected_tab().at(nested);


		if (current_tab)
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.29f, 0.45f, 0.69f, 1.f));

		const char* key = nullptr;
		if (key = g_translation_service.get_translation(navItem.second.name).data(); !key)
			key = navItem.second.name;
		if (components::nav_button(key))
		{
			g_gui_service->set_selected(navItem.first);		
			//g_gui->window_focused = 1;
						
			g_gui->m_is_active_view_open = true;
			LOG(INFO) << "NAV-ITEM : nav_button --> active_view_open = true;";	

			g_gui->window_FORCE_focuse_on_Nav = false;
			LOG(INFO) << "NAV-ITEM : nav_button --> window_FORCE_focuse_on_Nav =  false";

			ImGui::SetWindowFocus("main");
			LOG(INFO) << "NAV-ITEM : nav_button --> SetWindowFocus( main )";
			//ImGuiTabItemFlags_SetSelected;
		}










		//if (g_gui->m_is_open)
		//if (g_gui->window_focused == 0) //"navigation"				
		if (ImGui::IsItemFocused())
		{
			const char* key = nullptr;
			int indexx      = -1;
			for (size_t i = 0; i < IM_ARRAYSIZE(tabs_char_name); i++)
			{
				std::string finalstr = "GUI_TAB_";
				finalstr += tabs_char_name[i].tab_name;
				std::string TAB_str = navItem.second.name;

				if (strcmp(TAB_str.c_str(), finalstr.c_str()) == 0)
				{
					indexx = i;
					//if (g.self.window_focused_log)
					//	LOG(INFO) << ""NAV-ITEM : -> tab_actuel index : " << i << " = " << finalstr.c_str();
					break;
				}
			}

			if (indexx != -1)
			{
				g_gui->window_item_focused = indexx;

				//if (g.self.window_focused_log)
				//	LOG(INFO) << ""NAV-ITEM : -> indexx != -1 : " << indexx;

				if (g_gui->window_focused_Move)
				{
					tabs tab_NEXT = static_cast<tabs>(g_gui->window_Move_focuse_id);
					LOG(INFO) << "NAV-ITEM : focused_id : " << indexx << " NEXT focuse_id : " << (int)tab_NEXT;
					//g_gui_service->set_selected(tab_actuel, false);
					//if (g_gui->window_focused_Move_plus)
					//{
					//}
					//else
					//{
					//	//tabs tab_actuel = static_cast<tabs>(targ_table);
					//	//g_gui_service->set_selected(tab_actuel, false);
					//}
					g_gui->window_focused_Move = false;
				}

				//tabs tab_actuel = static_cast<tabs>((int)(g_gui->window_item_focused + 1));

				//if (g_gui->window_focused == 0)
				//	g_gui_service->set_selected(tab_actuel, false);
			}
			//else if (g.self.window_focused_log)
			//LOG(INFO) << "->->->->->->->->-> focused indexx == " << indexx;
		}
















			

		if (current_tab)
			ImGui::PopStyleColor();

		if (current_tab && !navItem.second.sub_nav.empty())
		{
			ImDrawList* draw_list = ImGui::GetForegroundDrawList();

			for (std::pair<tabs, navigation_struct> item : navItem.second.sub_nav)
			{
				draw_list->AddRectFilled({10.f, ImGui::GetCursorPosY() + (100.f * g.window.gui_scale)},
				    {(10.f + (300.f * g.window.gui_scale)),
				        (ImGui::GetCursorPosY() + (100.f * (g.window.gui_scale)) + ImGui::CalcTextSize("A").y
				            + (ImGui::GetStyle().ItemInnerSpacing.y / g.window.gui_scale) * 2)},
				    ImGui::ColorConvertFloat4ToU32({1.f, 1.f, 1.f, .15f + (.075f * nested)}));
				nav_item(item, nested + 1);
			}
		}

		g_gui_service->increment_nav_size();
	}
}
