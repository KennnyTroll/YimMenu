#include "hooking/hooking.hpp"
#include "services/players/player_service.hpp"
#include "util/notify.hpp"

namespace big
{
	eAckCode hooks::received_clone_sync(CNetworkObjectMgr* mgr, CNetGamePlayer* src, CNetGamePlayer* dst, eNetObjType object_type, uint16_t object_id, rage::datBitBuffer* buffer, uint16_t unk, uint32_t timestamp)
	{
		if (object_type < eNetObjType::NET_OBJ_TYPE_AUTOMOBILE || object_type > eNetObjType::NET_OBJ_TYPE_TRAIN)
		{
			notify::crash_blocked(src, "out of bounds object type");
			return eAckCode::ACKCODE_FAIL;
		}

		if (auto net_obj = g_pointers->m_gta.m_get_net_object(mgr, object_id, true); net_obj && net_obj->m_object_type != (int16_t)object_type)
		{
			notify::crash_blocked(src, "incorrect object type");
			return eAckCode::ACKCODE_FAIL;
		}

		auto plyr = g_player_service->get_by_id(src->m_player_id);

		if (plyr && plyr->block_clone_sync)
			return eAckCode::ACKCODE_FAIL;

		g.m_syncing_player      = src;
		g.m_syncing_object_type = object_type;

		g.debug.fuzzer.thread_id = GetCurrentThreadId();
		if (g.debug.fuzzer.enabled_object_types[(int)object_type])
			g.debug.fuzzer.active = true;
		g.debug.fuzzer.syncing_object_id = object_id;
		auto ret = g_hooking->get_original<received_clone_sync>()(mgr, src, dst, object_type, object_id, buffer, unk, timestamp);
		g.debug.fuzzer.active = false;

		return ret;
	}

