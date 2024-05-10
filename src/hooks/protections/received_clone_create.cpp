#include "hooking/hooking.hpp"
#include "services/players/player_service.hpp"
#include "util/notify.hpp"

#include "base/CObject.hpp"
#include "core/data/task_types.hpp"
#include "util/model_info.hpp"
#include "services/gta_data/gta_data_service.hpp"

#include "util/sync_trees.hpp"

#include "netsync/netSyncDataNode.hpp"
#include "netsync/netSyncTree.hpp"
#include "netsync/nodes/automobile/CAutomobileCreationNode.hpp"
#include "netsync/nodes/door/CDoorCreationDataNode.hpp"
#include "netsync/nodes/door/CDoorMovementDataNode.hpp"
#include "netsync/nodes/door/CDoorScriptGameStateDataNode.hpp"
#include "netsync/nodes/door/CDoorScriptInfoDataNode.hpp"
#include "netsync/nodes/dynamic_entity/CDynamicEntityGameStateDataNode.hpp"
#include "netsync/nodes/entity/CEntityScriptGameStateDataNode.hpp"
#include "netsync/nodes/entity/CEntityScriptInfoDataNode.hpp"
#include "netsync/nodes/object/CObjectCreationDataNode.hpp"
#include "netsync/nodes/ped/CPedAIDataNode.hpp"
#include "netsync/nodes/ped/CPedAttachDataNode.hpp"
#include "netsync/nodes/ped/CPedComponentReservationDataNode.hpp"
#include "netsync/nodes/ped/CPedCreationDataNode.hpp"
#include "netsync/nodes/ped/CPedGameStateDataNode.hpp"
#include "netsync/nodes/ped/CPedHealthDataNode.hpp"
#include "netsync/nodes/ped/CPedInventoryDataNode.hpp"
#include "netsync/nodes/ped/CPedMovementDataNode.hpp"
#include "netsync/nodes/ped/CPedOrientationDataNode.hpp"
#include "netsync/nodes/ped/CPedScriptCreationDataNode.hpp"
#include "netsync/nodes/ped/CPedTaskSequenceDataNode.hpp"
#include "netsync/nodes/ped/CPedTaskSpecificDataNode.hpp"
#include "netsync/nodes/ped/CPedTaskTreeDataNode.hpp"
#include "netsync/nodes/physical/CPhysicalAngVelocityDataNode.hpp"
#include "netsync/nodes/physical/CPhysicalAttachDataNode.hpp"
#include "netsync/nodes/physical/CPhysicalGameStateDataNode.hpp"
#include "netsync/nodes/physical/CPhysicalHealthDataNode.hpp"
#include "netsync/nodes/physical/CPhysicalMigrationDataNode.hpp"
#include "netsync/nodes/physical/CPhysicalScriptGameStateDataNode.hpp"
#include "netsync/nodes/physical/CPhysicalScriptMigrationDataNode.hpp"
#include "netsync/nodes/physical/CPhysicalVelocityDataNode.hpp"
#include "netsync/nodes/pickup/CPickupCreationDataNode.hpp"
#include "netsync/nodes/pickup_placement/CPickupPlacementCreationDataNode.hpp"
#include "netsync/nodes/player/CPlayerAmbientModelStreamingNode.hpp"
#include "netsync/nodes/player/CPlayerAppearanceDataNode.hpp"
#include "netsync/nodes/player/CPlayerCameraDataNode.hpp"
#include "netsync/nodes/player/CPlayerCreationDataNode.hpp"
#include "netsync/nodes/player/CPlayerExtendedGameStateNode.hpp"
#include "netsync/nodes/player/CPlayerGameStateDataNode.hpp"
#include "netsync/nodes/player/CPlayerGamerDataNode.hpp"
#include "netsync/nodes/player/CPlayerSectorPosNode.hpp"
#include "netsync/nodes/player/CPlayerWantedAndLOSDataNode.hpp"
#include "netsync/nodes/proximity_migrateable/CGlobalFlagsDataNode.hpp"
#include "netsync/nodes/proximity_migrateable/CMigrationDataNode.hpp"
#include "netsync/nodes/proximity_migrateable/CSectorDataNode.hpp"
#include "netsync/nodes/proximity_migrateable/CSectorPositionDataNode.hpp"
#include "netsync/nodes/train/CTrainGameStateDataNode.hpp"
#include "netsync/nodes/vehicle/CVehicleComponentReservationDataNode.hpp"
#include "netsync/nodes/vehicle/CVehicleControlDataNode.hpp"
#include "netsync/nodes/vehicle/CVehicleCreationDataNode.hpp"
#include "netsync/nodes/vehicle/CVehicleGadgetDataNode.hpp"
#include "netsync/nodes/vehicle/CVehicleProximityMigrationDataNode.hpp"
#include "netsync/nodes/vehicle/CVehicleTaskDataNode.hpp"

