#include "gui_service.hpp"
#include "gui.hpp"
//#include "services/gui/gui_service.hpp"

namespace big
{
	gui_service::gui_service()
	{
		g_gui_service = this;
	}

	gui_service::~gui_service()
	{
		g_gui_service = nullptr;
	}

	navigation_struct* gui_service::get_selected()
	{
		static navigation_struct tab_none = {"", nullptr};
		if (current_tab.empty() || current_tab.at(0) == tabs::NONE)
			return &tab_none;

		navigation_struct* current_nav = &nav.at(current_tab.at(0));
		if (current_tab.size() > 1)
		{
			for (const tabs& t : current_tab)
			{
				if (t == current_tab.at(0))
					continue;
				current_nav = &current_nav->sub_nav.at(t);
			}
		}	

		return current_nav;
	}

	std::vector<tabs>& gui_service::get_selected_tab()
	{
		return current_tab;
	}

	bool gui_service::has_switched_view()
	{
		return switched_view;
	}

	void gui_service::set_selected(tabs tab/*, bool focus_on_select*/)
	{
		LOG(INFO) << "Set_selected : -->";
		if (!current_tab.empty())
			LOG(INFO) << "Set_selected : --> ACTUAL : selected --> name : " << get_selected()->name;

		if (current_tab.empty())
		{
			LOG(INFO) << "Set_selected : --> ACTUAL EMPTY : selected : current_tab.push_back(tab)  -->";

			//static navigation_struct tab_none = {"", nullptr};
			//if (current_tab.empty() || current_tab.at(0) == tabs::NONE)
			//	return &tab_none;

			return current_tab.push_back(tab);
		}		

		if (auto it = get_selected()->sub_nav.find(tab); it != get_selected()->sub_nav.end())
		{
			LOG(INFO) << "Set_selected : --> SUBNAV : get_selected()->sub_nav.find(tab) : current_tab.push_back(tab)  -->";
			current_tab.push_back(tab);

			//if ((get_selected()->name != "GUI_TAB_SELF") && 
			//	(get_selected()->name != "GUI_TAB_VEHICLE") && 
			//	(get_selected()->name != "GUI_TAB_WORLD") && 
			//	(get_selected()->name != "GUI_TAB_NETWORK") && 
			//	(get_selected()->name != "GUI_TAB_SETTINGS"))
			//{
			//ImGui::SetWindowFocus("main");
			//g_gui->window_focused = 1;
			//}
			
		}
		else
		{
			LOG(INFO) << "Set_selected : --> NO SUBNAV : current_tab.pop_back(); set_selected(tab, false)  -->";
			current_tab.pop_back();
			set_selected(tab);				
		}
		
		LOG(INFO) << "Set_selected : --> name : " << get_selected()->name;

		//LOG(INFO) << "set_selected --> g_gui->m_is_active_view_open = true;";
		//g_gui->m_is_active_view_open = true;




		//g_gui->window_focused = 0;
		//ImGui::SetWindowFocus("navigation");
		//LOG(INFO) << "set_selected --> SetWindowFocus(navigation)";

		//if (focus_on_select == true)
		//{
		//	LOG(INFO) << "set_selected --> (focus_on_select == true) : ImGui::SetWindowFocus( --> main <-- )  -->";
		//	ImGui::SetWindowFocus("main");
		//	g_gui->window_focused = 1;
		//}

		////g_gui->m_is_active_view_open = true;
		//ImGui::SetWindowFocus("main");
		
		//if (g_gui_service->get_selected()->name)
		//{
		//	ImGui::SetWindowFocus("main");
		//}		
	}

	void gui_service::set_nav_size(int nav_size)
	{
		nav_ctr = nav_size;
	}

	void gui_service::increment_nav_size()
	{
		nav_ctr++;
	}


	void gui_service::reset_nav_size()
	{
		nav_ctr = 0;
	}

	std::map<tabs, navigation_struct>& gui_service::get_navigation()
	{
		return nav;
	}

	void gui_service::remove_from_nav_internal(std::map<big::tabs, big::navigation_struct>& nav, big::tabs existing_tab_id)
	{
		std::erase_if(nav, [&](auto& nav_item) {
			if (nav_item.first == existing_tab_id)
			{
				set_selected(tabs::NONE);
				return true;
			}
			else
			{
				remove_from_nav_internal(nav_item.second.sub_nav, existing_tab_id);
			}

			return false;
		});
	}

	void gui_service::remove_from_nav(tabs existing_tab_id)
	{
		remove_from_nav_internal(get_navigation(), existing_tab_id);
	}
}
