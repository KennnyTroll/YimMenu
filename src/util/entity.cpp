#include "entity.hpp"

#include "model_info.hpp"
#include "util/vehicle.hpp"

#include "util/entity.hpp"
#include "fiber_pool.hpp"

#include "base/CObject.hpp"
#include "services/gta_data/gta_data_service.hpp"

namespace big::entity
{
	std::uint32_t get_hash(const std::string& str)
	{
		rage::joaat_t hash = 0;
		if (str.substr(0, 2) == "0x")
			std::stringstream(str.substr(2)) >> std::hex >> hash;
		else
			hash = rage::joaat(str.c_str());
		return hash;
	}

	void cage_ped(Ped ped)
	{
		Hash hash = "prop_gold_cont_01"_J;

		Vector3 location = ENTITY::GET_ENTITY_COORDS(ped, true);
		OBJECT::CREATE_OBJECT(hash, location.x, location.y, location.z - 1.f, true, false, false);
	}

	void clean_ped(Ped ped)
	{
		PED::CLEAR_PED_BLOOD_DAMAGE(ped);
		PED::CLEAR_PED_WETNESS(ped);
		PED::CLEAR_PED_ENV_DIRT(ped);
		PED::RESET_PED_VISIBLE_DAMAGE(ped);

		// https://forum.cfx.re/t/information-needed-to-clear-visible-player-damage-scars-etc/283216
		// https://docs.fivem.net/natives/?_0x397C38AA7B4A5F83
		for (int i = 0; i <= 5; ++i)
			PED::CLEAR_PED_DAMAGE_DECAL_BY_ZONE(ped, i, "ALL");
	}

	void delete_entity(Entity& ent, bool force)
	{
		if (!ENTITY::DOES_ENTITY_EXIST(ent))
		{
			ent = NULL;
			return;
		}
		if (!force && !take_control_of(ent))
		{
			LOG(VERBOSE) << "Failed to take control of entity before deleting";
			return;
		}

		if (ENTITY::IS_ENTITY_A_VEHICLE(ent))
		{
			for (auto obj : pools::get_all_props())
			{
				auto object = g_pointers->m_gta.m_ptr_to_handle(obj);
				if (!object)
					break;

				if (!ENTITY::IS_ENTITY_ATTACHED_TO_ENTITY(ent, object))
					continue;

				ENTITY::DELETE_ENTITY(&object);
			}

			for (auto veh : pools::get_all_vehicles())
			{
				auto vehicle = g_pointers->m_gta.m_ptr_to_handle(veh);
				if (!vehicle)
					break;

				if (ent == vehicle || !ENTITY::IS_ENTITY_ATTACHED_TO_ENTITY(ent, vehicle))
					continue;

				ENTITY::DELETE_ENTITY(&vehicle);
			}
		}

		ENTITY::DETACH_ENTITY(ent, 1, 1);
		ENTITY::SET_ENTITY_COORDS_NO_OFFSET(ent, 7000.f, 7000.f, 15.f, 0, 0, 0);
		if (!ENTITY::IS_ENTITY_A_MISSION_ENTITY(ent))
		{
			ENTITY::SET_ENTITY_AS_MISSION_ENTITY(ent, true, true);
		}
		ENTITY::DELETE_ENTITY(&ent);
	}

	bool raycast(Entity* ent)
	{
		BOOL hit;
		Vector3 endCoords;
		Vector3 surfaceNormal;

		Vector3 camCoords = CAM::GET_GAMEPLAY_CAM_COORD();
		Vector3 rot       = CAM::GET_GAMEPLAY_CAM_ROT(2);
		Vector3 dir       = math::rotation_to_direction(rot);
		Vector3 farCoords;

		farCoords.x = camCoords.x + dir.x * 1000;
		farCoords.y = camCoords.y + dir.y * 1000;
		farCoords.z = camCoords.z + dir.z * 1000;

		int ray = SHAPETEST::START_EXPENSIVE_SYNCHRONOUS_SHAPE_TEST_LOS_PROBE(camCoords.x,
		    camCoords.y,
		    camCoords.z,
		    farCoords.x,
		    farCoords.y,
		    farCoords.z,
		    -1,
		    0,
		    7);
		SHAPETEST::GET_SHAPE_TEST_RESULT(ray, &hit, &endCoords, &surfaceNormal, ent);

		return (bool)hit;
	}

