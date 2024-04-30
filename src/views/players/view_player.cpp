#include "services/gui/gui_service.hpp"
#include "views/view.hpp"

#include "core/scr_globals.hpp"
#include <script/globals/GPBD_FM.hpp>
#include <script/globals/GPBD_FM_3.hpp>

namespace big
{
	void view::view_player()
	{
		ImGui::Checkbox("SPECTATE"_T.data(), &g.player.spectating);

		if (g.player.spectating)
		{
			components::command_checkbox<"overridecamdistance">();

			if (g.player.override_cam_distance)
				components::command_int_slider<"overridecamdistanceint">();
		}

		if (g_player_service->get_selected()->is_valid())
		{
			player_ptr current_player     = g_player_service->get_selected();
			navigation_struct& player_tab = g_gui_service->get_navigation().at(tabs::PLAYER);

			std::string name_appendage{};
			if (current_player->is_host())
			{
				name_appendage += std::format(" [{}]", "VIEW_PLAYER_IS_HOST"_T);
			}
			if (current_player->is_friend())
			{
				name_appendage += std::format(" [{}]", "VIEW_PLAYER_IS_FRIEND"_T);
			}
			if (current_player->is_modder)
			{
				name_appendage += std::format(" [{}]", "MOD"_T);
			}
			if (current_player->is_trusted)
			{
				name_appendage += std::format(" [{}]", "TRUST"_T);
			}

			auto& stats     = scr_globals::gpbd_fm_1.as<GPBD_FM*>()->Entries[current_player->id()].PlayerStats;
			//auto& boss_goon = scr_globals::gpbd_fm_3.as<GPBD_FM_3*>()->Entries[current_player->id()].BossGoon;
			const auto money  = reinterpret_cast<uint64_t&>(stats.Money);
			const auto wallet = reinterpret_cast<uint64_t&>(stats.WalletBalance);			

			//ImGui::Text(std::format("{}: {}", "PLAYER_INFO_WALLET"_T, wallet).c_str());
			//ImGui::Text(std::format("{}: {}", "PLAYER_INFO_BANK"_T, money - wallet).c_str());
			//ImGui::Text(std::format("{}: {}", "PLAYER_INFO_TOTAL_MONEY"_T, money).c_str());

			strcpy(player_tab.name,
			    std::format("{} ({}){} {} {}", current_player->get_name(), current_player->id(), name_appendage, /*wallet, money - wallet,*/ money, wallet)
			        .c_str());

			view::player_info();
			ImGui::SameLine();
			view::player_teleport();

			view::player_kick();
			ImGui::SameLine();
			view::player_toxic();

			view::player_misc();
			ImGui::SameLine();
			view::player_vehicle();
		}
	}
}