#include "backend/looped/looped.hpp"
#include "gta/pickup_rewards.hpp"
#include "services/players/player_service.hpp"
#include "util/globals.hpp"
#include "util/misc.hpp"

#include "util/math.hpp"
#include "gta/enums.hpp"
#include "script_global.hpp"
#include "util/troll.hpp"

#define SERVICE_SPEND_CASH_DROP_p 2707016 //2764405 //2764230 //2787534 //above SERVICE_SPEND_CASH_DROP  
#define NETWORK_SPENT_CASH_DROP_p 4535851  //4535172 //4534105 //above MPROP_RWD_TCK1m
#define SERVICE_SPEND_CASH_DROP_p0 2707022 //2764411 //2764236 //2787540 above SERVICE_SPEND_CASH_DROP above global1

#define MONEY_NETWORK_SPENT_HOLDUPS_p 2738587 

namespace big
{
	long long last_bounty_time_save = 0;

	void create_holdup_money_pickup(Vector3 pose, int value)
	{
		auto SPENT_HOLDUPS_global = script_global(MONEY_NETWORK_SPENT_HOLDUPS_p);

		*SPENT_HOLDUPS_global.at(5056).as<int*>() = value;

		*SPENT_HOLDUPS_global.at(5058).as<float*>() = pose.x;
		*SPENT_HOLDUPS_global.at(5059).as<float*>() = pose.y;
		*SPENT_HOLDUPS_global.at(5060).as<float*>() = pose.z;

		*script_global(NETWORK_SPENT_CASH_DROP_p).at(*SPENT_HOLDUPS_global.at(5055).as<int*>(), 85).at(66).at(2).as<int*>() = 2;
		*SPENT_HOLDUPS_global.at(7).as<int*>()                                                              = 1 << 1;
	}

	void create_spend_cash_money_pickup(Vector3 pose, int value)
	{
		auto pickup_global = script_global(SERVICE_SPEND_CASH_DROP_p);

		*pickup_global.at(1).as<int*>() = value;

		*pickup_global.at(3).as<float*>() = pose.x;
		*pickup_global.at(4).as<float*>() = pose.y;
		*pickup_global.at(5).as<float*>() = pose.z;

		*script_global(NETWORK_SPENT_CASH_DROP_p).at(*pickup_global.as<int*>(), 85).at(66).at(2).as<int*>() = 2;
		*script_global(SERVICE_SPEND_CASH_DROP_p0).as<int*>()                                               = 1 << 0;
	}