	bool raycast(Vector3* endcoor)
	{
		Entity ent;
		BOOL hit;
		Vector3 surfaceNormal;

		Vector3 camCoords = CAM::GET_GAMEPLAY_CAM_COORD();
		Vector3 dir       = math::rotation_to_direction(CAM::GET_GAMEPLAY_CAM_ROT(2));
		Vector3 farCoords;

		farCoords.x = camCoords.x + dir.x * 1000;
		farCoords.y = camCoords.y + dir.y * 1000;
		farCoords.z = camCoords.z + dir.z * 1000;

		int ray = SHAPETEST::START_EXPENSIVE_SYNCHRONOUS_SHAPE_TEST_LOS_PROBE(camCoords.x,
		    camCoords.y,
		    camCoords.z,
		    farCoords.x,
		    farCoords.y,
		    farCoords.z,
		    -1,
		    0,
		    7);
		SHAPETEST::GET_SHAPE_TEST_RESULT(ray, &hit, endcoor, &surfaceNormal, &ent);

		return (bool)hit;
	}

	bool network_has_control_of_entity(rage::netObject* net_object)
	{
		return !net_object || !net_object->m_next_owner_id && (net_object->m_control_id == -1);
	}

	bool take_control_of(Entity ent, int timeout)
	{
		if (!*g_pointers->m_gta.m_is_session_started)
			return true;

		auto hnd = g_pointers->m_gta.m_handle_to_ptr(ent);

		if (!hnd || !hnd->m_net_object)
			return false;

		if (network_has_control_of_entity(hnd->m_net_object))
			return true;

		for (int i = 0; i < timeout; i++)
		{
			g_pointers->m_gta.m_request_control(hnd->m_net_object);

			if (network_has_control_of_entity(hnd->m_net_object))
				return true;

			if (timeout != 0)
				script::get_current()->yield();
		}

		return false;
	}

	std::vector<Entity> get_entities(bool vehicles, bool peds, bool props, bool include_self_veh)
	{
		std::vector<Entity> target_entities;

		if (vehicles)
		{
			for (auto vehicle : pools::get_all_vehicles())
			{
				if (!vehicle || (!include_self_veh && vehicle == gta_util::get_local_vehicle()))
					continue;

				target_entities.push_back(g_pointers->m_gta.m_ptr_to_handle(vehicle));
			}
		}

		if (peds)
		{
			for (auto ped : pools::get_all_peds())
			{
				if (!ped || ped == g_local_player)
					continue;

				target_entities.push_back(g_pointers->m_gta.m_ptr_to_handle(ped));
			}
		}

		if (props)
		{
			for (auto prop : pools::get_all_props())
			{
				if (!prop)
					continue;

				target_entities.push_back(g_pointers->m_gta.m_ptr_to_handle(prop));
			}
		}
		return target_entities;
	}

	bool load_ground_at_3dcoord(Vector3& location)
	{
		constexpr float max_ground_check = 1000.f;
		constexpr int max_attempts       = 300;
		float ground_z                   = location.z;
		int current_attempts             = 0;
		bool found_ground;
		float height;

		do
		{
			found_ground = MISC::GET_GROUND_Z_FOR_3D_COORD(location.x, location.y, max_ground_check, &ground_z, FALSE, FALSE);
			STREAMING::REQUEST_COLLISION_AT_COORD(location.x, location.y, location.z);

			if (current_attempts % 10 == 0)
			{
				location.z += 25.f;
			}

			++current_attempts;

			script::get_current()->yield();
		} while (!found_ground && current_attempts < max_attempts);

		if (!found_ground)
		{
			return false;
		}

		if (WATER::GET_WATER_HEIGHT(location.x, location.y, location.z, &height))
		{
			location.z = height;
		}
		else
		{
			location.z = ground_z + 1.f;
		}

		return true;
	}

