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

		if (plyr && plyr->block_clone_sync) [[unlikely]]
			return eAckCode::ACKCODE_FAIL;

		if (plyr && plyr->recev_log) [[unlikely]]
		{
			LOG(INFO) << std::format("received_clone_sync plyr.recev_log {} != {} g.m_syncing_object_type", src->get_name(), (int)object_type);
		}
			
		

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

		auto plyr = g_player_service->get_by_id(target_player->m_player_id);		
		if (plyr != nullptr && plyr->frezz_game_sync) [[unlikely]]
		{

			auto net_obj = pObject;

			LOG(INFO) << "frezz_game_sync net_obj";

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





			//auto net_obj = pObject;
			CObject* game_obj = net_obj->GetGameObject();
			if (game_obj != nullptr)
			{
				Object Obj = g_pointers->m_gta.m_ptr_to_handle(game_obj);
				if (Obj == (Object)plyr->frezz_game_sync_object_id)
				{
					LOG(INFO) << std::format("frezz_game_sync_object_id {} founded {:X}\n",
					    (int)Obj,
					    game_obj->m_model_info->m_hash);

					game_obj->m_model_info->m_hash = get_hash(g.protections.freeze_model);
					
					LOG(INFO) << std::format("frezz_game_sync_object_id modded h {:X}\n",
					    game_obj->m_model_info->m_hash);

					plyr->frezz_game_sync = false;
					LOG(INFO) << "frezz_game_sync net_obj  false";
					return g_hooking->get_original<send_clone_sync>()(mgr, target_player, net_obj, msgBuffer, seqNum, sendImmediately);					
				}
			}			

			//return g_hooking->get_original<send_clone_sync>()(mgr, target_player, pObject, msgBuffer, seqNum, sendImmediately);
		}
		else if (plyr != nullptr && plyr->block_send_clone_sync) [[unlikely]]
		{
			//frezze
			//pObject->m_object_type = (int16_t)eNetObjType::NET_OBJ_TYPE_HELI;

			return eAckCode::ACKCODE_FAIL;
		}


		return g_hooking->get_original<send_clone_sync>()(mgr, target_player, pObject, msgBuffer, seqNum, sendImmediately);

	}
}
