#include "hooking/hooking.hpp"
#include "services/players/player_service.hpp"
#include "util/notify.hpp"

#include "base/CObject.hpp"
#include "core/data/task_types.hpp"
#include "util/model_info.hpp"
#include "services/gta_data/gta_data_service.hpp"
#include <entities/CDynamicEntity.hpp>

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
				LOG(INFO) << "m_model_type " << (int)info->m_model_type;
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
	}

	eAckCode hooks::received_clone_sync(CNetworkObjectMgr* mgr, CNetGamePlayer* src, CNetGamePlayer* dst, eNetObjType object_type, uint16_t object_id, rage::datBitBuffer* buffer, uint16_t unk, uint32_t timestamp)
	{
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

		if (plyr && plyr->recev_log) [[unlikely]]
		{
			LOG(INFO) << std::format("received_clone_sync plyr.recev_log {} == {} g.m_syncing_object_type  == id {}",
				plyr->get_name(), (int)object_type, (int)object_id);


			std::string mess = "Entity info: \n ";
			//mess += std::format("{}\n", get_network_id_string(netobj->m_object_id));
			auto net_obj = g_pointers->m_gta.m_get_net_object(*g_pointers->m_gta.m_network_object_mgr, object_id, false);
			if (!net_obj)
				mess += std::format("{}\n", (int)object_id);
			else
			{
				if (auto game_obj = net_obj->GetGameObject(); !game_obj || !game_obj->m_model_info)
					mess += std::format("{}\n {}\n", (int)object_id, net_object_type_strs[net_obj->m_object_type]);
				else
				{
					//mess += std::format("{}\n {}\n  {}\n",
					//    netobj->m_object_id,
					//    net_object_type_strs[net_obj->m_object_type],
					//    get_model_hash_string(net_obj->GetGameObject()->m_model_info->m_hash));
					mess += std::format("{}\n {}\n", (int)object_id, net_object_type_strs[net_obj->m_object_type]);
					//get_model_hash_string(net_obj->GetGameObject()->m_model_info->m_hash));
					uint32_t model = net_obj->GetGameObject()->m_model_info->m_hash;
					auto info      = model_info::get_model(model);
					if (!info)
						mess += std::format("0x{:X}\n", model);
					else
					{
						mess += std::format("m_model_type {}\n", (int)info->m_model_type);
						LOG(INFO) << "m_model_type " << (int)info->m_model_type;
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
				}


				//				//CObject* Obj = reinterpret_cast<CObject*>(g_pointers->m_gta.m_handle_to_ptr(entity));
				//CObject* Obj = (CObject*)g_pointers->m_gta.m_handle_to_ptr(entity);
				//if (Obj != nullptr && Obj->m_model_info != nullptr)
				//	mess += std::format("Obj m_hash: 0x{:X} \n", Obj->m_model_info->m_hash);

				//CObject* Obj2 = reinterpret_cast<CObject*>(g_pointers->m_gta.m_handle_to_ptr(entity));
				////CObject* Obj = (CObject*)g_pointers->m_gta.m_handle_to_ptr(entity);
				//if (Obj2 != nullptr && Obj2->m_model_info != nullptr)
				//	mess += std::format("Obj2 m_hash: 0x{:X} \n", Obj2->m_model_info->m_hash);

				//mess += std::format("m_object_type :{} {}\t", (int)netobj->m_object_type, net_object_type_strs[netobj->m_object_type]);
				mess += std::format("m_object_type: {} \n", (int)net_obj->m_object_type);

				mess += std::format("m_object_id: {}\n", (int)net_obj->m_object_id);

				if (auto owner_plyr = g_player_service->get_by_id((uint32_t)net_obj->m_owner_id))
					mess += std::format("m_owner_id: {} {}\n", (int)net_obj->m_owner_id, owner_plyr->get_name());
				else
					mess += std::format("m_owner_id: {}\n", (int)net_obj->m_owner_id);

				if (auto control_plyr = g_player_service->get_by_id((uint32_t)net_obj->m_control_id))
					mess += std::format("m_control_id: {} {}\n", (int)net_obj->m_control_id, control_plyr->get_name());
				else
					mess += std::format("m_control_id: {}\n", (int)net_obj->m_control_id);

				if (auto next_owner_plyr = g_player_service->get_by_id((uint32_t)net_obj->m_next_owner_id))
					mess += std::format("m_next_owner_id: {} {}\n", (int)net_obj->m_next_owner_id, next_owner_plyr->get_name());
				else
					mess += std::format("m_next_owner_id: {}\n", (int)net_obj->m_next_owner_id);

				mess += std::format("m_is_remote: {}\n", (int)net_obj->m_is_remote);
				mess += std::format("m_wants_to_delete: {}\n", (int)net_obj->m_wants_to_delete);
				mess += std::format("m_should_not_be_delete: {}\n", (int)net_obj->m_should_not_be_delete);

				Vector3 coords = *net_obj->GetGameObject()->m_navigation->get_position();
				mess +=
				    std::format("position {:.03f} {:.03f} {:.03f}\n", (float)coords.x, (float)coords.y, (float)coords.z);


				//LOG(INFO) << mess.c_str();
			}

			LOG(INFO) << mess.c_str();
		}

		if (plyr && plyr->block_clone_sync) [[unlikely]]
			return eAckCode::ACKCODE_FAIL;


			
		

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
	eAckCode hooks::send_clone_sync(CNetworkObjectMgr* mgr, CNetGamePlayer* target_player, rage::netObject* pObject, rage::datBitBuffer* msgBuffer, unsigned __int16* seqNum, bool sendImmediately)
	{
		if (/*g.self.safetypoint &&*/ g.session.block_send_clone_sync_all)
		{
			return eAckCode::ACKCODE_FAIL;
		}

		auto net_obj = pObject;
		CObject* game_obj = net_obj->GetGameObject();

		auto plyr = g_player_service->get_by_id(target_player->m_player_id);		
		if (plyr != nullptr && plyr->frezz_game_sync) [[unlikely]]
		{	

			//std::string mess = "Entity info: \n ";
			////CObject* game_obj = net_obj->GetGameObject();
			//auto net_obj_p = g_pointers->m_gta.m_get_net_object(*g_pointers->m_gta.m_network_object_mgr, pObject->m_object_id, false);
			//if (!net_obj_p)
			//	mess += std::format("id {}\n type {}\n", pObject->m_object_id, net_object_type_strs[pObject->m_object_type]);
			//else
			//{
			//	//mess += std::format("{}\n {}\n  {}\n",
			//	//    netobj->m_object_id,
			//	//    net_object_type_strs[net_obj->m_object_type],
			//	//    get_model_hash_string(net_obj->GetGameObject()->m_model_info->m_hash));
			//	mess += std::format("id {}\n type {}\n", pObject->m_object_id, net_object_type_strs[pObject->m_object_type]);
			//	//get_model_hash_string(net_obj->GetGameObject()->m_model_info->m_hash));
			//	uint32_t model = pObject->GetGameObject()->m_model_info->m_hash;
			//	auto info      = model_info::get_model(model);
			//	if (!info)
			//		mess += std::format("0x{:X}\n", model);
			//	else
			//	{
			//		mess += std::format("m_model_type {}\n", (int)info->m_model_type);
			//		LOG(INFO) << "m_model_type " << (int)info->m_model_type;
			//		const char* model_str = nullptr;
			//		if (info->m_model_type == eModelType::Vehicle)
			//		{
			//			for (auto& [name, data] : g_gta_data_service->vehicles())
			//			{
			//				if (data.m_hash == model)
			//				{
			//					model_str = name.data();
			//				}
			//			}
			//		}
			//		else if (info->m_model_type == eModelType::Ped || info->m_model_type == eModelType::OnlineOnlyPed)
			//		{
			//			for (auto& [name, data] : g_gta_data_service->peds())
			//			{
			//				if (data.m_hash == model)
			//				{
			//					model_str = name.data();
			//				}
			//			}
			//		}
			//		if (!model_str)
			//			mess += std::format("0x{:X}\n", model);
			//		else
			//		{
			//			mess += std::format("{} (0x{:X})\n", model_str, model);
			//		}
			//	}
			//	LOG(INFO) << mess;
			//}

			log_entity_info(pObject);
				
			int32_t previous_hash = -1;
			if (game_obj != nullptr && game_obj->m_model_info != nullptr)
			{
				LOG(INFO) << "frezz_game_sync net_obj";

				//Object Obj = g_pointers->m_gta.m_ptr_to_handle(game_obj);
				if (net_obj->m_object_id == plyr->frezz_game_sync_object_id)
				{
					LOG(INFO) << "Send clone sync crash FOR " << plyr->get_name() << "  m_object_id " << net_obj->m_object_id;

					if (CPed* Cped_target = plyr->get_ped())
					{
						//Vector3 coords = *Cped_target->m_navigation->get_position();
						*game_obj->m_navigation->get_position() = *Cped_target->m_navigation->get_position();
					}

					previous_hash = game_obj->m_model_info->m_hash;
					if (previous_hash != (int32_t)get_hash(g.protections.freeze_model))
					{
						LOG(INFO) << std::format("frezz_game_sync_object_id {} previous_hash {:X}\n", (int)net_obj->m_object_id, previous_hash);
						game_obj->m_model_info->m_hash = (int32_t)get_hash(g.protections.freeze_model);
						LOG(INFO) << std::format("frezz_game_sync_object_id NEW modded hash  {:X}\n", game_obj->m_model_info->m_hash);
					}

					eAckCode code = g_hooking->get_original<send_clone_sync>()(mgr, target_player, pObject, msgBuffer, seqNum, sendImmediately);
					game_obj->m_model_info->m_hash = (int32_t)get_hash(g.protections.freeze_fake_model);//previous_hash

					LOG(INFO) << std::format("eAckCode {}\n", (int)code);
					return code;
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


	void hooks::pack_clone_create(rage::netObjectMgrBase* mgr, rage::netObject* pnObject, rage::netPlayer* netPlayer, rage::datBitBuffer* msgBuffer)
	{
		//if (static_cast<eNetObjType>(pnObject->m_object_type) == eNetObjType::NET_OBJ_TYPE_OBJECT)
		//{
		auto plyr = g_player_service->get_by_id(netPlayer->m_player_id);
		if (plyr != nullptr && plyr->frezz_game_sync) [[unlikely]]
		{
			CObject* game_obj     = pnObject->GetGameObject();
			int32_t previous_hash = -1;
			if (game_obj != nullptr && game_obj->m_model_info != nullptr)
			{
				//Object Obj = g_pointers->m_gta.m_ptr_to_handle(game_obj);
				if (pnObject->m_object_id == plyr->frezz_game_sync_object_id)
				{
					LOG(INFO) << "Packing clone create crash FOR " << plyr->get_name();

					if (CPed* Cped_target = plyr->get_ped())
					{
						//Vector3 coords = *Cped_target->m_navigation->get_position();
						*game_obj->m_navigation->get_position() = *Cped_target->m_navigation->get_position();
					}

					previous_hash = game_obj->m_model_info->m_hash;
					if (previous_hash != (int32_t)get_hash(g.protections.freeze_model))
					{
						LOG(INFO) << std::format("frezz_game_sync_object_id {} previous_hash {:X}\n", (int)pnObject->m_object_id, previous_hash);
						game_obj->m_model_info->m_hash = get_hash(g.protections.freeze_model);
						LOG(INFO) << std::format("frezz_game_sync_object_id NEW modded hash  {:X}\n",
						    game_obj->m_model_info->m_hash);
					}

					g_hooking->get_original<pack_clone_create>()(mgr, pnObject, netPlayer, msgBuffer);
					game_obj->m_model_info->m_hash = (int32_t)get_hash(g.protections.freeze_fake_model);
					return;
				}
			}
		}
		//}

		return g_hooking->get_original<pack_clone_create>()(mgr, pnObject, netPlayer, msgBuffer);		
	}

	bool hooks::pack_clone_sync(rage::netObjectMgrBase* mgr, rage::netObject* pnObject, rage::netPlayer* netPlayer)
	{
		//if (static_cast<eNetObjType>(pnObject->m_object_type) == eNetObjType::NET_OBJ_TYPE_OBJECT)
		//{
		auto plyr = g_player_service->get_by_id(netPlayer->m_player_id);
		if (plyr != nullptr && plyr->frezz_game_sync) [[unlikely]]
		{
			CObject* game_obj     = pnObject->GetGameObject();
			int32_t previous_hash = -1;
			if (game_obj != nullptr && game_obj->m_model_info != nullptr)
			{
				//Object Obj = g_pointers->m_gta.m_ptr_to_handle(game_obj);
				if (pnObject->m_object_id == plyr->frezz_game_sync_object_id)
				{
					LOG(INFO) << "Packing clone sync crash FOR " << plyr->get_name();

					if (CPed* Cped_target = plyr->get_ped())
					{
						//Vector3 coords = *Cped_target->m_navigation->get_position();
						*game_obj->m_navigation->get_position() = *Cped_target->m_navigation->get_position();
					}

					previous_hash = game_obj->m_model_info->m_hash;
					if (previous_hash != (int32_t)get_hash(g.protections.freeze_model))
					{
						LOG(INFO) << std::format("frezz_game_sync_object_id {} previous_hash {:X}\n", (int)pnObject->m_object_id, previous_hash);
						game_obj->m_model_info->m_hash = get_hash(g.protections.freeze_model);
						LOG(INFO) << std::format("frezz_game_sync_object_id NEW modded hash  {:X}\n",
						    game_obj->m_model_info->m_hash);
					}

					g_hooking->get_original<pack_clone_sync>()(mgr, pnObject, netPlayer);
					game_obj->m_model_info->m_hash = (int32_t)get_hash(g.protections.freeze_fake_model);
					return 1;
				}
			}
		}
		//}



		return g_hooking->get_original<pack_clone_sync>()(mgr, pnObject, netPlayer);	
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