	// rate limit script events to prevent crashes
	static int offRadarPlayer    = 0;
	static int neverWantedPlayer = 0;
	void looped::player_good_options() 
	{
		if (!*g_pointers->m_gta.m_is_session_started)
			return;

		offRadarPlayer++;
		if (offRadarPlayer > 32)
			offRadarPlayer = 0;

		neverWantedPlayer++;
		if (neverWantedPlayer > 32)
			neverWantedPlayer = 0;

		g_player_service->iterate([](const player_entry& entry) {
			if ((g.session.off_radar_all || entry.second->off_radar) && offRadarPlayer == entry.second->id())
				globals::give_remote_otr(entry.second->id());
		});

		g_player_service->iterate([](const player_entry& entry) {
			if ((g.session.never_wanted_all || entry.second->never_wanted)
			    && PLAYER::GET_PLAYER_WANTED_LEVEL(entry.second->id()) > 0 && neverWantedPlayer == entry.second->id())
				globals::clear_wanted_player(entry.second->id());
		});

		if (g.session.semi_godmode_all)
		{
			g_pointers->m_gta.m_give_pickup_rewards(-1, REWARD_HEALTH);
			g_pointers->m_gta.m_give_pickup_rewards(-1, REWARD_ARMOUR);
		}
		else
		{
			g_player_service->iterate([](const player_entry& entry) {
				if (entry.second->semi_godmode)
				{
					if (CPed* ped = entry.second->get_ped())
					{
						if (ped->m_maxhealth == 0 || ped->m_health == 0 || misc::has_bit_set((int*)&ped->m_damage_bits, 8))
							return;

						if (ped->m_health < ped->m_maxhealth)
						{
							g_pointers->m_gta.m_give_pickup_rewards(1 << entry.second->id(), REWARD_HEALTH);
						}

						if (ped->m_armor < 50.0f)
						{
							g_pointers->m_gta.m_give_pickup_rewards(1 << entry.second->id(), REWARD_ARMOUR);
						}
					}
				}
			});
		}

		if (g.session.vehicle_fix_all)
		{
			g_pointers->m_gta.m_give_pickup_rewards(-1, REWARD_VEHICLE_FIX);
		}
		else
		{
			g_player_service->iterate([](const player_entry& entry) {
				if (entry.second->fix_vehicle)
					g_pointers->m_gta.m_give_pickup_rewards(1 << entry.second->id(), REWARD_VEHICLE_FIX);
			});
		}

		g_player_service->iterate([](const player_entry& entry) 
		{
			if (entry.second->pickup_gun || g.session.pickup_gun_all)
			{
				if (CPed* Cped_p = entry.second->get_ped())
				{
					entry.second->pickup_gun_time_now           = std::chrono::steady_clock::now();
					entry.second->pickup_gun_elapsed_time_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
					    entry.second->pickup_gun_time_now - entry.second->pickup_gun_last_time);

					Ped Ped_handle = g_pointers->m_gta.m_ptr_to_handle(Cped_p);

					if (TASK::IS_TASK_MOVE_NETWORK_ACTIVE(Ped_handle) && entry.second->pickup_gun_elapsed_time_in_ms >= 1498ms)
					{
						Vector3 v_s = Cped_p->get_bone_coords(ePedBoneType::HEAD);
						//Vector3 v_e  = Cped_p->get_bone_coords(ePedBoneType::L_HAND);
						Vector3 v_e  = PED::GET_PED_BONE_COORDS(Ped_handle, (int)PedBones::SKEL_L_Finger00, 0, 0, 0);
						Vector3 v_d  = math::VecteurDirection(v_s, v_e);
						Vector3 v_dn = math::normaliserVecteur(v_d);

						Vector3 hitCords;
						if (math::ray_cast_hitCords(&hitCords, v_s, v_dn, Ped_handle))
						{
							v_s = Cped_p->get_bone_coords(ePedBoneType::HEAD);
							if (1.5f < math::distance_between_vectors(hitCords, v_s))
								create_spend_cash_money_pickup(hitCords, 2000);
							//OBJECT::CREATE_AMBIENT_PICKUP(RAGE_JOAAT("PICKUP_ARMOUR_STANDARD"),
							//    hitCords.x,
							//    hitCords.y,
							//    hitCords.z,
							//    0,
							//    0,
							//    RAGE_JOAAT("Prop_Armour_Pickup"),
							//    false,
							//    true);
						}
						entry.second->pickup_gun_last_time = entry.second->pickup_gun_time_now;
					}
				}
			}

			if (entry.second->spend_holdup)
			{
				if (CPed* Cped_p = entry.second->get_ped())
				{
					entry.second->spend_holdup_time_now = std::chrono::steady_clock::now();
					entry.second->spend_holdup_elapsed_time_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
					    entry.second->spend_holdup_time_now - entry.second->spend_holdup_last_time);

					if (entry.second->spend_holdup_elapsed_time_in_ms >= 556ms)
					{
						Vector3 v_s = Cped_p->get_bone_coords(ePedBoneType::HEAD);
						v_s.z += 1.5f;
						create_holdup_money_pickup(v_s, 2400);
						entry.second->spend_holdup_last_time = entry.second->spend_holdup_time_now;
					}
				}
			}

			if (entry.second->win_10k_bounty || g.session.win_10k_bounty_all)
			{
				long long last_bounty_time = (long long)(*scr_globals::gsbd_fm_events.at(9).as<int*>());
				if (last_bounty_time != entry.second->last_bounty_time_save)
				{
					entry.second->last_bounty_time_save = last_bounty_time;
					troll::win_bounty_on_player(entry.second, 10000, false);
					troll::win_bounty_on_player(entry.second, 10000, false);
					troll::win_bounty_on_player(entry.second, 10000, false);
					troll::win_bounty_on_player(entry.second, 10000, false);
					//troll::win_bounty_on_player(entry.second, 10000, false);
				}
			}

			if (entry.second->win_ceo_payement)
			{
				if (CPed* Cped_p = entry.second->get_ped())
				{
					entry.second->win_ceo_time_now           = std::chrono::steady_clock::now();
					entry.second->win_ceo_elapsed_time_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
					    entry.second->win_ceo_time_now - entry.second->win_ceo_last_time);

					if (entry.second->win_ceo_elapsed_time_in_ms >= 7000ms)
					{
						troll::ceo_payement(entry.second, 30000, false);

						entry.second->win_ceo_last_time = entry.second->win_ceo_time_now;
					}
				}
			}

			//if (CPed* Cped_p = entry.second->get_ped())
			//if (Cped_p->m_weapon_manager != nullptr)
			//{
			//	int64_t pwm    = reinterpret_cast<int64_t>(g_local_player->m_weapon_manager);
			//	Vector3 imppoz = *reinterpret_cast<Vector3*>(pwm + 432);
			//}

		});
	}
}