namespace big
{
	template<typename T>
	inline bool is_valid_ptr(T ptr)
	{
		uint64_t address = (uint64_t)ptr;
		if (address < 0x5000)
			return false;
		if ((address & 0xFFFFFFFF) == 0xFFFFFFFF)
			return false;
		if ((address & 0xFFFFFFFF) <= 0xFF)
			return false;

		if (*(uint8_t*)((uint64_t)&address + 6) != 0 || *(uint8_t*)((uint64_t)&address + 7) != 0)
			return false;

		return true;
	}

	std::string get_vtable_name(uint64_t table)
	{
		uint64_t object_locator = *(uint64_t*)(table - 8);
		if (object_locator)
		{
			uint32_t offset = *(uint32_t*)(object_locator + 0xC);
			if (offset)
			{
				const char* string = (const char*)(game_base_address + offset + 0x10);
				if (string)
				{
					std::string str(string);
					if (str.find(".?A") == 0)
					{
						str = str.substr(4, str.length());
					}

					if (str.find("NodeCommonDataOperations") != std::string::npos)
					{
						str = str.substr(2, str.length());

						std::size_t at = str.find_first_of('@');
						if (at != std::string::npos)
						{
							std::string base_class = str.substr(at + 1);
							if (!base_class.empty())
							{
								at = base_class.find_first_of("@@");
								if (at != std::string::npos)
								{
									str = base_class.substr(0, at);
									if (!str.empty())
									{
										if (str.length() > 2)
										{
											if (str[0] == 'V' && str[1] == 'C')
											{
												str = "NodeCommonDataOperations<" + str.substr(1) + ">";
											}
										}
									}
								}
							}
						}
					}
					else if (str.find("AutoIdDesc__") != std::string::npos)
					{
						std::size_t at = str.find("AutoIdDesc__");
						if (at == 0)
						{
							at = str.find_first_of('@');
							if (at != std::string::npos)
							{
								str = str.substr(13);
								str = str.substr(0, str.find('@'));
							}
						}
						else
						{
							at = str.find("::AutoIdDesc__");
							if (at != std::string::npos)
							{
								str = str.substr(0, at);
							}
						}
					}
					else
					{
						std::size_t at = str.find_first_of('@');
						if (at > 0 && at < 0xffff)
						{
							str = str.substr(0, at);
						}
					}

					return str;
				}

				return string;
			}
		}

		return "";
	}

	//struct bit_buffer_sync_instance
	//{
	//	uint64_t m_VTable;
	//	uint8_t m_type; // 1 = reader, 2 = writer, 3 = calculator, 4 = logger
	//	char _0x0009[0x7];
	//	uint64_t m_sub_type; // 0 = reader, 2 = writer, 3 = calculator, 4 = logger
	//	rage::datBitBuffer* m_buffer;
	//};

	//bit_buffer_sync_instance rage::datBitBuffer::create_reader()
	//{
	//	bit_buffer_sync_instance instance;
	//	instance.m_VTable   = (uint64_t)global::vars::g_sync_data_reader_vtable;
	//	instance.m_type     = 1;
	//	instance.m_sub_type = 0;
	//	instance.m_buffer   = this;
	//	return instance;
	//}

