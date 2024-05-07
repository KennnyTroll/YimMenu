#include "backend/looped/looped.hpp"
#include "backend/looped_command.hpp"
#include "fiber_pool.hpp"
#include "natives.hpp"

#include <base/phArchetype.hpp>
#include <base/phBoundComposite.hpp>

#include "util/teleport.hpp"

namespace big
{
	class no_collision : looped_command
	{
		using looped_command::looped_command;

		virtual void on_tick() override
		{
			if (g_local_player) [[likely]]
				((rage::phBoundComposite*)g_local_player->m_navigation->m_damp->m_bound)
				    ->m_bounds[0]
				    ->m_bounding_box_max_xyz_margin_w.w = -1;
		}

		virtual void on_disable() override
		{
			if (g_local_player) [[likely]]
				((rage::phBoundComposite*)g_local_player->m_navigation->m_damp->m_bound)
				    ->m_bounds[0]
				    ->m_bounding_box_max_xyz_margin_w.w = 0.25;
		}
	};
	no_collision g_no_collision("nocollision", "NO_COLLISION", "NO_COLLISION_DESC", g.self.no_collision);


	Vector3 savedpose;
	Vector3 savepose = {-806.737, 8987.396, 6000.0};
	//bool Save_Speed_Bool = false;
	class teleport_to_safetypoint : looped_command
	{
		using looped_command::looped_command;

		virtual void on_enable() override
		{
			savedpose = self::pos;

			teleport::to_coords(savepose);
			ENTITY::FREEZE_ENTITY_POSITION(self::ped, true);
			ENTITY::SET_ENTITY_HAS_GRAVITY(self::ped, false);
			ENTITY::FREEZE_ENTITY_POSITION(self::ped, false);
			
			g.session.block_send_clone_sync_all = true;
			g.session.block_receiv_cad_all = true;
		}

		virtual void on_tick() override
		{
			if (g_local_player)
				((rage::phBoundComposite*)g_local_player->m_navigation->m_damp->m_bound)
				    ->m_bounds[0]
				    ->m_bounding_box_max_xyz_margin_w.w = -1;

			if (PAD::IS_CONTROL_PRESSED(0, (int)ControllerInputs::INPUT_SPRINT))
			{
				ENTITY::APPLY_FORCE_TO_ENTITY(self::ped, 1, 0, 7.6, 0, 0, 0, 0, 0, true, true, true, false, true);
			}
		}

		virtual void on_disable() override
		{
			if (g_local_player)
				((rage::phBoundComposite*)g_local_player->m_navigation->m_damp->m_bound)
				    ->m_bounds[0]
				    ->m_bounding_box_max_xyz_margin_w.w = 0.25;

			ENTITY::SET_ENTITY_HAS_GRAVITY(self::ped, true);
			savedpose.z = savedpose.z - 1.0f;
			teleport::to_coords(savedpose);

			g.session.block_receiv_cad_all = false;
		}
	};
	teleport_to_safetypoint g_teleport_to_safetypoint("safetypointtp", "teleport to safety point", "props drop crash teleport to safety point for", g.self.safetypoint);
}