	bool request_model(rage::joaat_t hash)
	{
		if (STREAMING::HAS_MODEL_LOADED(hash))
		{
			return true;
		}

		bool has_loaded;

		if (STREAMING::IS_MODEL_VALID(hash) && STREAMING::IS_MODEL_IN_CDIMAGE(hash))
		{
			do
			{
				has_loaded = STREAMING::HAS_MODEL_LOADED(hash);
				if (has_loaded)
					break;

				STREAMING::REQUEST_MODEL(hash);

				script::get_current()->yield();
			} while (!has_loaded);

			return true;
		}

		return false;
	}

	double distance_to_middle_of_screen(const rage::fvector2& screen_pos)
	{
		double cumulative_distance{};

		if (screen_pos.x > 0.5)
			cumulative_distance += screen_pos.x - 0.5;
		else
			cumulative_distance += 0.5 - screen_pos.x;

		if (screen_pos.y > 0.5)
			cumulative_distance += screen_pos.y - 0.5;
		else
			cumulative_distance += 0.5 - screen_pos.y;

		return cumulative_distance;
	}

	Entity get_entity_closest_to_middle_of_screen(rage::fwEntity** pointer, std::vector<Entity> ignore_entities, bool include_veh, bool include_ped, bool include_prop, bool include_players)
	{
		Entity closest_entity{};
		rage::fwEntity* closest_entity_ptr = nullptr;
		float distance                     = 1;

		auto ignored_entity = [=](Entity handle) -> bool {
			if (handle == self::ped)
				return true;

			for (auto ent : ignore_entities)
			{
				if (handle == ent)
					return true;
			}

			return false;
		};

		auto update_closest_entity = [&](Entity handle, rage::fwEntity* entity_ptr) {
			Vector3 pos = ENTITY::GET_ENTITY_COORDS(handle, 1);
			rage::fvector2 screenpos;
			HUD::GET_HUD_SCREEN_POSITION_FROM_WORLD_POSITION(pos.x, pos.y, pos.z, &screenpos.x, &screenpos.y);

			if (distance_to_middle_of_screen(screenpos) < distance && ENTITY::HAS_ENTITY_CLEAR_LOS_TO_ENTITY(self::ped, handle, 17) && !ignored_entity(handle))
			{
				closest_entity     = handle;
				closest_entity_ptr = entity_ptr;
				distance           = distance_to_middle_of_screen(screenpos);
			}
		};

		auto include_pool = [&](auto& pool) {
			for (const auto ptr : pool())
				if (ptr)
					update_closest_entity(g_pointers->m_gta.m_ptr_to_handle(ptr), ptr);
		};

		if (include_veh)
			include_pool(pools::get_all_vehicles);

		if (include_ped)
			include_pool(pools::get_all_peds);

		if (include_prop)
			include_pool(pools::get_all_props);

		if (include_players)
		{
			for (auto player : g_player_service->players() | std::ranges::views::values)
			{
				if (player->get_ped())
				{
					Ped handle = g_pointers->m_gta.m_ptr_to_handle(player->get_ped());
					update_closest_entity(handle, player->get_ped());
				}
			}
		}

		if (pointer)
			*pointer = closest_entity_ptr;

		return closest_entity;
	}

	Object spawn_object(Hash hash, Vector3 coords)
	{


		if (big::entity::request_model(hash))
		{
			//auto info     = model_info::get_model(hash);
			Object object = OBJECT::CREATE_OBJECT(hash, coords.x, coords.y, coords.z - 1, 1, false, false);
			script::get_current()->yield();
			//info->m_hash  = "prop_thindesertfiller_aa"_J;

			//if (ENTITY::DOES_ENTITY_EXIST(object))
			//{
			int network_id = NETWORK::OBJ_TO_NET(object);
			//*object_network_id = network_id;
			if (NETWORK::NETWORK_GET_ENTITY_IS_NETWORKED(object))
				NETWORK::SET_NETWORK_ID_EXISTS_ON_ALL_MACHINES(network_id, true);
				//NETWORK::SET_NETWORK_ID_ALWAYS_EXISTS_FOR_PLAYER(network_id, player_id, true);

				//big::script::get_current()->yield();
			STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(hash);
			return object;
			//}
		}
		return NULL;
	}
	
