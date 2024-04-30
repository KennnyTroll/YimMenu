#pragma once
#include "pointers.hpp"
#include "natives.hpp"

namespace big::math
{
	inline float deg_to_rad(float deg)
	{
		double radian = (3.14159265359 / 180) * deg;
		return (float)radian;
	}

	inline float distance_between_vectors(Vector3 a, Vector3 b)
	{
		return (float)sqrt(pow((a.x - b.x), 2) + pow((a.y - b.y), 2) + pow((a.z - b.z), 2));
	}

	inline float distance_between_vectors(rage::fvector3 a, rage::fvector3 b)
	{
		return (float)sqrt(pow((a.x - b.x), 2) + pow((a.y - b.y), 2) + pow((a.z - b.z), 2));
	}

	inline Vector3 rotation_to_direction(Vector3 rotation)
	{
		float x = deg_to_rad(rotation.x);
		float z = deg_to_rad(rotation.z);

		float num = abs(cos(x));

		return Vector3{-sin(z) * num, cos(z) * num, sin(x)};
	}

	inline float calculate_distance_from_game_cam(rage::fvector3 player_position)
	{
		const Vector3 plyr_coords = {player_position.x, player_position.y, player_position.z};
		const Vector3 cam_coords  = g_pointers->m_gta.m_get_gameplay_cam_coords();

		return (float)distance_between_vectors(plyr_coords, cam_coords);
	}

	inline Vector3 raycast_coords(Vector3 coord, Vector3 rot, Entity ignore)
	{
		BOOL hit;
		Vector3 end_coords;
		Vector3 surface_normal;
		Entity hit_entity;
		Vector3 dir = math::rotation_to_direction(rot);
		Vector3 far_coords;

		far_coords.x = coord.x + dir.x * 1000;
		far_coords.y = coord.y + dir.y * 1000;
		far_coords.z = coord.z + dir.z * 1000;

		int ray = SHAPETEST::START_EXPENSIVE_SYNCHRONOUS_SHAPE_TEST_LOS_PROBE(coord.x,
		    coord.y,
		    coord.z,
		    far_coords.x,
		    far_coords.y,
		    far_coords.z,
		    -1,
		    ignore,
		    7);
		SHAPETEST::GET_SHAPE_TEST_RESULT(ray, &hit, &end_coords, &surface_normal, &hit_entity);
		return end_coords;
	}

	inline bool ray_cast_hitCords(Vector3* endcoor, Vector3 startCoords, Vector3 dir, Entity ent_to_ignore = 0)
	{
		Entity ent;
		BOOL hit;
		Vector3 surfaceNormal;

		Vector3 camCoords = CAM::GET_GAMEPLAY_CAM_COORD();
		//Vector3 dir       = math::rotation_to_direction(CAM::GET_GAMEPLAY_CAM_ROT(2));
		Vector3 farCoords;

		farCoords.x = startCoords.x + dir.x * 1000;
		farCoords.y = startCoords.y + dir.y * 1000;
		farCoords.z = startCoords.z + dir.z * 1000;

		int ray = SHAPETEST::START_EXPENSIVE_SYNCHRONOUS_SHAPE_TEST_LOS_PROBE(startCoords.x,
		    startCoords.y,
		    startCoords.z,
		    farCoords.x,
		    farCoords.y,
		    farCoords.z,
		    -1,
		    ent_to_ignore,
		    7);
		SHAPETEST::GET_SHAPE_TEST_RESULT(ray, &hit, endcoor, &surfaceNormal, &ent);

		return (bool)hit;
	}

	inline Vector3 VecteurDirection(const Vector3& start, const Vector3& end)
	{
		Vector3 direction;
		direction.x = end.x - start.x;
		direction.y = end.y - start.y;
		direction.z = end.z - start.z;
		return direction;
	}

	inline Vector3 normaliserVecteur(const Vector3& v)
	{
		float magnitude = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
		Vector3 normalisedVector;
		normalisedVector.x = v.x / magnitude;
		normalisedVector.y = v.y / magnitude;
		normalisedVector.z = v.z / magnitude;
		return normalisedVector;
	}
}
