#include "hooking/hooking.hpp"
#include "services/players/player_service.hpp"
#include "util/notify.hpp"

#include "base/CObject.hpp"
#include "core/data/task_types.hpp"
#include "util/model_info.hpp"
#include "services/gta_data/gta_data_service.hpp"
#include <entities/CDynamicEntity.hpp>

#include "util/entity.hpp"

namespace big
{
	std::string log_entity_info(rage::netObject* pObject)
	{
		std::string mess = "Entity info: \n ";
		//CObject* game_obj = net_obj->GetGameObject();
		auto net_obj_p = g_pointers->m_gta.m_get_net_object(*g_pointers->m_gta.m_network_object_mgr, pObject->m_object_id, false);
		if (!net_obj_p)
			mess += std::format("id {}\n type {}\n", pObject->m_object_id, net_object_type_strs[pObject->m_object_type]);
		else
		{
			//mess += std::format("{}\n {}\n  {}\n",
			//    netobj->m_object_id,
			//    net_object_type_strs[net_obj->m_object_type],
			//    get_model_hash_string(net_obj->GetGameObject()->m_model_info->m_hash));

			mess += std::format("id {}\n type {}\n", pObject->m_object_id, net_object_type_strs[pObject->m_object_type]);
			//get_model_hash_string(net_obj->GetGameObject()->m_model_info->m_hash));

			uint32_t model = pObject->GetGameObject()->m_model_info->m_hash;
			auto info      = model_info::get_model(model);
			if (!info)
				mess += std::format("0x{:X}\n", model);
			else
			{
				mess += std::format("m_model_type {}\n", (int)info->m_model_type);
				//LOG(INFO) << "m_model_type " << (int)info->m_model_type;
				const char* model_str = nullptr;
				if (info->m_model_type == eModelType::Vehicle)
				{
					for (auto& [name, data] : g_gta_data_service->vehicles())
					{
						if (data.m_hash == model)
						{
							model_str = name.data();
						}
					}
				}
				else if (info->m_model_type == eModelType::Ped || info->m_model_type == eModelType::OnlineOnlyPed)
				{
					for (auto& [name, data] : g_gta_data_service->peds())
					{
						if (data.m_hash == model)
						{
							model_str = name.data();
						}
					}
				}
				if (!model_str)
					mess += std::format("0x{:X}\n", model);
				else
				{
					mess += std::format("{} (0x{:X})\n", model_str, model);
				}
			}

			LOG(INFO) << mess;
		}

		return mess;
	}