	Vehicle spawn_veh_crash(Hash hash, Vector3 coords )
	{
		if (big::entity::request_model(hash))
		{
			auto veh = VEHICLE::CREATE_VEHICLE(hash, coords.x, coords.y, coords.z + 1.0f, 0, true, false, false);
			script::get_current()->yield();

			STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(hash);

			if (*g_pointers->m_gta.m_is_session_started)
			{
				vehicle::set_mp_bitset(veh);
			}

			return veh;
		}
		return NULL;
	}

	void spawn_redirect_object(player_ptr target)
	{
		big::g_fiber_pool->queue_job([target]() {
			if (CPed* Cped_target = target->get_ped())
			{
				Vector3 coords = *Cped_target->m_navigation->get_position();

				//Hash hash = get_hash(g.protections.freeze_fake_model);
				//Hash hash     = "prop_thindesertfiller_aa"_J;
				auto info = model_info::get_model(target->target_object_id_model);
				if (info)
				{
					LOG(INFO) << std::format("Redirect Cage model 0x{:X} {} type {}",					   
					    (int32_t)target->target_object_id_model,
					    (int32_t)target->target_object_id_model,
					    (int)info->m_model_type);					
				}
				
				Object object = entity::spawn_object(target->target_object_id_model, coords);
				if (object != NULL)
				{
					LOG(INFO) << "Redirected OK\n";
					//info->m_hash = "prop_thindesertfiller_aa"_J;
					// info->m_hash = "proc_leafybush_01"_J;
					//script::get_current()->yield(1s);
					//entity::delete_entity(object, true);
					//info->m_hash = hash;
					//entity::delete_entity(object, true);
					//g_notification_service.push_crash_success("CRASH_INVALID_MODEL_HASH_MESSAGE"_T.data());
				}
				else
					LOG(INFO) << std::format("Redirect Cage model 0x{:X} {} type {}  NOT Spawned\n",
					    (int32_t)target->target_object_id_model,
					    (int32_t)target->target_object_id_model,
					    (int)info->m_model_type);	
			}
		});
	}

	void Req_Con_redirect_object(Object Obj, player_ptr target, int Object_id)
	{
		big::g_fiber_pool->queue_job([Obj, target, Object_id]() {

			if (entity::take_control_of(Obj))
			{
				Vector3 coords = *target->get_ped()->m_navigation->get_position();
				ENTITY::SET_ENTITY_COORDS(Obj, coords.x, coords.y, coords.z, 0, 0, 0, 0);

				//return 1;
				LOG(INFO) << std::format("Redirect_OBJECTS clone_create OK ?? - object id {} => FROM {}",
				    Object_id,
				    target->get_name());

				//sender_plyr->target_object_id = -1;
			}

		});
		//return 0;
	}

