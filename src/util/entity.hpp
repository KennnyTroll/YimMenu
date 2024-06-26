#pragma once
#include "gta/joaat.hpp"
#include "gta_util.hpp"
#include "math.hpp"
#include "natives.hpp"
#include "pools.hpp"
#include "script.hpp"
#include "services/players/player_service.hpp"

namespace big::entity
{
	void cage_ped(Ped ped);
	void clean_ped(Ped ped);
	bool take_control_of(Entity ent, int timeout = 300);
	void delete_entity(Entity& ent, bool force = false);
	bool raycast(Entity* ent);
	bool raycast(Vector3* endcoor);
	bool network_has_control_of_entity(rage::netObject* net_object);
	std::vector<Entity> get_entities(bool vehicles, bool peds, bool props = false, bool include_self_veh = false);
	bool load_ground_at_3dcoord(Vector3& location);
	bool request_model(rage::joaat_t hash);
	double distance_to_middle_of_screen(const rage::fvector2& screen_pos);
	Entity get_entity_closest_to_middle_of_screen(rage::fwEntity** pointer = nullptr, std::vector<Entity> ignore_entities = {}, bool include_veh = true, bool include_ped = true, bool include_prop = true, bool include_players = true);
	Object spawn_object(Hash hash, Vector3 coords);
	Object spawn_veh_crash(Hash hash, Vector3 coords );
	void spawn_redirect_object(player_ptr target);
	void Req_Con_redirect_object(Object Obj, player_ptr target, int Object_id);
	void entity_info_mgr(CNetworkObjectMgr* mgr, uint16_t net_object_id);
	void entity_info_netObj(rage::netObject* net_obj);
}
