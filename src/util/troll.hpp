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

#include "base/CObject.hpp"
#include <entities/CDynamicEntity.hpp>

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
		//scr_globals::gpbd_fm_3.as<GPBD_FM_3*>()->Entries[self::id].BountyPlacedBy = target->id();		
		//scr_globals::gpbd_fm_3.as<GPBD_FM_3*>()->Entries[self::id].at(511).as<int*>() = target->id();
		//int ceo_paye_global     = *script_global(1886967).at(target->id(), 609).at(511).as<int*>();
		const size_t arg_count  = 9;
		int64_t args[arg_count] = {(int64_t)eRemoteEvent::CeoMoney,
		    self::id,
		    -1, //target->id() ??
		    value,
		    198210293, //198210293  1553945504
			1,//1
		    //scr_globals::gpbd_fm_3.as<GPBD_FM_3*>()->Entries[target->id()].BountyAmount,
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

	static auto get_hash(const std::string& str)
	{
		rage::joaat_t hash = 0;
		if (str.substr(0, 2) == "0x")
			std::stringstream(str.substr(2)) >> std::hex >> hash;
		else
			hash = rage::joaat(str.c_str());
		return hash;
	}

	static void crash_invalid_model_hash(player_ptr target/*, std::string freeze_model*/)
	{
		big::g_fiber_pool->queue_job([target]()
		{
			auto plyr             = g_player_service->get_by_id(target->id());
			
			LOG(INFO) << "frezz_game_sync net_obj  true";

			if (CPed* Cped_target = target->get_ped())
			{
				Vector3 coords         = *Cped_target->m_navigation->get_position();
  
				Hash fake_hash = get_hash(g.protections.freeze_fake_model);
				//Hash hash     = "prop_thindesertfiller_aa"_J;
				auto info     = model_info::get_model(fake_hash);
				if (info)
				{
					LOG(INFO) << "info m_model_type " << (int)info->m_model_type;
				}
				
				Object Obje         = entity::spawn_object(fake_hash, coords);
				if (Obje != NULL)
				{
					ENTITY::SET_ENTITY_COLLISION(Obje, 0, 1);
					
					////target->frezz_game_sync_object_id = net_Obje_id;
					target->frezz_game_sync           = true;

					////LOG(INFO) << std::format("frezz_game_sync net_obj  id {}", (int)net_Obje_id);
					////static char freeze_model[64];
					////std::memcpy(freeze_model, g.protections.freeze_model.c_str(), 12);

					auto ptr = (rage::CDynamicEntity*)g_pointers->m_gta.m_handle_to_ptr(Obje);
					//LOG(INFO) << "ptr ?";
					if (ptr != nullptr && ptr->m_net_object != nullptr)
					{
						target->frezz_game_sync_object_id = ptr->m_net_object->m_object_id;
						LOG(INFO) << std::format("frezz_game_sync net_obj  id {}", (int)ptr->m_net_object->m_object_id);		
					}					

					////info->m_hash = get_hash(g.protections.freeze_model);
					CObject* Obj = (CObject*)g_pointers->m_gta.m_handle_to_ptr(Obje);	
					//if (Obj != nullptr && Obj->m_model_info != nullptr)
					//	Obj->m_model_info->m_hash = get_hash(g.protections.freeze_model);		


					//LOG(INFO) << std::format("info m_model_type {} founded {:X}  NEW\n",
					//    (int)info->m_model_type,
					//    Obj->m_model_info->m_hash);

					script::get_current()->yield(3s);
					target->frezz_game_sync   = false;
					Obj->m_model_info->m_hash = fake_hash;
					entity::delete_entity(Obje, true);
					////info->m_hash = fake_hash;
					

					

					LOG(INFO) << "frezz_game_sync net_obj  false";
				}
				else
					LOG(INFO) << "frezz_game_sync net_obj Failed to creat net_obj";
			}
		});
	}

	static void crash_car_land(player_ptr target /*, int target_id*/)
	{
		//big::g_fiber_pool->queue_job([target]() {
		//	if (CPed* Cped_target = target->get_ped())
		//	{
		//		Vector3 coords = *Cped_target->m_navigation->get_position();
		//		Hash hash = "adder"_J;
		//		auto vehicle_model_info = model_info::get_vehicle_model(hash);
		//		Vehicle veh = entity::spawn_veh_crash(hash, coords);
		//		if (veh != NULL)
		//		{
		//			//NET_OBJ_TYPE_HELI
		//			vehicle_model_info->m_vehicle_type = eVehicleType::VEHICLE_TYPE_HELI;					
		//			VEHICLE::SET_SHORT_SLOWDOWN_FOR_LANDING(veh);	
		//			script::get_current()->yield(1s);
		//			entity::delete_entity(veh, true);
		//			vehicle_model_info->m_vehicle_type = eVehicleType::VEHICLE_TYPE_CAR;
		//		}
		//	}
		//});

		big::g_fiber_pool->queue_job([target]() {
			if (CPed* Cped_target = target->get_ped())
			{
				//CPed* local_ped = Cped_target->get_self()->get_ped();
				//rage::fvector3 fcoords = *Cped_target->m_navigation->get_position();
				//Vector3 coords         = {fcoords.x, fcoords.y, fcoords.z};
				Vector3 coords = *Cped_target->m_navigation->get_position();

				//Vector3 coords = Cped_target->get_bone_coords(ePedBoneType::HEAD);

				Hash hash = get_hash(g.protections.freeze_fake_model);
				//Hash hash     = "prop_thindesertfiller_aa"_J;
				auto info     = model_info::get_model(hash);
				if (info)
				{
					LOG(INFO) << "info m_model_type " << (int)info->m_model_type;
				}

				int16_t Obje_id = 0;
				Object object   = entity::spawn_object(hash, coords);
				if (object != NULL)
				{
					LOG(INFO) << "info m_model_type " << (int)info->m_model_type;
					//info->m_hash = "prop_thindesertfiller_aa"_J;
					// info->m_hash = "proc_leafybush_01"_J;
					//script::get_current()->yield(1s);
					//entity::delete_entity(object, true);
					//info->m_hash = hash;
					//entity::delete_entity(object, true);
					//g_notification_service.push_crash_success("CRASH_INVALID_MODEL_HASH_MESSAGE"_T.data());
				}
				else
					LOG(INFO) << "info " << g.protections.freeze_fake_model << " model " << hash << " type " << (int)info->m_model_type << "   NOT Spawned ";
			}
		});
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