	bool get_node_creation(rage::netSyncNodeBase* node, eNetObjType object_type, rage::datBitBuffer* buffer, CNetGamePlayer* sender, uint32_t* hash/*, uint64_t* data*/)
	{
		if (is_valid_ptr(node))
		{
			//LOG(INFO) << "is_valid_ptr : ";	
			////rage::netSyncNodeBase* current = node;
			//while (is_valid_ptr(*(uint64_t*)((uint64_t)node)))
			//{
			//	LOG(INFO) << "is_valid_ptr : 2";	

			if (node->IsParentNode())
			{
				for (auto child = node->m_first_child; child; child = child->m_next_sibling)
				{
					//if (get_node_creation(child, sender, object))
					if (get_node_creation(child, object_type, buffer, sender, hash/*, data*/))
						return true;
				}


				//uint64_t address       = (uint64_t)node + 0x50;
				//uint32_t m_child_count = *reinterpret_cast<uint32_t*>(address);
				//if (m_child_count > 0)
				//{
				//	rage::netSyncNodeBase* children = node->m_first_child;
				//	if (is_valid_ptr(children))
				//	{
				//		if (get_node_creation(children, buffer, hash, data))
				//		{
				//			return true;
				//		}
				//	}
				//}
			}
			else if (node->IsDataNode())
			{
				const auto addr = (uintptr_t)node;
				//auto sender_plyr    = g_player_service->get_by_id(sender->m_player_id);
				const auto& node_id = sync_node_finder::find(object_type, addr);

				if ((((CProjectBaseSyncDataNode*)node)->flags & 1) == 0)
					return false;

				//*data = (uint64_t)((CProjectBaseSyncDataNode*)node)->m_common_data_operations.m_buffer;	
				
				switch (node_id)
				{
					case sync_node_id("CVehicleCreationDataNode"):
					{
					    const auto creation_node = (CVehicleCreationDataNode*)(node);
						std::string mess = std::format("CVehicleCreationDataNode Vehicle model = 0x{:X}) | pop_type {} | random_seed {} | vehicle_status {} | max_health {} | creation_token {} | car_budget {} | needs_to_be_hotwired {} | tires_dont_burst {} => FROM {}",
							creation_node->m_model,
						    creation_node->m_pop_type,
						    creation_node->m_random_seed,
						    creation_node->m_vehicle_status,
						    creation_node->m_max_health,
						    creation_node->m_creation_token,
						    creation_node->m_car_budget,
						    creation_node->m_needs_to_be_hotwired,
						    creation_node->m_tires_dont_burst,
							sender->get_name());
						LOG(INFO) << mess.c_str();

					    *hash = creation_node->m_model;					    				   
						return true;
						//return buffer->ReadDword(hash, 0x20);		

						break;
					}
					case sync_node_id("CObjectCreationDataNode"):
					{
					    const auto creation_node = (CObjectCreationDataNode*)(node);

						std::string mess = std::format("ObjectCreationDataNode object model = 0x{:X} | position {:.03f} {:.03f} {:.03f} | m_script_grab_radius {:.03f} | m_created_by {} | m_frag_group_index {} | m_ownership_token {} | m_no_reassign {} | m_player_wants_control {} | m_has_init_physics {} | m_script_grabbed_from_world {} | m_has_frag_group {} | m_is_broken {} | m_has_exploded {} | m_keep_registered {} | unk_0169 {} | unk_016A {} | unk_016B {} => FROM {}",
				    		creation_node->m_model,
					        creation_node->m_object_position.x,
					        creation_node->m_object_position.y,
					        creation_node->m_object_position.z,
					        creation_node->m_script_grab_radius,
					        creation_node->m_created_by,
					        creation_node->m_frag_group_index,
					        creation_node->m_ownership_token,
					        creation_node->m_no_reassign,
					        creation_node->m_player_wants_control,
					        creation_node->m_has_init_physics,
					        creation_node->m_script_grabbed_from_world,
					        creation_node->m_has_frag_group,
					        creation_node->m_is_broken,
					        creation_node->m_has_exploded,
					        creation_node->m_keep_registered,
					        creation_node->unk_0169,
					        creation_node->unk_016A,
					        creation_node->unk_016B,
					        sender->get_name());
					    LOG(INFO) << mess.c_str();
					   
						*hash = creation_node->m_model;

						//uint32_t unk = 0;
						//if (buffer->ReadDword(&unk, 5))
						//{
						//	if ((unk & 0xFFFFFFFD) != 0)
						//	{
						//		return buffer->ReadDword(hash, 0x20);
						//	}
						//}

						return true;

						//return false;

						break;
					}
					case sync_node_id("CPedCreationDataNode"):
					{
					    const auto creation_node = (CPedCreationDataNode*)(node);
					    std::string mes = std::format("PedCreationDataNode ped model = 0x{:X} | pop_type {} | random_seed {} | max_health {} | in_vehicle {} | vehicle_id {} | vehicle_seat {} | m_has_prop {} | m_prop_model 0x{:X}  | m_is_standing {} | m_is_respawn_object_id {} | m_is_respawn_flagged_for_removal {} | m_has_attr_damage_to_player {} | m_attribute_damage_to_player {} | m_voice_hash 0x{:X} => FROM {}",
					        creation_node->m_model,
					        creation_node->m_pop_type,
					        creation_node->m_random_seed,
					        creation_node->m_max_health,
					        creation_node->m_in_vehicle,
					        creation_node->m_vehicle_id,
					        creation_node->m_vehicle_seat,
					        creation_node->m_has_prop,
					        creation_node->m_prop_model,
					        creation_node->m_is_standing,
					        creation_node->m_is_respawn_object_id,
					        creation_node->m_is_respawn_flagged_for_removal,
					        creation_node->m_has_attr_damage_to_player,
					        creation_node->m_attribute_damage_to_player,
					        creation_node->m_voice_hash,
					        sender->get_name());
					    LOG(INFO) << mes.c_str();

						*hash = creation_node->m_model;

						//bool temp = false;
						//if (buffer->ReadBool(&temp))
						//{
						//	if (buffer->ReadBool(&temp))
						//	{
						//		if (buffer->ReadDword(hash, 4))
						//		{
						//			return buffer->ReadDword(hash, 0x20);
						//		}
						//	}
						//}

						return true;

						break;
					}

					case sync_node_id("CPickupCreationDataNode"):
				    {
					    const auto creation_node = (CPickupCreationDataNode*)(node);
					    std::string mess = std::format("PickupCreationDataNode pickup hash 0x{:X} | has_placement {} | amount {} | model 0x{:X} | life_time {} | num_weapon_components {} | tint_index {} | player_gift {} | unk_015D {} | unk_0164 {} | unk_0168 {} => FROM {}",
				    	    creation_node->m_pickup_hash,
					        creation_node->m_has_placement,
					        creation_node->m_amount,
					        creation_node->m_custom_model,
					        creation_node->m_life_time,
					        creation_node->m_num_weapon_components,
					        creation_node->m_tint_index,
					        creation_node->m_player_gift,
					        creation_node->unk_015D,
					        creation_node->unk_0164,
					        creation_node->unk_0168,
					        sender->get_name());
					    LOG(INFO) << mess.c_str();

						*hash = creation_node->m_pickup_hash;
					    
						return true;

					    break;
				    }
		
				}

			//	LOG(INFO) << "IsDataNode";
			//	CProjectBaseSyncDataNode* syncDataNode = (CProjectBaseSyncDataNode*)node;
			//	if (syncDataNode->m_common_data_operations.m_buffer != nullptr)
			//	{
			//		LOG(INFO) << "syncDataNode";
			//		std::string name = get_vtable_name(*(uint64_t*)((uint64_t)syncDataNode));
			//		if (!name.empty())
			//		{
			//			LOG(INFO) << "name";
			//			if (strstr(name.c_str(), "CreationDataNode") && !strstr(name.c_str(), "CAutomobileCreation"))
			//			{
			//				LOG(INFO) << "CAutomobileCreation";
			//				*data = (uint64_t)syncDataNode->m_common_data_operations.m_buffer;
			//				switch (rage::joaat(name.c_str()))
			//				{
			//				case rage::joaat("CDoorCreationDataNode"):
			//				case rage::joaat("CPlayerCreationDataNode"):
			//				case rage::joaat("CVehicleCreationDataNode"): return buffer->ReadDword(hash, 0x20);
			//				case rage::joaat("CObjectCreationDataNode"):
			//				{
			//					uint32_t unk = 0;
			//					if (buffer->ReadDword(&unk, 5))
			//					{
			//						if ((unk & 0xFFFFFFFD) != 0)
			//						{
			//							return buffer->ReadDword(hash, 0x20);
			//						}
			//					}
			//					return false;
			//				}
			//				case rage::joaat("CPedCreationDataNode"):
			//				{
			//					bool temp = false;
			//					if (buffer->ReadBool(&temp))
			//					{
			//						if (buffer->ReadBool(&temp))
			//						{
			//							if (buffer->ReadDword(hash, 4))
			//							{
			//								return buffer->ReadDword(hash, 0x20);
			//							}
			//						}
			//					}
			//					return false;
			//				}
			//				case rage::joaat("CPickupCreationDataNode"):
			//				{
			//					//rage::bit_buffer_sync_instance sync_reader = buffer->create_reader();
			//					//bool unk = false;
			//					//if (buffer->ReadBool(&unk))
			//					//{
			//					//	bool success         = false;
			//					//	uint32_t pickup_hash = 0;
			//					//	uint32_t hash2       = 0;
			//					//	uint8_t unk_data[0x200];
			//					//	if (unk)
			//					//	{
			//					//		success = caller::call<bool>(global::vars::g_read_pickup_from_bit_buffer, unk_data, &sync_reader);
			//					//		pickup_hash = *(uint32_t*)&unk_data[0x8];
			//					//	}
			//					//	else
			//					//	{
			//					//		success = buffer->ReadDword(&pickup_hash, 0x20);
			//					//	}
			//					//	if (success)
			//					//	{
			//					//		unk = *(uint32_t*)(&unk_data[84]) > 0;
			//					//		if (buffer->ReadBool(&unk))
			//					//		{
			//					//			if (unk)
			//					//			{
			//					//				success = buffer->ReadDword(&hash2, 0x20);
			//					//			}
			//					//			if (success)
			//					//			{
			//					//				*hash = *(uint32_t*)(&unk_data[88]);
			//					//				unk = *(uint32_t*)(&unk_data[88]) != 0;
			//					//				if (buffer->ReadBool(&unk))
			//					//				{
			//					//					if (unk)
			//					//					{
			//					//						return buffer->ReadDword(hash, 0x20);
			//					//					}
			//					//				}
			//					//			}
			//					//		}
			//					//	}
			//					//}
			//					//return false;
			//				}
			//				}
			//			}
			//		}
			//	}
			//}

			//node = node->m_next_sibling;
			}
		}
		return false;
	}