	eAckCode hooks::send_clone_sync(CNetworkObjectMgr* mgr, CNetGamePlayer* player, rage::netObject* pObject, rage::datBitBuffer* msgBuffer, unsigned __int16* seqNum, bool sendImmediately)
	{
		if (/*g.self.safetypoint &&*/ g.session.block_send_clone_sync_all)
		{
			return eAckCode::ACKCODE_FAIL;
		}

		auto plyr = g_player_service->get_by_id(player->m_player_id);

		if (plyr && plyr->block_send_clone_sync)
		{
			return eAckCode::ACKCODE_FAIL;
		}

		return g_hooking->get_original<send_clone_sync>()(mgr, player, pObject, msgBuffer, seqNum, sendImmediately);

	//	char* v10;                    // rsi
	//	void** v11;                   // rdi
	//	int Cursor;                   // eax
	//	unsigned int v13;             // ebp
	//	char v14;                     // bl
	//	int v15;                      // ecx
	//	int v16;                      // edx
	//	rage::netPlayer* PlayerOwner; // rax
	//	int v18;                      // ecx
	//	int v19;                      // ecx
	//	int v20;                      // ecx
	//	int v21;                      // ecx
	//	int v22;                      // ebx
	//	const char* v23;              // rax
	//	__int64 v24;                  // rdx
	//	__int64 v25;                  // [rsp+30h] [rbp-38h]
	//	v10
	//		= this + 968 * player->m_ActivePlayerIndex;
	//		if (!*(v10 + 8))
	//			*(v10 + 8) = rage::netObject::GetPlayerOwner(pObject);
	//		v11    = (v10 + 88);
	//		Cursor = rage::RsonWriter::GetCursor(msgBuffer);
	//		v13    = rage::s_NetworkTimeThisFrameStart;
	//		v14    = 0;
	//		v15    = Cursor + 17;
	//		if (!*(v10 + 18) || *(v10 + 18) != rage::s_NetworkTimeThisFrameStart || v10[78] == 7)
	//		{
	//			v14         = 1;
	//			v15         = Cursor + 52;
	//			*(v10 + 18) = rage::s_NetworkTimeThisFrameStart;
	//		}
	//		v16 = 31;
	//		if (rage::cloneSyncMsg::AutoId__.m_Id <= 0xFF)
	//			v16 = 23;
	//		if ((v10[116] & 1) != 0 || v15 + v16 + *(v10 + 26) + 61 > *(v10 + 25) || *(v10 + 8) != rage::netObject::GetPlayerOwner(pObject))
	//		{
	//			rage::netObjectMgrMessageHandler::SendConnectionSyncMessage(this, player);
	//			PlayerOwner = rage::netObject::GetPlayerOwner(pObject);
	//			*(v10 + 18) = v13;
	//			v14         = 1;
	//			*(v10 + 8)  = PlayerOwner;
	//		}
	//		if (sendImmediately)
	//			this->m_SendConnectionSyncImmediately |= 1 << player->m_ActivePlayerIndex;
	//		if (!v14)
	//			goto LABEL_26;
	//		if ((v10[116] & 1) == 0)
	//		{
	//			v18 = *(v10 + 26);
	//			if (v18 + 32 <= *(v10 + 25))
	//			{
	//				if ((v10[116] & 2) == 0)
	//					rage::datBitBuffer::WriteUnsigned(*v11, v13, 32, v18 + *(v10 + 24));
	//				rage::datBitBuffer::BumpReadWriteCursor((v10 + 88), 32);
	//			}
	//		}
	//		rage::cloneSyncMsg::UpdateNumObjectsCount((v10 + 64));
	//		if ((v10[116] & 1) == 0)
	//		{
	//			v19 = *(v10 + 26);
	//			if (v19 + 3 <= *(v10 + 25))
	//			{
	//				if ((v10[116] & 2) == 0)
	//					rage::datBitBuffer::WriteUnsigned(*v11, 0, 3, v19 + *(v10 + 24));
	//				rage::datBitBuffer::BumpReadWriteCursor((v10 + 88), 3);
	//			}
	//		LABEL_26:
	//			if ((v10[116] & 1) == 0)
	//			{
	//				v20
	//					= *(v10 + 26);
	//					if (v20 + 4 <= *(v10 + 25))
	//					{
	//						if ((v10[116] & 2) == 0)
	//							rage::datBitBuffer::WriteUnsigned(*v11, pObject->m_ObjectType, 4, v20 + *(v10 + 24));
	//						rage::datBitBuffer::BumpReadWriteCursor((v10 + 88), 4);
	//					}
	//					if ((v10[116] & 1) == 0)
	//					{
	//						v21 = *(v10 + 26);
	//						if (v21 + 13 <= *(v10 + 25))
	//						{
	//							if ((v10[116] & 2) == 0)
	//								rage::datBitBuffer::WriteUnsigned(*v11, pObject->m_ObjectID, 13, v21 + *(v10 + 24));
	//							rage::datBitBuffer::BumpReadWriteCursor((v10 + 88), 13);
	//						}
	//					}
	//			}
	//		}
	//		v22 = rage::RsonWriter::GetCursor(msgBuffer);
	//		v23 = rage::datBitBuffer::GetReadWriteBits(msgBuffer);
	//		rage::datBitBuffer::WriteBits((v10 + 88), v23, v22, 0);
	//		++v10[78];
	//		*seqNum = this->m_Sequence[player->m_ActivePlayerIndex];
	//		if (this->m_Log)
	//		{
	//			v25 = (pObject->vfptr->GetLogName)(pObject);
	//			audVehicleAudioEntity::TriggerEngineFailedToStart(this->m_Log, 0LL, *seqNum);
	//			this->m_Log->vfptr->WriteDataValue(this->m_Log, "Timestamp", "%d", v13, "SENDING_CLONE_SYNC", "%s", v25);
	//			LOBYTE(v24) = player->m_PhysicalPlayerIndex;
	//			(pObject->vfptr->GetUpdateLevel)(pObject, v24);
	//			this->m_Log->vfptr->WriteDataValue(this->m_Log, "Update Level", "%s", &Comp);
	//			if (sendImmediately)
	//				this->m_Log->vfptr->WriteDataValue(this->m_Log, "Send Immediately", "TRUE");
	//		}
	//		(this->m_ObjectManager->vfptr->OnSendCloneSync)(this->m_ObjectManager, pObject, v13);

	}
}
