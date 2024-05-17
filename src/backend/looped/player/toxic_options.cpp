#include "backend/looped/looped.hpp"
#include "gta/pickup_rewards.hpp"
#include "services/players/player_service.hpp"
#include "util/globals.hpp"
#include "util/misc.hpp"
#include "util/toxic.hpp"

#include "pointers.hpp"
#include "gta/net_object_mgr.hpp"
#include "util/entity.hpp"

#include "base/CObject.hpp"

namespace big
{
	void looped::player_toxic_options()
	{
		if (!*g_pointers->m_gta.m_is_session_started)
			return;

		int rotate_cam_bits = 0;

		g_player_service->iterate([&rotate_cam_bits](const player_entry& entry) {
			if (g_player_service->get_self()->get_ped() && entry.second->get_ped() && entry.second->get_ped()->m_health > 0)
			{
				if (entry.second->kill_loop && !(entry.second->get_ped()->m_damage_bits & (1 << 8)))
					g_pointers->m_gta.m_send_network_damage(g_player_service->get_self()->get_ped(),
					    entry.second->get_ped(),
					    entry.second->get_ped()->m_navigation->get_position(),
					    0,
					    true,
					    "weapon_explosion"_J,
					    10000.0f,
					    2,
					    0,
					    (1 << 4),
					    0,
					    0,
					    0,
					    false,
					    false,
					    true,
					    true,
					    nullptr);

				if (entry.second->explosion_loop)
					toxic::blame_explode_player(entry.second, entry.second, EXP_TAG_SUBMARINE_BIG, 9999.0f, true, false, 9999.0f);

				if (entry.second->ragdoll_loop && entry.second->get_ped()->m_net_object)
					g_pointers->m_gta.m_request_ragdoll(entry.second->get_ped()->m_net_object->m_object_id);

				if (entry.second->rotate_cam_loop)
					rotate_cam_bits |= (1 << entry.second->id());
			}

			if (rotate_cam_bits)
			{
				const size_t arg_count = 4;
				int64_t args[arg_count] = {(int64_t)eRemoteEvent::TSECommand, (int64_t)self::id, (int64_t)eRemoteEvent::TSECommandRotateCam, rotate_cam_bits};

				g_pointers->m_gta.m_trigger_script_event(1, args, arg_count, rotate_cam_bits, (int)eRemoteEvent::TSECommand);
			}
		});

		g_player_service->iterate([](const player_entry& entry) 
		{	
			if (g.protections.redirect_cage_object && entry.second->redirect_cage_object)
			{
				if (entry.second->get_ped() && entry.second->redirect_cage_object_Obj != -1) [[unlikely]]
				{	
					//rage::netObject* net_obj =
					//    g_pointers->m_gta.m_get_net_object(*g_pointers->m_gta.m_network_object_mgr, entry.second->target_object_id, false);
					//if (net_obj)
					//{
						//CObject* cobj = net_obj->GetGameObject();
						CObject* cobj = reinterpret_cast<CObject*>(g_pointers->m_gta.m_handle_to_ptr(entry.second->redirect_cage_object_Obj));				

						if (cobj)
						{

							entry.second->redirect_cage_object_time_now = std::chrono::steady_clock::now();
							entry.second->redirect_cage_object_time_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
							    entry.second->redirect_cage_object_time_now - entry.second->redirect_cage_object_last_time);

							CPed* tar          = entry.second->get_ped();
							Vector3 tar_pz     = *tar->m_navigation->get_position();
							Vector3 tar_pz_sav = tar_pz;
							Vector3 net_obj_pz = *cobj->m_navigation->get_position();

							net_obj_pz.z = 0;
							tar_pz.z     = 0;

							Vector3 min, max, result;
						    MISC::GET_MODEL_DIMENSIONS(cobj->m_model_info->m_hash, &min, &max);
						    result   = max - min;
						    float y_offset = result.y;

							if ((math::distance_between_vectors(tar_pz, net_obj_pz) > (y_offset * 1.5f)) && (entry.second->redirect_cage_object_time_in_ms >= 2000ms))
							{
								//LOG(INFO) << std::format("===> REDIRECT CAGE OBJECT - object id {} => FROM {} To New Position",
								//    (int)entry.second->target_object_id,
								//    entry.second->get_name());

								//Object Obj = g_pointers->m_gta.m_ptr_to_handle(cobj);
							    //ENTITY::SET_ENTITY_COORDS(Obj, tar_pz_sav.x, tar_pz_sav.y, tar_pz_sav.z + entry.second->redirect_cage_object_z_dist, 0, 0, 0, 0);
							    if (entity::take_control_of(entry.second->redirect_cage_object_Obj,500))
									ENTITY::SET_ENTITY_COORDS(entry.second->redirect_cage_object_Obj, tar_pz_sav.x, tar_pz_sav.y, tar_pz_sav.z + entry.second->redirect_cage_object_z_dist, 0, 0, 0, 0);
								
								entry.second->redirect_cage_object_last_time = entry.second->redirect_cage_object_time_now;
							}							
						}
					//}
				}
			}		
		});

	}
}