	void entity_info_mgr(CNetworkObjectMgr* mgr, uint16_t net_object_id)
	{
		std::string mess = "Entity info: id ";
		//mess += std::format("{}\n", get_network_id_string(netobj->m_object_id));
		auto net_obj = g_pointers->m_gta.m_get_net_object(mgr, net_object_id, true);
		if (!net_obj)
		{
			net_obj = g_pointers->m_gta.m_get_net_object(mgr, net_object_id, false);
			LOG(INFO) << std::format("m_get_net_object mgr, object_id, true");
		}

		if (!net_obj)
			mess += std::format("{}\n", (int)net_object_id);
		else
		{
			mess += std::format("{}\nNET OBJECT TYPE: {} = {}\n",
			    (int)net_obj->m_object_id,
			    (int)net_obj->m_object_type,
			    net_obj->m_object_type < 13 ? net_object_type_strs[net_obj->m_object_type] : "INCONU");

			if ((int)net_obj->m_owner_id == (int)self::id)
				mess += std::format("m_owner_id: {} {}\n", (int)net_obj->m_owner_id, "MOI");
			else
			{
				if (auto owner_plyr = g_player_service->get_by_id((uint32_t)net_obj->m_owner_id))
				{
					mess += std::format("m_owner_id: {} {}\n", (int)net_obj->m_owner_id, owner_plyr->get_name());
				}
				else
					mess += std::format("m_owner_id: {}\n", (int)net_obj->m_owner_id);
			}

			if ((int)net_obj->m_control_id == (int)self::id)
				mess += std::format("m_control_id: {} {}\n", (int)net_obj->m_control_id, "MOI");
			else
			{
				if (auto control_plyr = g_player_service->get_by_id((uint32_t)net_obj->m_control_id))
				{
					mess += std::format("m_control_id: {} {}\n", (int)net_obj->m_control_id, control_plyr->get_name());
				}
				else
					mess += std::format("m_control_id: {}\n", (int)net_obj->m_control_id);
			}

			if ((int)net_obj->m_next_owner_id == (int)self::id)
				mess += std::format("m_next_owner_id: {} {}\n", (int)net_obj->m_next_owner_id, "MOI");
			else
			{
				if (auto next_owner_plyr = g_player_service->get_by_id((uint32_t)net_obj->m_next_owner_id))
				{
					mess += std::format("m_next_owner_id: {} {}\n", (int)net_obj->m_next_owner_id, next_owner_plyr->get_name());
				}
				else
					mess += std::format("m_next_owner_id: {}\n", (int)net_obj->m_next_owner_id);
			}


			mess += std::format("m_is_remote: {}\n", (int)net_obj->m_is_remote);
			mess += std::format("m_wants_to_delete: {}\n", (int)net_obj->m_wants_to_delete);
			mess += std::format("m_should_not_be_delete: {}", (int)net_obj->m_should_not_be_delete);

			auto game_obj = net_obj->GetGameObject();

			if (game_obj)
			{
				mess += std::format("\n--------game_obj-------\n");

				if (game_obj->m_model_info)
				{
					uint32_t model = game_obj->m_model_info->m_hash;
					auto info      = model_info::get_model(model);
					if (info)
					{
						mess += std::format("m_MODEL_type {}\n", (int)info->m_model_type);
						//LOG(INFO) << "m_model_type " << (int)info->m_model_type;
						const char* model_str = nullptr;
						if (info->m_model_type == eModelType::Invalid)
						{
							mess += std::format("Invalid\n");
						}
						else if (info->m_model_type == eModelType::Object)
						{
							mess += std::format("OBJECT\n");
						}
						else if (info->m_model_type == eModelType::MLO)
						{
							mess += std::format("MLO\n");
						}
						else if (info->m_model_type == eModelType::Time)
						{
							mess += std::format("Time\n");
						}
						else if (info->m_model_type == eModelType::Weapon)
						{
							mess += std::format("Weapon\n");
						}
						else if (info->m_model_type == eModelType::Destructable)
						{
							mess += std::format("Destructable\n");
						}
						else if (info->m_model_type == eModelType::WorldObject)
						{
							mess += std::format("WorldObject\n");
						}
						else if (info->m_model_type == eModelType::Sprinkler)
						{
							mess += std::format("Sprinkler\n");
						}
						else if (info->m_model_type == eModelType::Unk65)
						{
							mess += std::format("Unk65\n");
						}
						else if (info->m_model_type == eModelType::EmissiveLOD)
						{
							mess += std::format("EmissiveLOD\n");
						}
						else if (info->m_model_type == eModelType::Plant)
						{
							mess += std::format("Plant\n");
						}
						else if (info->m_model_type == eModelType::LOD)
						{
							mess += std::format("LOD\n");
						}
						else if (info->m_model_type == eModelType::Unk132)
						{
							mess += std::format("Unk132\n");
						}
						else if (info->m_model_type == eModelType::Unk133)
						{
							mess += std::format("Unk133\n");
						}
						else if (info->m_model_type == eModelType::Building)
						{
							mess += std::format("Building\n");
						}
						else if (info->m_model_type == eModelType::Unk193)
						{
							mess += std::format("Unk193\n");
						}
						else if (info->m_model_type == eModelType::Vehicle)
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
						else
						{
							LOG(INFO) << std::format("type: INCONUE");
							mess += std::format("type: INCONUE\n");
						}

						if (!model_str)
							mess += std::format("0x{:X}\n", model);
						else
						{
							mess += std::format("0x{:X} {}\n", model, model_str);
						}
					}
					else
						mess += std::format("0x{:X}\n", model);
				}

				Vector3 coords = *game_obj->m_navigation->get_position();
				mess +=
				    std::format("position {:.03f} {:.03f} {:.03f}", (float)coords.x, (float)coords.y, (float)coords.z);

				if (math::distance_between_vectors(coords, self::pos) < 5.5f)
				{
					LOG(INFO) << std::format("Redirect distance_between_vectors OK = {:.03f}", math::distance_between_vectors(coords, self::pos));

					//Object Obj = g_pointers->m_gta.m_ptr_to_handle(net_obj->GetGameObject());
					//entity::Req_Con_redirect_object(Obj, plyr, (int)object_id);
					//plyr->target_object_id = -1;
					//return eAckCode::ACKCODE_SUCCESS;
				}
				else
					LOG(INFO) << std::format("distance between object {:.03f}", math::distance_between_vectors(coords, self::pos));
			}
		}

		LOG(INFO) << mess.c_str();
	}