	//bool get_node__creation(rage::netSyncNodeBase* node, rage::datBitBuffer* buffer, CNetGamePlayer* sender, uint32_t* hash, uint64_t* data)
	//{
	//	if (is_valid_ptr(node))
	//	{
	//		rage::netSyncNodeBase* current = node;
	//		if (is_valid_ptr(current) && is_valid_ptr(*(uint64_t*)((uint64_t)current)))
	//		{
	//			LOG(INFO) << "while";
	//			if (current->IsDataNode())
	//			{
	//				LOG(INFO) << "IsDataNode";
	//				//    (uint64_t)((CProjectBaseSyncDataNode*)node)->m_common_data_operations.m_buffer
	//				if ((uint64_t)((CProjectBaseSyncDataNode*)current)->m_common_data_operations.m_buffer)
	//				{
	//					LOG(INFO) << "m_buffer";
	//					if ((uint64_t)((CProjectBaseSyncDataNode*)current)->m_common_data_operations.m_buffer->m_data)
	//					{
	//						LOG(INFO) << "m_buffer->m_data";
	//						std::string name = get_vtable_name(*(uint64_t*)((uint64_t)current));
	//						if (!name.empty())
	//						{
	//							LOG(INFO) << name.empty();
	//							if (strstr(name.c_str(), "CreationDataNode") && !strstr(name.c_str(), "CAutomobileCreation"))
	//							{
	//								LOG(INFO) << name.c_str();
	//								*data = (uint64_t)((CProjectBaseSyncDataNode*)current)->m_common_data_operations.m_buffer->m_data;
	//								switch (MISC::GET_HASH_KEY(name.c_str()))
	//								{
	//									LOG(INFO) << "GET_HASH_KEY";
	//								case rage::joaat("CDoorCreationDataNode") :
	//								case rage::joaat("CPlayerCreationDataNode"):
	//								case rage::joaat("CVehicleCreationDataNode"): 
	//								{
	//									const auto creation_node = (CVehicleCreationDataNode*)(current);
	//									std::string mess = std::format("CVehicleCreationDataNode Vehicle model = 0x{:X}) | pop_type {} | random_seed {} | vehicle_status {} | max_health {} | creation_token {} | car_budget {} | needs_to_be_hotwired {} | tires_dont_burst {} => FROM {}",
	//									    creation_node->m_model,
	//									    creation_node->m_pop_type,
	//									    creation_node->m_random_seed,
	//									    creation_node->m_vehicle_status,
	//									    creation_node->m_max_health,
	//									    creation_node->m_creation_token,
	//									    creation_node->m_car_budget,
	//									    creation_node->m_needs_to_be_hotwired,
	//									    creation_node->m_tires_dont_burst,
	//									    sender->get_name());
	//									LOG(INFO) << mess.c_str();
	//									*hash = creation_node->m_model;	
	//									//return buffer->read_uint(hash, 0x20);
	//									return true;
	//								}
	//									
	//								case rage::joaat("CObjectCreationDataNode"):
	//								{
	//									//uint32_t unk = 0;
	//									//if (buffer->read_uint(&unk, 5))
	//									//{
	//									//	if ((unk & 0xFFFFFFFD) != 0)
	//									//	{
	//									//		return buffer->read_uint(hash, 0x20);
	//									//	}
	//									//}
	//									//return false;
	//									const auto creation_node = (CObjectCreationDataNode*)(current);
	//									std::string mess = std::format("ObjectCreationDataNode object model = 0x{:X} | position {:.03f} {:.03f} {:.03f} | m_script_grab_radius {:.03f} | m_created_by {} | m_frag_group_index {} | m_ownership_token {} | m_no_reassign {} | m_player_wants_control {} | m_has_init_physics {} | m_script_grabbed_from_world {} | m_has_frag_group {} | m_is_broken {} | m_has_exploded {} | m_keep_registered {} | unk_0169 {} | unk_016A {} | unk_016B {} => FROM {}",
	//									    creation_node->m_model,
	//									    creation_node->m_object_position.x,
	//									    creation_node->m_object_position.y,
	//									    creation_node->m_object_position.z,
	//									    creation_node->m_script_grab_radius,
	//									    creation_node->m_created_by,
	//									    creation_node->m_frag_group_index,
	//									    creation_node->m_ownership_token,
	//									    creation_node->m_no_reassign,
	//									    creation_node->m_player_wants_control,
	//									    creation_node->m_has_init_physics,
	//									    creation_node->m_script_grabbed_from_world,
	//									    creation_node->m_has_frag_group,
	//									    creation_node->m_is_broken,
	//									    creation_node->m_has_exploded,
	//									    creation_node->m_keep_registered,
	//									    creation_node->unk_0169,
	//									    creation_node->unk_016A,
	//									    creation_node->unk_016B,
	//									    sender->get_name());
	//									LOG(INFO) << mess.c_str();
	//									*hash = creation_node->m_model;
	//									return true;
	//								}
	//								case rage::joaat("CPedCreationDataNode"):
	//								{
	//									//bool temp = false;
	//									//if (buffer->read_bool(&temp))
	//									//{
	//									//	if (buffer->read_bool(&temp))
	//									//	{
	//									//		if (buffer->read_uint(hash, 4))
	//									//		{
	//									//			return buffer->read_uint(hash, 0x20);
	//									//		}
	//									//	}
	//									//}
	//									//return false;
	//									const auto creation_node = (CPedCreationDataNode*)(current);
	//									std::string mes = std::format("PedCreationDataNode ped model = 0x{:X} | pop_type {} | random_seed {} | max_health {} | in_vehicle {} | vehicle_id {} | vehicle_seat {} | m_has_prop {} | m_prop_model 0x{:X}  | m_is_standing {} | m_is_respawn_object_id {} | m_is_respawn_flagged_for_removal {} | m_has_attr_damage_to_player {} | m_attribute_damage_to_player {} | m_voice_hash 0x{:X} => FROM {}",
	//									    creation_node->m_model,
	//									    creation_node->m_pop_type,
	//									    creation_node->m_random_seed,
	//									    creation_node->m_max_health,
	//									    creation_node->m_in_vehicle,
	//									    creation_node->m_vehicle_id,
	//									    creation_node->m_vehicle_seat,
	//									    creation_node->m_has_prop,
	//									    creation_node->m_prop_model,
	//									    creation_node->m_is_standing,
	//									    creation_node->m_is_respawn_object_id,
	//									    creation_node->m_is_respawn_flagged_for_removal,
	//									    creation_node->m_has_attr_damage_to_player,
	//									    creation_node->m_attribute_damage_to_player,
	//									    creation_node->m_voice_hash,
	//									    sender->get_name());
	//									LOG(INFO) << mes.c_str();
	//									*hash = creation_node->m_model;
	//									return true;
	//								}
	//								case rage::joaat("CPickupCreationDataNode"):
	//								{
	//									//rage::bit_buffer_sync_instance sync_reader = buffer->create_reader();
	//									//bool unk = false;
	//									//if (buffer->read_bool(&unk))
	//									//{
	//									//	bool success         = false;
	//									//	uint32_t pickup_hash = 0;
	//									//	uint32_t hash2       = 0;
	//									//	uint8_t unk_data[0x200];
	//									//	if (unk)
	//									//	{
	//									//		success = caller::call<bool>(global::vars::g_read_pickup_from_bit_buffer, unk_data, &sync_reader);
	//									//		pickup_hash = *(uint32_t*)&unk_data[0x8];
	//									//	}
	//									//	else
	//									//	{
	//									//		success = buffer->read_uint(&pickup_hash, 0x20);
	//									//	}
	//									//	if (success)
	//									//	{
	//									//		unk = *(uint32_t*)(&unk_data[84]) > 0;
	//									//		if (buffer->read_bool(&unk))
	//									//		{
	//									//			if (unk)
	//									//			{
	//									//				success = buffer->read_uint(&hash2, 0x20);
	//									//			}
	//									//			if (success)
	//									//			{
	//									//				*hash = *(uint32_t*)(&unk_data[88]);
	//									//				unk = *(uint32_t*)(&unk_data[88]) != 0;
	//									//				if (buffer->read_bool(&unk))
	//									//				{
	//									//					if (unk)
	//									//					{
	//									//						return buffer->read_uint(hash, 0x20);
	//									//					}
	//									//				}
	//									//			}
	//									//		}
	//									//	}
	//									//}
	//									//return false;
	//									const auto creation_node = (CPickupCreationDataNode*)(node);
	//									std::string mess = std::format("PickupCreationDataNode pickup hash 0x{:X} | has_placement {} | amount {} | model 0x{:X} | life_time {} | num_weapon_components {} | tint_index {} | player_gift {} | unk_015D {} | unk_0164 {} | unk_0168 {} => FROM {}",
	//									    creation_node->m_pickup_hash,
	//									    creation_node->m_has_placement,
	//									    creation_node->m_amount,
	//									    creation_node->m_custom_model,
	//									    creation_node->m_life_time,
	//									    creation_node->m_num_weapon_components,
	//									    creation_node->m_tint_index,
	//									    creation_node->m_player_gift,
	//									    creation_node->unk_015D,
	//									    creation_node->unk_0164,
	//									    creation_node->unk_0168,
	//									    sender->get_name());
	//									LOG(INFO) << mess.c_str();
	//									*hash = creation_node->m_pickup_hash;
	//									return true;
	//								}
	//								}
	//							}
	//						}
	//					}
	//				}
	//			}
	//			if (current->IsParentNode())
	//			{
	//				LOG(INFO) << "IsParentNode";
	//				//   if (current->m_child_count > 0)
	//				//   {
	//				//rage::network::data_node* children = current->m_first_child;
	//				//if (is_valid_ptr(children))
	//				//{
	//				//	if (get_node_creation(children, buffer, hash, data))
	//				//	{
	//				//		return true;
	//				//	}
	//				//}
	//				//   }
	//				for (auto child = node->m_first_child; child; child = child->m_next_sibling)
	//				{
	//					//if (get_node_creation(child, sender, object))
	//					if (get_node__creation(child, buffer, sender, hash , data))
	//						return true;
	//				}
	//			}
	//			//current = current->m_next_sibling;
	//		}
	//	}
	//	return false;
	//}


