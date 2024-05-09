#include "backend/looped/looped.hpp"
#include "core/enums.hpp"
#include "gta/enums.hpp"
#include "util/entity.hpp"
#include "util/math.hpp"


#include "base/CObject.hpp"
#include "core/data/task_types.hpp"
#include "util/model_info.hpp"
#include "services/gta_data/gta_data_service.hpp"
#include <entities/CDynamicEntity.hpp>

namespace big
{
	void looped::weapons_delete_gun()
	{
		if (g.weapons.custom_weapon == CustomWeapon::DELETE_GUN && (!g.self.custom_weapon_stop || WEAPON::IS_PED_ARMED(self::ped, 4 | 2)))
		{
			if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)ControllerInputs::INPUT_AIM))
			{
				if (PAD::IS_DISABLED_CONTROL_JUST_RELEASED(0, (int)ControllerInputs::INPUT_ATTACK))
				{
					Entity entity;

					if (entity::raycast(&entity))
					{
						if (ENTITY::IS_ENTITY_A_PED(entity) && PED::IS_PED_A_PLAYER(entity))
						{
							g_notification_service.push_error("CUSTOM_WEAPONS"_T.data(), "BACKEND_LOOPED_WEAPONS_DELETE_GUN_PLAYER"_T.data());
						}
						else
						{
							Vector3 entLoc = ENTITY::GET_ENTITY_COORDS(entity, true);
							double dist    = math::distance_between_vectors(self::pos, entLoc);

							if (dist > 500)
							{
								g_notification_service.push_error("CUSTOM_WEAPONS"_T.data(), "BACKEND_LOOPED_WEAPONS_DELETE_GUN_TOO_FAR"_T.data());
							}
							else
							{
								if (entity::take_control_of(entity))
								{
									entity::delete_entity(entity);
								}
								else
									g_notification_service.push_error("CUSTOM_WEAPONS"_T.data(), "TELEPORT_FAILED_TO_TAKE_CONTROL"_T.data());
							}
						}
					}
					else
					{
						g_notification_service.push_error("CUSTOM_WEAPONS"_T.data(), "BACKEND_LOOPED_WEAPONS_CAGE_GUN_NO_ENTITY_FOUND"_T.data());
					}
				}
			}
		}
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
			return std::format("{}\n ({})", net_id, net_object_type_strs[net_obj->m_object_type]);

		return std::format("{}\n({}, {})",
		    net_id,
		    net_object_type_strs[net_obj->m_object_type],
		    get_model_hash_string(net_obj->GetGameObject()->m_model_info->m_hash));
	}


	void looped::entity_info_gun()
	{
		if (g.weapons.custom_weapon == CustomWeapon::INFO_GUN && (!g.self.custom_weapon_stop || WEAPON::IS_PED_ARMED(self::ped, 4 | 2)))
		{
			if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)ControllerInputs::INPUT_AIM))
			{
				if (PAD::IS_DISABLED_CONTROL_JUST_RELEASED(0, (int)ControllerInputs::INPUT_ATTACK))
				{
					Entity entity;

					if (entity::raycast(&entity))
					{
						if (ENTITY::IS_ENTITY_A_PED(entity) && PED::IS_PED_A_PLAYER(entity))
						{
							g_notification_service.push_error("CUSTOM_WEAPONS"_T.data(), "BACKEND_LOOPED_WEAPONS_DELETE_GUN_PLAYER"_T.data());
						}
						else
						{
							Vector3 entLoc = ENTITY::GET_ENTITY_COORDS(entity, true);
							double dist    = math::distance_between_vectors(self::pos, entLoc);

							if (dist > 500)
							{
								g_notification_service.push_error("CUSTOM_WEAPONS"_T.data(), "BACKEND_LOOPED_WEAPONS_DELETE_GUN_TOO_FAR"_T.data());
							}
							else
							{
								
								LOG(INFO) << "Entity_info_gun";	

								//return reinterpret_cast<CVehicle*>(g_pointers->m_gta.m_handle_to_ptr(personal_vehicle));
								//if (handle_to_ptr(entity))

								auto ptr = (rage::CDynamicEntity*)g_pointers->m_gta.m_handle_to_ptr(entity);
								LOG(INFO) << "ptr ?";	
								if (ptr != nullptr && ptr->m_net_object != nullptr)
								{
									LOG(INFO) << "m_handle_to_ptr CDynamicEntity";
									if (auto netobj = ptr->m_net_object)
									{										
										//LOG(INFO) << "Entity info: " << get_network_id_string(netobj->m_object_id);
										//LOG(INFO) << "m_object_type : " << (int)netobj->m_object_type;
										//LOG(INFO) << "m_object_id : " << (int)netobj->m_object_id;
										//LOG(INFO) << "m_owner_id : " << (int)netobj->m_owner_id;
										//LOG(INFO) << "m_control_id : " << (int)netobj->m_control_id;
										//LOG(INFO) << "m_next_owner_id : " << (int)netobj->m_next_owner_id;
										//LOG(INFO) << "m_is_remote : " << (int)netobj->m_is_remote;
										//LOG(INFO) << "m_wants_to_delete : " << (int)netobj->m_wants_to_delete;
										//LOG(INFO) << "m_object_type : " << (int)netobj->m_should_not_be_delete;
										//Vector3 coords   = *netobj->GetGameObject()->m_navigation->get_position();
										//std::string mess = std::format("position {:.03f} {:.03f} {:.03f} ",
										//    (float)coords.x,
										//    (float)coords.y,
										//    (float)coords.z);
										//LOG(INFO) << mess.c_str();

										std::string mess = "Entity info: \n ";
										//mess += std::format("{}\n", get_network_id_string(netobj->m_object_id));

										auto net_obj = g_pointers->m_gta.m_get_net_object(*g_pointers->m_gta.m_network_object_mgr, netobj->m_object_id, false);
										if (!net_obj)
											mess += std::format("{}\n", netobj->m_object_id);
										else
										{	
											if (auto game_obj = net_obj->GetGameObject(); !game_obj || !game_obj->m_model_info)
												mess += std::format("{}\n {}\n",
												    netobj->m_object_id,
												    net_object_type_strs[net_obj->m_object_type]);
											else
											{
												//mess += std::format("{}\n {}\n  {}\n",
												//    netobj->m_object_id,
												//    net_object_type_strs[net_obj->m_object_type],
												//    get_model_hash_string(net_obj->GetGameObject()->m_model_info->m_hash));

												mess += std::format("{}\n {}\n",
												    netobj->m_object_id,
												    net_object_type_strs[net_obj->m_object_type]);
												//get_model_hash_string(net_obj->GetGameObject()->m_model_info->m_hash));

												uint32_t model = net_obj->GetGameObject()->m_model_info->m_hash;
												auto info = model_info::get_model(model);
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
										}	

										//CObject* Obj = reinterpret_cast<CObject*>(g_pointers->m_gta.m_handle_to_ptr(entity));
										CObject* Obj = (CObject*)g_pointers->m_gta.m_handle_to_ptr(entity);	
										if (Obj != nullptr && Obj->m_model_info != nullptr)
										mess += std::format("Obj m_hash: 0x{:X} \n", Obj->m_model_info->m_hash);			

										CObject* Obj2 = reinterpret_cast<CObject*>(g_pointers->m_gta.m_handle_to_ptr(entity));
										//CObject* Obj = (CObject*)g_pointers->m_gta.m_handle_to_ptr(entity);
										if (Obj2 != nullptr && Obj2->m_model_info != nullptr)
										mess += std::format("Obj2 m_hash: 0x{:X} \n", Obj2->m_model_info->m_hash);		
										
										//mess += std::format("m_object_type :{} {}\t", (int)netobj->m_object_type, net_object_type_strs[netobj->m_object_type]);
										mess += std::format("m_object_type: {} \n", (int)netobj->m_object_type);
										
										mess += std::format("m_object_id: {}\n", (int)netobj->m_object_id);

										if (auto owner_plyr = g_player_service->get_by_id((uint32_t)netobj->m_owner_id))
										mess += std::format("m_owner_id: {} {}\n", (int)netobj->m_owner_id, owner_plyr->get_name());
										else
										mess += std::format("m_owner_id: {}\n", (int)netobj->m_owner_id);

										if (auto control_plyr = g_player_service->get_by_id((uint32_t)netobj->m_control_id))
										mess += std::format("m_control_id: {} {}\n", (int)netobj->m_control_id, control_plyr->get_name());
										else
										mess += std::format("m_control_id: {}\n", (int)netobj->m_control_id);
	
										if (auto next_owner_plyr = g_player_service->get_by_id((uint32_t)netobj->m_next_owner_id))
										mess += std::format("m_next_owner_id: {} {}\n", (int)netobj->m_next_owner_id, next_owner_plyr->get_name());
										else
										mess += std::format("m_next_owner_id: {}\n", (int)netobj->m_next_owner_id);

										mess += std::format("m_is_remote: {}\n", (int)netobj->m_is_remote);
										mess += std::format("m_wants_to_delete: {}\n", (int)netobj->m_wants_to_delete);
										mess += std::format("m_should_not_be_delete: {}\n", (int)netobj->m_should_not_be_delete);

										Vector3 coords   = *netobj->GetGameObject()->m_navigation->get_position();
										mess += std::format("position {:.03f} {:.03f} {:.03f}\n",
										    (float)coords.x,
										    (float)coords.y,
										    (float)coords.z);


										LOG(INFO) << mess.c_str();

										g_notification_service.push_success("Entity info gun", mess);
									}
								}
								else
									LOG(INFO) << "!m_handle_to_ptr";	
							}
						}
					}
					else
					{
						g_notification_service.push_error("CUSTOM_WEAPONS"_T.data(), "BACKEND_LOOPED_WEAPONS_CAGE_GUN_NO_ENTITY_FOUND"_T.data());
					}
				}
			}
		}
	}
}