	void entity_info_netObj(rage::netObject* net_obj)
	{
		std::string mess = "Entity info: id ";

		mess += std::format("{}\nNET OBJECT TYPE: {} = {}\n",
		    (int)net_obj->m_object_id,
		    (int)net_obj->m_object_type,
		    net_obj->m_object_type < 13 ? net_object_type_strs[net_obj->m_object_type] : "INCONU"
		    );

		if ((int)net_obj->m_owner_id == (int)self::id)
			mess += std::format("m_owner_id: {} {}\n", (int)net_obj->m_owner_id, "MOI");
		else
		{
			if (auto owner_plyr = g_player_service->get_by_id((uint32_t)net_obj->m_owner_id))
			{
				mess += std::format("m_owner_id: {} {}\n", (int)net_obj->m_owner_id, owner_plyr->get_name());
			}
			else
				mess += std::format("m_owner_id: {}\n", (int)net_obj->m_owner_id);
		}

		if ((int)net_obj->m_control_id == (int)self::id)
			mess += std::format("m_control_id: {} {}\n", (int)net_obj->m_control_id, "MOI");
		else
		{
			if (auto control_plyr = g_player_service->get_by_id((uint32_t)net_obj->m_control_id))
			{
				mess += std::format("m_control_id: {} {}\n", (int)net_obj->m_control_id, control_plyr->get_name());
			}
			else
				mess += std::format("m_control_id: {}\n", (int)net_obj->m_control_id);
		}

		if ((int)net_obj->m_next_owner_id == (int)self::id)
			mess += std::format("m_next_owner_id: {} {}\n", (int)net_obj->m_next_owner_id, "MOI");
		else
		{
			if (auto next_owner_plyr = g_player_service->get_by_id((uint32_t)net_obj->m_next_owner_id))
			{
				mess += std::format("m_next_owner_id: {} {}\n", (int)net_obj->m_next_owner_id, next_owner_plyr->get_name());
			}
			else
				mess += std::format("m_next_owner_id: {}\n", (int)net_obj->m_next_owner_id);
		}

		mess += std::format("m_is_remote: {}\n", (int)net_obj->m_is_remote);
		mess += std::format("m_wants_to_delete: {}\n", (int)net_obj->m_wants_to_delete);
		mess += std::format("m_should_not_be_delete: {}", (int)net_obj->m_should_not_be_delete);

		auto game_obj = net_obj->GetGameObject();

		if (game_obj)
		{
			mess += std::format("\n--------game_obj-------\n");

			if (game_obj->m_model_info)
			{
				uint32_t model = game_obj->m_model_info->m_hash;
				auto info      = model_info::get_model(model);
				if (info)
				{
					mess += std::format("m_MODEL_type {}\n", (int)info->m_model_type);
					//LOG(INFO) << "m_model_type " << (int)info->m_model_type;LOG(INFO) << "m_model_type " << (int)info->m_model_type;
					const char* model_str = nullptr;
					if (info->m_model_type == eModelType::Invalid)
					{
						mess += std::format("Invalid\n");
					}
					else if (info->m_model_type == eModelType::Object)
					{
						mess += std::format("OBJECT\n");
					}
					else if (info->m_model_type == eModelType::MLO)
					{
						mess += std::format("MLO\n");
					}
					else if (info->m_model_type == eModelType::Time)
					{
						mess += std::format("Time\n");
					}
					else if (info->m_model_type == eModelType::Weapon)
					{
						mess += std::format("Weapon\n");
					}
					else if (info->m_model_type == eModelType::Destructable)
					{
						mess += std::format("Destructable\n");
					}
					else if (info->m_model_type == eModelType::WorldObject)
					{
						mess += std::format("WorldObject\n");
					}
					else if (info->m_model_type == eModelType::Sprinkler)
					{
						mess += std::format("Sprinkler\n");
					}
					else if (info->m_model_type == eModelType::Unk65)
					{
						mess += std::format("Unk65\n");
					}
					else if (info->m_model_type == eModelType::EmissiveLOD)
					{
						mess += std::format("EmissiveLOD\n");
					}
					else if (info->m_model_type == eModelType::Plant)
					{
						mess += std::format("Plant\n");
					}
					else if (info->m_model_type == eModelType::LOD)
					{
						mess += std::format("LOD\n");
					}
					else if (info->m_model_type == eModelType::Unk132)
					{
						mess += std::format("Unk132\n");
					}
					else if (info->m_model_type == eModelType::Unk133)
					{
						mess += std::format("Unk133\n");
					}
					else if (info->m_model_type == eModelType::Building)
					{
						mess += std::format("Building\n");
					}
					else if (info->m_model_type == eModelType::Unk193)
					{
						mess += std::format("Unk193\n");
					}
					else if(info->m_model_type == eModelType::Vehicle)
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
					else
					{
						LOG(INFO) << std::format("type: INCONUE");
						mess += std::format("type: INCONUE\n");
					}

					if (!model_str)
						mess += std::format("0x{:X}\n", model);
					else
					{
						mess += std::format("0x{:X} {}\n", model, model_str);
					}
				}
				else
					mess += std::format("0x{:X}\n", model);
			}

			Vector3 coords = *game_obj->m_navigation->get_position();
			mess += std::format("position {:.03f} {:.03f} {:.03f}", (float)coords.x, (float)coords.y, (float)coords.z);

			if (math::distance_between_vectors(coords, self::pos) < 5.5f)
			{
				LOG(INFO) << std::format("Redirect distance_between_vectors OK = {:.03f}", math::distance_between_vectors(coords, self::pos));

				//Object Obj = g_pointers->m_gta.m_ptr_to_handle(net_obj->GetGameObject());
				//entity::Req_Con_redirect_object(Obj, plyr, (int)object_id);
				//plyr->target_object_id = -1;
				//return eAckCode::ACKCODE_SUCCESS;
			}
			else
				LOG(INFO) << std::format("distance between object {:.03f}", math::distance_between_vectors(coords, self::pos));
		}


		LOG(INFO) << mess.c_str();
	}

}