	inline std::string get_task_type_string(int type)
	{
		std::string buffer = "";

		for (auto& value : task_type_names)
		{
			if (value.first == type)
			{
				buffer += value.second;
				buffer += '|';
			}

			if (value.first > type)
				break; // a minor optimization
		}

		if (buffer.length() > 1)
			buffer.pop_back();
		else
			buffer = "Unknown";

		buffer += " (" + std::to_string(type) + ")";

		return buffer;
	}

	inline std::string get_model_hash_string(uint32_t model)
	{
		auto info = model_info::get_model(model);

		if (!info)
			return std::format("0x{:X}", model);

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
			return std::format("0x{:X}", model);

		return std::format("{} (0x{:X})", model_str, model);
	}

	inline std::string get_network_id_string(int16_t net_id)
	{
		auto net_obj = g_pointers->m_gta.m_get_net_object(*g_pointers->m_gta.m_network_object_mgr, net_id, false);

		if (!net_obj)
			return std::format("{}", net_id);

		if (auto game_obj = net_obj->GetGameObject(); !game_obj || !game_obj->m_model_info)
			return std::format("{} ({})", net_id, net_object_type_strs[net_obj->m_object_type]);

		return std::format("{} ({}, {})",
		    net_id,
		    net_object_type_strs[net_obj->m_object_type],
		    get_model_hash_string(net_obj->GetGameObject()->m_model_info->m_hash));
	}