	eAckCode hooks::received_clone_sync(CNetworkObjectMgr* mgr, CNetGamePlayer* src, CNetGamePlayer* dst, eNetObjType object_type, uint16_t object_id, rage::datBitBuffer* buffer, uint16_t unk, uint32_t timestamp)
	{
		if (src->m_player_id != self::id) [[unlikely]]
		if (object_type < eNetObjType::NET_OBJ_TYPE_AUTOMOBILE || object_type > eNetObjType::NET_OBJ_TYPE_TRAIN) [[unlikely]]
		{
			notify::crash_blocked(src, "out of bounds object type");
			return eAckCode::ACKCODE_FAIL;
		}

		if (auto net_obj = g_pointers->m_gta.m_get_net_object(mgr, object_id, true); net_obj && net_obj->m_object_type != (int16_t)object_type) [[unlikely]]
		{
			notify::crash_blocked(src, "incorrect object type");
			return eAckCode::ACKCODE_FAIL;
		}


		auto plyr = g_player_service->get_by_id(src->m_player_id);

		if (plyr && plyr->block_clone_sync) [[unlikely]]
			return eAckCode::ACKCODE_FAIL;




		if (plyr && plyr->recev_log) [[unlikely]]
		{
			LOG(INFO) << std::format("RECEIVED CLONE SYNC - object id {} net type {} => FROM {}", (int)object_id, (int)object_type, plyr->get_name());

			entity::entity_info_mgr(mgr, (int)object_id);
		}

		//if (plyr && object_id == plyr->target_object_id && plyr->target_object_id != -1) [[unlikely]]
		//{
		//	LOG(INFO) << std::format("===> RECEIVED CLONE SYNC - object id {} net type {} => FROM {}", (int)object_id, (int)object_type, plyr->get_name());
		//	entity::entity_info_mgr(mgr, (int)object_id);
		//	//plyr->target_object_id = -1;
		//}

		g.m_syncing_player      = src;
		g.m_syncing_object_type = object_type;					

		g.debug.fuzzer.thread_id = GetCurrentThreadId();
		if (g.debug.fuzzer.enabled_object_types[(int)object_type]) [[unlikely]]
			g.debug.fuzzer.active = true;
		g.debug.fuzzer.syncing_object_id = object_id;
		auto ret = g_hooking->get_original<received_clone_sync>()(mgr, src, dst, object_type, object_id, buffer, unk, timestamp);
		g.debug.fuzzer.active = false;

		return ret;
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

	void hooks::pack_clone_create(rage::netObjectMgrBase* mgr, rage::netObject* pnObject, rage::netPlayer* netPlayer, rage::datBitBuffer* msgBuffer)
	{
		if (static_cast<eNetObjType>(pnObject->m_object_type) != eNetObjType::NET_OBJ_TYPE_PLAYER)
		{
			auto plyr         = g_player_service->get_by_id(netPlayer->m_player_id);
			CObject* game_obj = pnObject->GetGameObject();

			if (game_obj != nullptr)
			{
				//if (plyr != nullptr && pnObject->m_object_id == plyr->target_object_id)
				//{
				//	LOG(INFO) << std::format("===> PACK CLONE CREAT - object id {} => FROM {}", (int)pnObject->m_object_id, plyr->get_name());
				//	entity::entity_info_netObj(pnObject);
				//}
			}
			
			if (plyr != nullptr && plyr->frezz_game_sync) [[unlikely]]
			{				
				int32_t previous_hash = -1;
				if (game_obj != nullptr && game_obj->m_model_info != nullptr)
				{
					//Object Obj = g_pointers->m_gta.m_ptr_to_handle(game_obj);
					if (pnObject->m_object_id == plyr->frezz_game_sync_object_id)
					{
						LOG(INFO) << "Packing clone CREATE crash FOR " << plyr->get_name();

						if (CPed* Cped_target = plyr->get_ped())
						{
							Vector3 coords = *Cped_target->m_navigation->get_position();
							LOG(INFO) << std::format("Packing clone CREATE crash target position {:.03f} {:.03f} {:.03f} ",
							    coords.x,
							    coords.y,
							    coords.z);

							Object Obj = g_pointers->m_gta.m_ptr_to_handle(game_obj);
							//if (entity::take_control_of(self::veh, 0))
							ENTITY::SET_ENTITY_COORDS(Obj, coords.x, coords.y, coords.z, 0, 0, 0, 0);
							//*game_obj->m_navigation->get_position() = *Cped_target->m_navigation->get_position();
							coords = *game_obj->m_navigation->get_position();
							LOG(INFO) << std::format("Packing clone CREATE crash object NEW position {:.03f} {:.03f} {:.03f} ",
							    coords.x,
							    coords.y,
							    coords.z);

						}

						previous_hash = game_obj->m_model_info->m_hash;
						if (previous_hash != (int32_t)get_hash(g.protections.freeze_model))
						{
							LOG(INFO) << std::format("Packing clone CREATE crash netobject_id {} previous_hash {:X}", (int)pnObject->m_object_id, previous_hash);
							game_obj->m_model_info->m_hash = get_hash(g.protections.freeze_model);
							LOG(INFO) << std::format("Packing clone CREATE crash netobject hash  {:X}\n",
							    game_obj->m_model_info->m_hash);
						}

						//g_hooking->get_original<pack_clone_create>()(mgr, pnObject, netPlayer, msgBuffer);
						//game_obj->m_model_info->m_hash = (int32_t)get_hash(g.protections.freeze_fake_model);
						//return;

						return g_hooking->get_original<pack_clone_create>()(mgr, pnObject, netPlayer, msgBuffer);
						
					}
				}
			}
		}



		return g_hooking->get_original<pack_clone_create>()(mgr, pnObject, netPlayer, msgBuffer);		
	}

	bool hooks::pack_clone_sync(rage::netObjectMgrBase* mgr, rage::netObject* pnObject, rage::netPlayer* netPlayer)
	{
		CObject* game_obj = pnObject->GetGameObject();
		//if (static_cast<eNetObjType>(pnObject->m_object_type) == eNetObjType::NET_OBJ_TYPE_OBJECT)
		//{
		auto plyr = g_player_service->get_by_id(netPlayer->m_player_id);
		if (plyr != nullptr && plyr->frezz_game_sync) [[unlikely]]
		{
			
			int32_t previous_hash = -1;
			if (game_obj != nullptr && game_obj->m_model_info != nullptr)
			{
				//Object Obj = g_pointers->m_gta.m_ptr_to_handle(game_obj);
				if (pnObject->m_object_id == plyr->frezz_game_sync_object_id)
				{
					LOG(INFO) << "Packing clone SYNC crash FOR " << plyr->get_name();

					if (CPed* Cped_target = plyr->get_ped())
					{
						//Vector3 coords = *Cped_target->m_navigation->get_position();
						*game_obj->m_navigation->get_position() = *Cped_target->m_navigation->get_position();
					}

					previous_hash = game_obj->m_model_info->m_hash;
					if (previous_hash != (int32_t)get_hash(g.protections.freeze_model))
					{
						LOG(INFO) << std::format("Packing clone SYNC crash object_id {} previous_hash {:X}", (int)pnObject->m_object_id, previous_hash);
						game_obj->m_model_info->m_hash = get_hash(g.protections.freeze_model);
						LOG(INFO) << std::format("Packing clone SYNC crash object NEW modded hash  {:X}\n",
						    game_obj->m_model_info->m_hash);
					}

					//g_hooking->get_original<pack_clone_sync>()(mgr, pnObject, netPlayer);
					//game_obj->m_model_info->m_hash = (int32_t)get_hash(g.protections.freeze_fake_model);
					//return 1;

					return g_hooking->get_original<pack_clone_sync>()(mgr, pnObject, netPlayer);
				}
			}
		}

		
		if (game_obj != nullptr && g.protections.redirect_cage_object)
		{
			if (pnObject->m_object_id == plyr->target_object_id)
			{
				LOG(INFO) << std::format("===> PACK CLONE SYNC - object id {} => FROM {}", (int)pnObject->m_object_id, plyr->get_name());
				entity::entity_info_netObj(pnObject);

				Vector3 game_obj_coords = *game_obj->m_navigation->get_position();
				Vector3 selfpos         = self::pos;
				float self_poszpz        = selfpos.z;
				float obj_zpz           = game_obj_coords.z;
				//float obj_zpose_final        = 0.0f;
				if (self_poszpz < obj_zpz)
				{
					plyr->redirect_cage_object_z_dist = obj_zpz - self_poszpz;
				}
			
				game_obj_coords.z       = 0;
				selfpos.z               = 0;


				Vector3 min, max, result;
				MISC::GET_MODEL_DIMENSIONS(game_obj->m_model_info->m_hash, &min, &max);
				result         = max - min;
				float y_offset = result.y;

				if (math::distance_between_vectors(game_obj_coords, selfpos) < (y_offset))
				{
					LOG(INFO) << std::format("game_obj_coords position {:.03f} {:.03f} {:.03f}",
					    (float)game_obj_coords.x,
					    (float)game_obj_coords.y,
					    (float)game_obj_coords.z);

					LOG(INFO) << std::format("Redirect distance_between_vectors OK = {:.03f}", math::distance_between_vectors(game_obj_coords, self::pos));






					std::string ms = std::format("model 0x{:X}", game_obj->m_model_info->m_hash);
					auto msg = std::vformat("{} tried to cage me using {}!", std::make_format_args(plyr->get_name(), ms.c_str()));
					msg = std::format("{} {}", g.session.chat_output_prefix, msg);
					chat::send_message(msg);

					//auto msg = std::vformat("NOTIFICATION_CRASH_TYPE_BLOCKED"_T, std::make_format_args(sender->get_name(), mess.c_str()));
					//msg = std::format("{} {}", g.session.chat_output_prefix, msg);
					//chat::send_message(msg);
					g_notification_service.push_error("Protections", std::format("Blocked {} from {}", "cage object", plyr->get_name()));







					if (CPed* Cped_target = plyr->get_ped())
					{
						//Vector3 coords = *Cped_target->m_navigation->get_position();
						//LOG(INFO) << std::format("Redirect clone CREATE target position {:.03f} {:.03f} {:.03f} ",
						//    (float)coords.x,
						//    (float)coords.y,
						//    (float)coords.z);

						//Object Obj = g_pointers->m_gta.m_ptr_to_handle(game_obj);
						////if (entity::take_control_of(self::veh, 0))
						//ENTITY::SET_ENTITY_COORDS(Obj, coords.x, coords.y, coords.z, 0, 0, 0, 0);
						////*game_obj->m_navigation->get_position() = *Cped_target->m_navigation->get_position();
						//coords = *game_obj->m_navigation->get_position();
						//LOG(INFO) << std::format("Redirect clone CREATE target NEW position {:.03f} {:.03f} {:.03f} ",
						//    (float)coords.x,
						//    (float)coords.y,
						//    (float)coords.z);
						Vector3 Cped_target_obj_coords = *Cped_target->m_navigation->get_position();
						Cped_target_obj_coords.z       = Cped_target_obj_coords.z + plyr->redirect_cage_object_z_dist;
						LOG(INFO) << std::format("target position {:.03f} {:.03f} {:.03f} decalage hauteur {:.03f}",
						    (float)Cped_target_obj_coords.x,
						    (float)Cped_target_obj_coords.y,
						    (float)Cped_target_obj_coords.z,
						    plyr->redirect_cage_object_z_dist);

						//*game_obj->m_navigation->get_position() = *Cped_target->m_navigation->get_position();

						//game_obj->m_navigation->m_transformation_matrix.rows[4].x = Cped_target_obj_coords.x;
						//game_obj->m_navigation->m_transformation_matrix.rows[4].y = Cped_target_obj_coords.y;
						//game_obj->m_navigation->m_transformation_matrix.rows[4].z = Cped_target_obj_coords.z;

						//game_obj->m_navigation->m_transformation_matrix.rows[3] =
						//    Cped_target->m_navigation->m_transformation_matrix.rows[3];

						Object Obj = g_pointers->m_gta.m_ptr_to_handle(game_obj);
						plyr->redirect_cage_object_Obj = Obj;
						if (entity::take_control_of(Obj, 500))
						ENTITY::SET_ENTITY_COORDS(Obj,
						    Cped_target_obj_coords.x,
						    Cped_target_obj_coords.y,
						    Cped_target_obj_coords.z,
						    0,
						    0,
						    0,
						    0);

						//game_obj->m_navigation->m_transformation_matrix.rows[3].x =
						//    Cped_target->m_navigation->m_transformation_matrix.rows[3].x;
						//game_obj->m_navigation->m_transformation_matrix.rows[3].y =
						//    Cped_target->m_navigation->m_transformation_matrix.rows[3].y;
						//game_obj->m_navigation->m_transformation_matrix.rows[3].z =
						//    Cped_target->m_navigation->m_transformation_matrix.rows[3].z;

						//game_obj->m_navigation->m_transformation_matrix.rows[3].w =
						//    Cped_target->m_navigation->m_transformation_matrix.rows[3].w;

						/*Memory::set_value({ 0x08, 0x030, 0x050 }, poseX);
							Memory::set_value({ 0x08, 0x030, 0x054 }, poseY);
							Memory::set_value({ 0x08, 0x030, 0x058 }, poseZ);*/

						Vector3 new_game_obj_coords = *game_obj->m_navigation->get_position();

						LOG(INFO) << std::format("game_obj_coords position {:.03f} {:.03f} {:.03f}",
						    (float)new_game_obj_coords.x,
						    (float)new_game_obj_coords.y,
						    (float)new_game_obj_coords.z);

						
						plyr->redirect_cage_object = true;

						//Vector3 coords = *Cped_target->m_navigation->get_position();
						//LOG(INFO) << std::format("Redirect clone CREATE target position {:.03f} {:.03f} {:.03f} ",
						//    (float)coords.x,
						//    (float)coords.y,
						//    (float)coords.z);

						//Object Obj = g_pointers->m_gta.m_ptr_to_handle(game_obj);
						////if (entity::take_control_of(self::veh, 0))
						//ENTITY::SET_ENTITY_COORDS(Obj, coords.x, coords.y, coords.z, 0, 0, 0, 0);
						////*game_obj->m_navigation->get_position() = *Cped_target->m_navigation->get_position();
						//coords = *game_obj->m_navigation->get_position();
						//LOG(INFO) << std::format("Redirect clone CREATE target NEW position {:.03f} {:.03f} {:.03f} ",
						//    (float)coords.x,
						//    (float)coords.y,
						//    (float)coords.z);

						//return g_hooking->get_original<pack_clone_sync>()(mgr, pnObject, netPlayer);						
					}
				}
				else
					LOG(INFO) << std::format("distance between object {:.03f}", math::distance_between_vectors(game_obj_coords, self::pos));
			}
		}

		//}



		return g_hooking->get_original<pack_clone_sync>()(mgr, pnObject, netPlayer);	
	}

	eAckCode hooks::send_clone_sync(CNetworkObjectMgr* mgr, CNetGamePlayer* target_player, rage::netObject* pObject, rage::datBitBuffer* msgBuffer, unsigned __int16* seqNum, bool sendImmediately)
	{
		if (/*g.self.safetypoint &&*/ g.session.block_send_clone_sync_all)
		{
			return eAckCode::ACKCODE_FAIL;
		}
		auto plyr         = g_player_service->get_by_id(target_player->m_player_id);
		auto net_obj      = pObject;
		CObject* game_obj = pObject->GetGameObject();

		if (game_obj != nullptr)
		{
			//if (pObject->m_object_id == plyr->target_object_id)
			//{
			//	LOG(INFO) << std::format("===> SEND CLONE SYNC - object id {} => FROM {}", (int)pObject->m_object_id, plyr->get_name());
			//	entity::entity_info_netObj(pObject);

			//	Vector3 game_obj_coords = *game_obj->m_navigation->get_position();
			//	if (math::distance_between_vectors(game_obj_coords, self::pos) < 5.5f)
			//	{
			//		LOG(INFO) << std::format("game_obj_coords position {:.03f} {:.03f} {:.03f}",
			//		    (float)game_obj_coords.x,
			//		    (float)game_obj_coords.y,
			//		    (float)game_obj_coords.z);

			//		LOG(INFO) << std::format("Redirect distance_between_vectors OK = {:.03f}", math::distance_between_vectors(game_obj_coords, self::pos));

			//		if (CPed* Cped_target = plyr->get_ped())
			//		{
			//			//Vector3 coords = *Cped_target->m_navigation->get_position();
			//			//LOG(INFO) << std::format("Redirect clone CREATE target position {:.03f} {:.03f} {:.03f} ",
			//			//    (float)coords.x,
			//			//    (float)coords.y,
			//			//    (float)coords.z);

			//			//Object Obj = g_pointers->m_gta.m_ptr_to_handle(game_obj);
			//			////if (entity::take_control_of(self::veh, 0))
			//			//ENTITY::SET_ENTITY_COORDS(Obj, coords.x, coords.y, coords.z, 0, 0, 0, 0);
			//			////*game_obj->m_navigation->get_position() = *Cped_target->m_navigation->get_position();
			//			//coords = *game_obj->m_navigation->get_position();
			//			//LOG(INFO) << std::format("Redirect clone CREATE target NEW position {:.03f} {:.03f} {:.03f} ",
			//			//    (float)coords.x,
			//			//    (float)coords.y,
			//			//    (float)coords.z);
			//			Vector3 Cped_target_obj_coords = *Cped_target->m_navigation->get_position();
			//			LOG(INFO) << std::format("target position {:.03f} {:.03f} {:.03f}",
			//			    (float)Cped_target_obj_coords.x,
			//			    (float)Cped_target_obj_coords.y,
			//			    (float)Cped_target_obj_coords.z);

			//			//*game_obj->m_navigation->get_position() = *Cped_target->m_navigation->get_position();

			//			//game_obj->m_navigation->m_transformation_matrix.rows[4].x = Cped_target_obj_coords.x;
			//			//game_obj->m_navigation->m_transformation_matrix.rows[4].y = Cped_target_obj_coords.y;
			//			//game_obj->m_navigation->m_transformation_matrix.rows[4].z = Cped_target_obj_coords.z;

			//			//game_obj->m_navigation->m_transformation_matrix.rows[3] =
			//			//    Cped_target->m_navigation->m_transformation_matrix.rows[3];

			//			Object Obj = g_pointers->m_gta.m_ptr_to_handle(game_obj);
			//			ENTITY::SET_ENTITY_COORDS(Obj,
			//			    Cped_target_obj_coords.x,
			//			    Cped_target_obj_coords.y,
			//			    Cped_target_obj_coords.z,
			//			    0,
			//			    0,
			//			    0,
			//			    0);

			//			//game_obj->m_navigation->m_transformation_matrix.rows[3].x =
			//			//    Cped_target->m_navigation->m_transformation_matrix.rows[3].x;
			//			//game_obj->m_navigation->m_transformation_matrix.rows[3].y =
			//			//    Cped_target->m_navigation->m_transformation_matrix.rows[3].y;
			//			//game_obj->m_navigation->m_transformation_matrix.rows[3].z =
			//			//    Cped_target->m_navigation->m_transformation_matrix.rows[3].z;

			//			//game_obj->m_navigation->m_transformation_matrix.rows[3].w =
			//			//    Cped_target->m_navigation->m_transformation_matrix.rows[3].w;

			//			/*Memory::set_value({ 0x08, 0x030, 0x050 }, poseX);
			//				Memory::set_value({ 0x08, 0x030, 0x054 }, poseY);
			//				Memory::set_value({ 0x08, 0x030, 0x058 }, poseZ);*/

			//			Vector3 new_game_obj_coords = *game_obj->m_navigation->get_position();

			//			LOG(INFO) << std::format("game_obj_coords position {:.03f} {:.03f} {:.03f}",
			//			    (float)new_game_obj_coords.x,
			//			    (float)new_game_obj_coords.y,
			//			    (float)new_game_obj_coords.z);

			//			return eAckCode::ACKCODE_SUCCESS;

			//			//Vector3 coords = *Cped_target->m_navigation->get_position();
			//			//LOG(INFO) << std::format("Redirect clone CREATE target position {:.03f} {:.03f} {:.03f} ",
			//			//    (float)coords.x,
			//			//    (float)coords.y,
			//			//    (float)coords.z);

			//			//Object Obj = g_pointers->m_gta.m_ptr_to_handle(game_obj);
			//			////if (entity::take_control_of(self::veh, 0))
			//			//ENTITY::SET_ENTITY_COORDS(Obj, coords.x, coords.y, coords.z, 0, 0, 0, 0);
			//			////*game_obj->m_navigation->get_position() = *Cped_target->m_navigation->get_position();
			//			//coords = *game_obj->m_navigation->get_position();
			//			//LOG(INFO) << std::format("Redirect clone CREATE target NEW position {:.03f} {:.03f} {:.03f} ",
			//			//    (float)coords.x,
			//			//    (float)coords.y,
			//			//    (float)coords.z);

			//			//return g_hooking->get_original<pack_clone_sync>()(mgr, pnObject, netPlayer);
			//		}
			//	}
			//	else
			//		LOG(INFO) << std::format("distance between object {:.03f}", math::distance_between_vectors(game_obj_coords, self::pos));
			//}
		}
		
		if (plyr != nullptr && plyr->frezz_game_sync) [[unlikely]]
		{
			if ((static_cast<eNetObjType>(pObject->m_object_type) != eNetObjType::NET_OBJ_TYPE_PLAYER)
			    && (static_cast<eNetObjType>(pObject->m_object_type) != eNetObjType::NET_OBJ_TYPE_PED))
			{
				int32_t previous_hash = -1;
				if (game_obj != nullptr && game_obj->m_model_info != nullptr)
				{
					//LOG(INFO) << "frezz_game_sync net_obj";

					//Object Obj = g_pointers->m_gta.m_ptr_to_handle(game_obj);
					if (net_obj->m_object_id == plyr->frezz_game_sync_object_id)
					{
						LOG(INFO) << "Send clone sync crash";
						log_entity_info(pObject);

						LOG(INFO) << "Send clone sync crash FOR " << plyr->get_name() << "  m_object_id " << net_obj->m_object_id;

						if (CPed* Cped_target = plyr->get_ped())
						{
							//Vector3 coords = *Cped_target->m_navigation->get_position();
							*game_obj->m_navigation->get_position() = *Cped_target->m_navigation->get_position();
						}

						previous_hash = game_obj->m_model_info->m_hash;
						if (previous_hash != (int32_t)get_hash(g.protections.freeze_model))
						{
							LOG(INFO) << std::format("Send clone sync crash object_id {} previous_hash {:X}",
							    (int)net_obj->m_object_id,
							    previous_hash);
							game_obj->m_model_info->m_hash = (int32_t)get_hash(g.protections.freeze_model);
							LOG(INFO) << std::format("Send clone sync crash object NEW modded hash  {:X}\n",
							    game_obj->m_model_info->m_hash);
						}

						//eAckCode code = g_hooking->get_original<send_clone_sync>()(mgr, target_player, pObject, msgBuffer, seqNum, sendImmediately);
						//game_obj->m_model_info->m_hash = (int32_t)get_hash(g.protections.freeze_fake_model); //previous_hash
						//LOG(INFO) << std::format("eAckCode {}\n", (int)code);
						//return code;
						return g_hooking->get_original<send_clone_sync>()(mgr, target_player, pObject, msgBuffer, seqNum, sendImmediately);
					}
				}
			}
		}
		else if (plyr != nullptr && plyr->block_send_clone_sync) [[unlikely]]
		{
			//frezze
			//pObject->m_object_type = (int16_t)eNetObjType::NET_OBJ_TYPE_HELI;

			return eAckCode::ACKCODE_FAIL;
		}
		else if (plyr != nullptr) [[unlikely]]
		{
			int32_t previous_hash = -1;			
			if (game_obj != nullptr && game_obj->m_model_info != nullptr)
			{
				previous_hash = game_obj->m_model_info->m_hash;
				if (previous_hash == (int32_t)get_hash(g.protections.freeze_model))
				{
					LOG(INFO) << std::format("Player protections freeze_model {:X}\n", previous_hash);
					game_obj->m_model_info->m_hash = (int32_t)get_hash(g.protections.freeze_fake_model);

					return g_hooking->get_original<send_clone_sync>()(mgr, target_player, pObject, msgBuffer, seqNum, sendImmediately);
				}
			}
		}
		return g_hooking->get_original<send_clone_sync>()(mgr, target_player, pObject, msgBuffer, seqNum, sendImmediately);
	}





	//void hooks::pack_clone_create(CNetworkObjectMgr* mgr, rage::netObject* pNetObject, CNetGamePlayer* pNetGamePlayer, rage::datBitBuffer* pDatBitBuffer)
	//{
	//	player_ptr player = g_player_service->get_by_id(pNetGamePlayer->m_player_id);

	//	if (player && player->crash_packing_clone_create && pNetObject != nullptr && static_cast<eNetObjType>(pNetObject->m_object_type) == eNetObjType::NET_OBJ_TYPE_OBJECT)
	//	{
	//		CObject* pObject = pNetObject->GetGameObject();

	//		int32_t previous_hash = -1;
	//		if (pObject != nullptr && pObject->m_model_info != nullptr)
	//		{
	//			previous_hash                 = pObject->m_model_info->m_hash;
	//			pObject->m_model_info->m_hash = invalid_hashes[selected_hash];
	//			selected_hash++;

	//			if (selected_hash > 66)
	//			{
	//				LOG(VERBOSE) << "passed all 66 in packing";
	//				selected_hash = 66;
	//			}
	//			eAckCode code = g_hooking->get_original<packing_clone_create>()(mgr, pNetObject, pNetGamePlayer, pDatBitBuffer);

	//			pObject->m_model_info->m_hash = previous_hash;
	//			return code;
	//		}
	//	}

	//	return g_hooking->get_original<packing_clone_create>()(mgr, pNetObject, pNetGamePlayer, pDatBitBuffer);
	//}



}