	inline rage::netObject* get_netObject_from_id(int16_t net_id)
	{
		rage::netObject* net_obj = g_pointers->m_gta.m_get_net_object(*g_pointers->m_gta.m_network_object_mgr, net_id, false);
		if (!net_obj)
		return NULL;

		return net_obj;
	}	

	inline CObject* get_CObject_from_id(int16_t net_id)
	{
		auto net_obj = g_pointers->m_gta.m_get_net_object(*g_pointers->m_gta.m_network_object_mgr, net_id, false);
		if (!net_obj)
		return NULL;

		if (auto game_obj = net_obj->GetGameObject(); !game_obj || !game_obj->m_model_info)
		return NULL;
		else
		return game_obj;
	}

	inline CVehicle* get_CVehicle_from_id(int16_t net_id)
	{
		auto net_obj = g_pointers->m_gta.m_get_net_object(*g_pointers->m_gta.m_network_object_mgr, net_id, false);
		//auto net_obj_2 = g_pointers->m_gta.m_get_net_object(*g_pointers->m_gta.m_network_object_mgr, net_id, true);
		if (!net_obj)
			return NULL;

		auto game_veh = (CVehicle*)net_obj->GetGameObject();
		if (!game_veh)
			return NULL;
		
		return game_veh;
	}



	void hooks::received_clone_create(CNetworkObjectMgr* mgr, CNetGamePlayer* sender, CNetGamePlayer* toPlayer, eNetObjType object_type, uint16_t object_id, uint16_t object_flag, rage::datBitBuffer* buffer, int32_t timestamp)
	{
		if (object_type < eNetObjType::NET_OBJ_TYPE_AUTOMOBILE || object_type > eNetObjType::NET_OBJ_TYPE_TRAIN) [[unlikely]]
		{
			notify::crash_blocked(sender, "out of bounds object type");
			return;
		}

		auto sender_plyr = g_player_service->get_by_id(sender->m_player_id);

		uint32_t model_hash = 0;
		if (sender_plyr && sender_plyr->cad_log) [[unlikely]]
		{
			LOG(INFO) << "received_clone_create sender_plyr " << sender_plyr->get_name();

			LOG(INFO) << "id " << (int)object_id << "  flag " << (int)object_flag << "  type " << (int)object_type << "  " << net_object_type_strs[(int)object_type];


			LOG(INFO) << "string info : " << get_network_id_string(object_id);

			rage::netSyncTree* tree = g_pointers->m_gta.m_get_sync_tree_for_type(mgr, (uint16_t)object_type);

			//uint64_t creation_data  = 0;
			if (tree != nullptr && get_node_creation(tree->m_last_sync_node->m_parent, object_type, buffer, sender, &model_hash /*, &creation_data*/))
			{
				LOG(INFO) << "get_tree_node_creation info :";
				LOG(INFO) << std::format("model_hash : 0x{:X}", model_hash);
				//LOG(INFO) << "m_object_id : " << creation_data;

				//if (!STREAMING::IS_MODEL_VALID(model_hash))
				//{
				//	std::string mess = std::format("MODEL HAS INVALID MODEL HASH : {}", model_hash);
				//
				//	LOG(INFO) << mess;
				//	notify::crash_blocked(sender, mess.c_str());
				//	return;
				//}
			}

			//uint32_t model__hash   = 0;
			//uint64_t creation__data = 0;
			//if (tree != nullptr && get_node__creation(tree->m_last_sync_node->m_parent, buffer, sender, &model__hash, &creation__data))
			//{
			//	LOG(INFO) << "get_node__creation info 2 :";
			//	LOG(INFO) << std::format("model__hash 2 : 0x{:X}", model__hash);
			//}
		}

		if (sender_plyr && sender_plyr->recev_log) [[unlikely]]
		{
			LOG(INFO) << std::format("received_clone_create plyr.recev_log {} == {} g.m_syncing_object_type  == id {}",
			    sender_plyr->get_name(),
			    (int)object_type,
			    (int)object_id);

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

				////mess += std::format("m_object_type :{} {}\t", (int)netobj->m_object_type, net_object_type_strs[netobj->m_object_type]);
				//mess += std::format("m_object_type: {} \n", (int)netobj->m_object_type);

				//mess += std::format("m_object_id: {}\n", (int)netobj->m_object_id);

				//if (auto owner_plyr = g_player_service->get_by_id((uint32_t)netobj->m_owner_id))
				//	mess += std::format("m_owner_id: {} {}\n", (int)netobj->m_owner_id, owner_plyr->get_name());
				//else
				//	mess += std::format("m_owner_id: {}\n", (int)netobj->m_owner_id);

				//if (auto control_plyr = g_player_service->get_by_id((uint32_t)netobj->m_control_id))
				//	mess += std::format("m_control_id: {} {}\n", (int)netobj->m_control_id, control_plyr->get_name());
				//else
				//	mess += std::format("m_control_id: {}\n", (int)netobj->m_control_id);

				//if (auto next_owner_plyr = g_player_service->get_by_id((uint32_t)netobj->m_next_owner_id))
				//	mess += std::format("m_next_owner_id: {} {}\n", (int)netobj->m_next_owner_id, next_owner_plyr->get_name());
				//else
				//	mess += std::format("m_next_owner_id: {}\n", (int)netobj->m_next_owner_id);

				//mess += std::format("m_is_remote: {}\n", (int)netobj->m_is_remote);
				//mess += std::format("m_wants_to_delete: {}\n", (int)netobj->m_wants_to_delete);
				//mess += std::format("m_should_not_be_delete: {}\n", (int)netobj->m_should_not_be_delete);

				//Vector3 coords = *netobj->GetGameObject()->m_navigation->get_position();
				//mess +=
				//    std::format("position {:.03f} {:.03f} {:.03f}\n", (float)coords.x, (float)coords.y, (float)coords.z);


				//LOG(INFO) << mess.c_str();









			}	

			LOG(INFO) << mess.c_str();


		}

		if (sender_plyr && sender_plyr->block_clone_create) [[unlikely]]
			return;

		g.m_syncing_player      = sender;
		g.m_syncing_object_type = object_type;
		g.m_syncing_model_hash  = model_hash;

		g.debug.fuzzer.thread_id = GetCurrentThreadId();
		if (g.debug.fuzzer.enabled_object_types[(int)object_type]) [[unlikely]]
			g.debug.fuzzer.active = true;
		g.debug.fuzzer.syncing_object_id = object_id;
		g_hooking->get_original<hooks::received_clone_create>()(mgr, sender, toPlayer, object_type, object_id, object_flag, buffer, timestamp);
		g.debug.fuzzer.active = false;
	}

}