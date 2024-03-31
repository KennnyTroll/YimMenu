#include "gui.hpp"

#include "common.hpp"
#include "natives.hpp"
#include "renderer/renderer.hpp"
#include "script.hpp"
#include "views/view.hpp"

#include <imgui.h>

#include "gta/enums.hpp"
#include "services/gui/gui_service.hpp"
#include "pointers.hpp"

#include "services/model_preview/model_preview_service.hpp"
#include "imgui_internal.h"

//Gamepad
#include <XInput.h>
#pragma comment(lib, "Xinput.lib")
#pragma comment(lib, "Xinput9_1_0.lib")

//Gamepad
typedef DWORD(WINAPI* PFN_XInputGetCapabilities)(DWORD, DWORD, XINPUT_CAPABILITIES*);
typedef DWORD(WINAPI* PFN_XInputGetState)(DWORD, XINPUT_STATE*);
static HMODULE g_XInputDLL                               = NULL;
static PFN_XInputGetCapabilities g_XInputGetCapabilities = NULL;
static PFN_XInputGetState g_XInputGetState               = NULL;
static bool g_WantUpdateHasGamepad                       = true;
static bool g_HasGamepad                                 = false;
//Timeeer MainTimer;
static bool g_Log_GamePad_Imput_Stat = false;

static bool g_Enable_CONTROL_ACTION = false;

namespace big
{
	struct funcs
	{
		static bool IsLegacyNativeDupe(ImGuiKey key)
		{
			return key < 512 && ImGui::GetIO().KeyMap[key] != -1;
		}
	};

	float Get_Stick_Val(SHORT sThumb, int XINPUT__GAMEPAD__TRIGGER__THRESHOLD, int XINPUT__Offset)
	{
		float vn = (float)(sThumb - XINPUT__GAMEPAD__TRIGGER__THRESHOLD) / (float)(XINPUT__Offset - XINPUT__GAMEPAD__TRIGGER__THRESHOLD);
		return vn;
	}

	/**
	 * @brief The later an entry comes in this enum to higher up it comes in the z-index.
	 */
	enum eRenderPriority
	{
		// low priority
		ESP,
		CONTEXT_MENU,

		// medium priority
		MENU = 0x1000,
		VEHICLE_CONTROL,

		// high priority
		INFO_OVERLAY = 0x2000,
		CMD_EXECUTOR,

		GTA_DATA_CACHE = 0x3000,
		ONBOARDING,

		// should remain in a league of its own
		NOTIFICATIONS = 0x4000,
	};

	gui::gui() :
	    m_is_open(false),
	    m_override_mouse(false)
	{
		g_renderer.add_dx_callback(view::notifications, eRenderPriority::NOTIFICATIONS);
		g_renderer.add_dx_callback(view::onboarding, eRenderPriority::ONBOARDING);
		g_renderer.add_dx_callback(view::gta_data, eRenderPriority::GTA_DATA_CACHE);
		g_renderer.add_dx_callback(view::cmd_executor, eRenderPriority::CMD_EXECUTOR);
		g_renderer.add_dx_callback(view::overlay, eRenderPriority::INFO_OVERLAY);

		g_renderer.add_dx_callback(view::vehicle_control, eRenderPriority::VEHICLE_CONTROL);
		g_renderer.add_dx_callback(esp::draw, eRenderPriority::ESP); // TODO: move to ESP service
		g_renderer.add_dx_callback(view::context_menu, eRenderPriority::CONTEXT_MENU);

		g_renderer.add_dx_callback(
		    [this] {
			    dx_on_tick();
		    },
		    eRenderPriority::MENU);

		g_renderer.add_wndproc_callback([this](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
			wndproc(hwnd, msg, wparam, lparam);
		});

		g_renderer.add_wndproc_callback([](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
			if (g.cmd_executor.enabled && msg == WM_KEYUP && wparam == VK_ESCAPE)
			{
				g.cmd_executor.enabled = false;
			}
		});


		dx_init();

		g_gui = this;
		g_renderer.rescale(g.window.gui_scale);
	}

	gui::~gui()
	{
		g_gui = nullptr;
	}

	bool gui::is_open()
	{
		return m_is_open;
	}

	void gui::toggle(bool toggle)
	{
		m_is_open = toggle;

		toggle_mouse();
	}

	void gui::override_mouse(bool override)
	{
		m_override_mouse = override;

		toggle_mouse();
	}

	void gui::dx_init()
	{
		static auto bgColor     = ImVec4(0.09f, 0.094f, 0.129f, .9f);
		static auto primary     = ImVec4(0.172f, 0.380f, 0.909f, 1.f);
		static auto secondary   = ImVec4(0.443f, 0.654f, 0.819f, 1.f);
		static auto whiteBroken = ImVec4(0.792f, 0.784f, 0.827f, 1.f);

		auto& style             = ImGui::GetStyle();
		style.WindowPadding     = ImVec2(15, 15);
		style.WindowRounding    = 10.f;
		style.WindowBorderSize  = 0.f;
		style.FramePadding      = ImVec2(5, 5);
		style.FrameRounding     = 4.0f;
		style.ItemSpacing       = ImVec2(12, 8);
		style.ItemInnerSpacing  = ImVec2(8, 6);
		style.IndentSpacing     = 25.0f;
		style.ScrollbarSize     = 15.0f;
		style.ScrollbarRounding = 9.0f;
		style.GrabMinSize       = 5.0f;
		style.GrabRounding      = 3.0f;
		style.ChildRounding     = 4.0f;

		auto& colors                          = style.Colors;
		colors[ImGuiCol_Text]                 = ImGui::ColorConvertU32ToFloat4(g.window.text_color);
		colors[ImGuiCol_TextDisabled]         = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		colors[ImGuiCol_WindowBg]             = ImGui::ColorConvertU32ToFloat4(g.window.background_color);
		colors[ImGuiCol_ChildBg]              = ImGui::ColorConvertU32ToFloat4(g.window.background_color);
		colors[ImGuiCol_PopupBg]              = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		colors[ImGuiCol_Border]               = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
		colors[ImGuiCol_BorderShadow]         = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
		colors[ImGuiCol_FrameBg]              = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		colors[ImGuiCol_FrameBgActive]        = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		colors[ImGuiCol_TitleBg]              = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
		colors[ImGuiCol_TitleBgActive]        = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		colors[ImGuiCol_MenuBarBg]            = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		colors[ImGuiCol_CheckMark]            = ImVec4(1.00f, 0.98f, 0.95f, 0.61f);
		colors[ImGuiCol_SliderGrab]           = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		colors[ImGuiCol_Button]               = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		colors[ImGuiCol_ButtonHovered]        = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		colors[ImGuiCol_ButtonActive]         = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		colors[ImGuiCol_Header]               = ImVec4(0.30f, 0.29f, 0.32f, 1.00f);
		colors[ImGuiCol_HeaderHovered]        = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		colors[ImGuiCol_HeaderActive]         = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		colors[ImGuiCol_ResizeGrip]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_ResizeGripHovered]    = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		colors[ImGuiCol_ResizeGripActive]     = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		colors[ImGuiCol_PlotLines]            = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		colors[ImGuiCol_PlotLinesHovered]     = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogram]        = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg]       = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);

		save_default_style();
	}

	void gui::dx_on_tick()
	{
		if (m_is_open)
		{
			push_theme_colors();
			view::root(); // frame bg
			pop_theme_colors();
		}
	}

	void gui::save_default_style()
	{
		memcpy(&m_default_config, &ImGui::GetStyle(), sizeof(ImGuiStyle));
	}

	void gui::restore_default_style()
	{
		memcpy(&ImGui::GetStyle(), &m_default_config, sizeof(ImGuiStyle));
	}

	void gui::push_theme_colors()
	{
		auto button_color = ImGui::ColorConvertU32ToFloat4(g.window.button_color);
		auto button_active_color =
		    ImVec4(button_color.x + 0.33f, button_color.y + 0.33f, button_color.z + 0.33f, button_color.w);
		auto button_hovered_color =
		    ImVec4(button_color.x + 0.15f, button_color.y + 0.15f, button_color.z + 0.15f, button_color.w);
		auto frame_color = ImGui::ColorConvertU32ToFloat4(g.window.frame_color);
		auto frame_hovered_color =
		    ImVec4(frame_color.x + 0.14f, frame_color.y + 0.14f, frame_color.z + 0.14f, button_color.w);
		auto frame_active_color =
		    ImVec4(frame_color.x + 0.30f, frame_color.y + 0.30f, frame_color.z + 0.30f, button_color.w);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ColorConvertU32ToFloat4(g.window.background_color));
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(g.window.text_color));
		ImGui::PushStyleColor(ImGuiCol_Button, button_color);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, button_hovered_color);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, button_active_color);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, frame_color);
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, frame_hovered_color);
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, frame_active_color);
	}

	void gui::pop_theme_colors()
	{
		ImGui::PopStyleColor(8);
	}

	void gui::script_on_tick()
	{
		if ((g_gui->m_is_open || g_gui->m_override_mouse) /*&& g_gui->m_is_active_view_open*/)			
		{
			//for (uint8_t i = 0; i <= 6; i++)
			//	PAD::DISABLE_CONTROL_ACTION(2, i, true);
			//PAD::DISABLE_CONTROL_ACTION(2, 106, true);
			//PAD::DISABLE_CONTROL_ACTION(2, 329, true);
			//PAD::DISABLE_CONTROL_ACTION(2, 330, true);
			//PAD::DISABLE_CONTROL_ACTION(2, 14, true);
			//PAD::DISABLE_CONTROL_ACTION(2, 15, true);
			//PAD::DISABLE_CONTROL_ACTION(2, 16, true);
			//PAD::DISABLE_CONTROL_ACTION(2, 17, true);
			//PAD::DISABLE_CONTROL_ACTION(2, 24, true);
			//PAD::DISABLE_CONTROL_ACTION(2, 69, true);
			//PAD::DISABLE_CONTROL_ACTION(2, 70, true);
			//PAD::DISABLE_CONTROL_ACTION(2, 84, true);
			//PAD::DISABLE_CONTROL_ACTION(2, 85, true);
			//PAD::DISABLE_CONTROL_ACTION(2, 99, true);
			//PAD::DISABLE_CONTROL_ACTION(2, 92, true);
			//PAD::DISABLE_CONTROL_ACTION(2, 100, true);
			//PAD::DISABLE_CONTROL_ACTION(2, 114, true);
			//PAD::DISABLE_CONTROL_ACTION(2, 115, true);
			//PAD::DISABLE_CONTROL_ACTION(2, 121, true);
			//PAD::DISABLE_CONTROL_ACTION(2, 142, true);
			//PAD::DISABLE_CONTROL_ACTION(2, 241, true);
			//PAD::DISABLE_CONTROL_ACTION(2, 261, true);
			//PAD::DISABLE_CONTROL_ACTION(2, 257, true);
			//PAD::DISABLE_CONTROL_ACTION(2, 262, true);
			//PAD::DISABLE_CONTROL_ACTION(2, 331, true);

			//https: //docs.fivem.net/docs/game-references/controls/#controls
			if (!g_Enable_CONTROL_ACTION)
			{
				for (uint8_t i = 0; i <= 6; i++)
					PAD::DISABLE_CONTROL_ACTION(0, i, true);

				PAD::DISABLE_CONTROL_ACTION(0, 24, true);  //INPUT_ATTACK
				PAD::DISABLE_CONTROL_ACTION(0, 69, true);  //INPUT_VEH_ATTACK
				PAD::DISABLE_CONTROL_ACTION(0, 70, true);  //INPUT_VEH_ATTACK2
				PAD::DISABLE_CONTROL_ACTION(0, 72, true);  //INPUT_VEH_PASSENGER_ATTACK	
				PAD::DISABLE_CONTROL_ACTION(0, 106, true); //INPUT_VEH_MOUSE_CONTROL_OVERRIDE
				PAD::DISABLE_CONTROL_ACTION(0, 122, true); //INPUT_VEH_FLY_MOUSE_CONTROL_OVERRIDE
				PAD::DISABLE_CONTROL_ACTION(0, 135, true); //INPUT_VEH_SUB_MOUSE_CONTROL_OVERRIDE
				//PAD::DISABLE_CONTROL_ACTION(0, 142, true); //INPUT_MELEE_ATTACK_ALTERNATE				
				PAD::DISABLE_CONTROL_ACTION(0, 257, true); //INPUT_ATTACK2
				PAD::DISABLE_CONTROL_ACTION(0, 331, true); //INPUT_VEH_FLY_ATTACK2
			}

			//Select : Desactive le changement de camera
			PAD::DISABLE_CONTROL_ACTION(0, (int)ControllerInputs::INPUT_NEXT_CAMERA, true);//0

			//Fleche bas :
			PAD::DISABLE_CONTROL_ACTION(0, (int)ControllerInputs::INPUT_CHARACTER_WHEEL, true);//19 Fleche bas (desactive roue des joueurs) + 166 a 169
			PAD::DISABLE_CONTROL_ACTION(0, (int)ControllerInputs::INPUT_MULTIPLAYER_INFO, true);//20

			//if (g_gui->m_is_active_view_open)		
			//Fleche Haut :
			PAD::DISABLE_CONTROL_ACTION(0, (int)ControllerInputs::INPUT_PHONE, true);//27 Fleche Haut (desactive ouverture telephone)

			//L1 : Desactive l ouverture de la roue de selection d armes
			PAD::DISABLE_CONTROL_ACTION(0, (int)ControllerInputs::INPUT_SELECT_WEAPON, true);//37

			//Fleche droite :
			PAD::DISABLE_CONTROL_ACTION(0, (int)ControllerInputs::INPUT_VEH_HEADLIGHT, true);//74 Fleche droite (desactive changement phars)
			PAD::DISABLE_CONTROL_ACTION(0, (int)ControllerInputs::INPUT_VEH_SELECT_NEXT_WEAPON, true);//99 [](desactive changement arme en vehicule)

			//Fleche gauche :
			PAD::DISABLE_CONTROL_ACTION(0, (int)ControllerInputs::INPUT_VEH_CIN_CAM, true); //8O(desactive changement Camerra en vehicule)
			PAD::DISABLE_CONTROL_ACTION(0, (int)ControllerInputs::INPUT_VEH_RADIO_WHEEL, true); //85 Fleche gauche (desactive changement radio)

			//O ou B : Desactive les actions de combat
			PAD::DISABLE_CONTROL_ACTION(0, (int)ControllerInputs::INPUT_MELEE_ATTACK_LIGHT, true);//140
			PAD::DISABLE_CONTROL_ACTION(0, (int)ControllerInputs::INPUT_MELEE_ATTACK_HEAVY, true);//141
			PAD::DISABLE_CONTROL_ACTION(0, (int)ControllerInputs::INPUT_MELEE_ATTACK_ALTERNATE, true);//142

			for (int i = 166; i <= 169; i++)
				PAD::DISABLE_CONTROL_ACTION(0, i, true);
			//INPUT_SELECT_CHARACTER_MICHAEL,
			//INPUT_SELECT_CHARACTER_FRANKLIN,
			//INPUT_SELECT_CHARACTER_TREVOR,

			PAD::DISABLE_CONTROL_ACTION(0, (int)ControllerInputs::INPUT_MP_TEXT_CHAT_ALL, true);//245
			PAD::DISABLE_CONTROL_ACTION(0, (int)ControllerInputs::INPUT_MP_TEXT_CHAT_TEAM, true);//246
			PAD::DISABLE_CONTROL_ACTION(0, (int)ControllerInputs::INPUT_MP_TEXT_CHAT_FRIENDS, true);//247
			PAD::DISABLE_CONTROL_ACTION(0, (int)ControllerInputs::INPUT_MP_TEXT_CHAT_CREW, true);//248

			//Fleche bas : Desactive les actions d enregistrement
			PAD::DISABLE_CONTROL_ACTION(0, (int)ControllerInputs::INPUT_REPLAY_START_STOP_RECORDING, true);//288
			PAD::DISABLE_CONTROL_ACTION(0, (int)ControllerInputs::INPUT_REPLAY_START_STOP_RECORDING_SECONDARY, true);//289
			PAD::DISABLE_CONTROL_ACTION(0, (int)ControllerInputs::INPUT_REPLAY_RECORD, true);//302

			PAD::DISABLE_CONTROL_ACTION(0, (int)ControllerInputs::INPUT_MAP_POI, true);//348

			if (g.self.disable_test != -1)
			{
				int acc_f = g.self.disable_test + g.self.disable_plus_test;
				PAD::DISABLE_CONTROL_ACTION(0, acc_f, true); //348
			}
			

			


			//PAD::DISABLE_CONTROL_ACTION(2, (int)ControllerInputs::INPUT_WEAPON_WHEEL_NEXT, true); //14
			//PAD::DISABLE_CONTROL_ACTION(2, (int)ControllerInputs::INPUT_WEAPON_WHEEL_PREV, true); //15
			//PAD::DISABLE_CONTROL_ACTION(2, (int)ControllerInputs::INPUT_SELECT_NEXT_WEAPON, true); //16
			//PAD::DISABLE_CONTROL_ACTION(2, (int)ControllerInputs::INPUT_SELECT_PREV_WEAPON, true); //17
			//PAD::DISABLE_CONTROL_ACTION(2, (int)ControllerInputs::INPUT_VEH_PREV_RADIO_TRACK, true); //84			
			//PAD::DISABLE_CONTROL_ACTION(2, (int)ControllerInputs::INPUT_VEH_PASSENGER_ATTACK, true); //92			
			//PAD::DISABLE_CONTROL_ACTION(2, (int)ControllerInputs::INPUT_VEH_SELECT_PREV_WEAPON, true); //100			
			//PAD::DISABLE_CONTROL_ACTION(2, (int)ControllerInputs::INPUT_VEH_FLY_ATTACK, true);//114	
			//PAD::DISABLE_CONTROL_ACTION(2, (int)ControllerInputs::INPUT_VEH_FLY_SELECT_NEXT_WEAPON, true);//115				
			//PAD::DISABLE_CONTROL_ACTION(2, (int)ControllerInputs::INPUT_VEH_FLY_ATTACK_CAMERA, true);//121		
			//PAD::DISABLE_CONTROL_ACTION(2, (int)ControllerInputs::INPUT_MELEE_ATTACK_HEAVY, true);//141			
			//PAD::DISABLE_CONTROL_ACTION(2, (int)ControllerInputs::INPUT_SELECT_WEAPON_SMG, true);//161
			//PAD::DISABLE_CONTROL_ACTION(2, (int)ControllerInputs::INPUT_SELECT_WEAPON_AUTO_RIFLE, true);//162
			//PAD::DISABLE_CONTROL_ACTION(2, (int)ControllerInputs::INPUT_VEH_DRIVE_LOOK, true);//329			
			//PAD::DISABLE_CONTROL_ACTION(2, (int)ControllerInputs::INPUT_VEH_DRIVE_LOOK2, true);//330	
			

		}
	}

	void gui::script_func()
	{
		while (true)
		{
			g_gui->script_on_tick();
			script::get_current()->yield();
		}
	}

	static auto last_time = std::chrono::steady_clock::now();

	void gui::wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		const auto time_now           = std::chrono::steady_clock::now();
		const auto elapsed_time_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_now - last_time);

		XINPUT_STATE xinput_state;
		static POINT cursor_coords{};
		if (g_WantUpdateHasGamepad)
		{
			const char* xinput_dll_names[] = {
			    "xinput1_4.dll",   // Windows 8+
			    "xinput1_3.dll",   // DirectX SDK
			    "xinput9_1_0.dll", // Windows Vista, Windows 7
			    "xinput1_2.dll",   // DirectX SDK
			    "xinput1_1.dll"    // DirectX SDK
			};
			for (int n = 0; n < IM_ARRAYSIZE(xinput_dll_names); n++)
			{
				if (HMODULE dll = ::LoadLibraryA(xinput_dll_names[n]))
				{
					g_XInputDLL             = dll;
					g_XInputGetCapabilities = (PFN_XInputGetCapabilities)::GetProcAddress(dll, "XInputGetCapabilities");
					g_XInputGetState        = (PFN_XInputGetState)::GetProcAddress(dll, "XInputGetState");
					break;
				}
			}

			XINPUT_CAPABILITIES caps;
			g_HasGamepad = g_XInputGetCapabilities ? (g_XInputGetCapabilities(0, XINPUT_FLAG_GAMEPAD, &caps) == ERROR_SUCCESS) : false;
			if (g_HasGamepad)
			{
				LOG(VERBOSE) << "Founded Gamepad Ok = " << g_HasGamepad;
			}
			else
				LOG(VERBOSE) << "Not Gamepad Founded";

			g_WantUpdateHasGamepad = false;
		}

		if (g_HasGamepad && g_XInputGetState && g_XInputGetState(0, &xinput_state) == ERROR_SUCCESS)
		{
			ImGuiIO& io = ImGui::GetIO();

			XINPUT_GAMEPAD& game_pad = xinput_state.Gamepad;

			g_Enable_CONTROL_ACTION = false;

			//MAP_ANALOG(gamepad.bLeftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, 255);
			//MAP_ANALOG(gamepad.bRightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, 255);
			float LeftTrigger_Val  = Get_Stick_Val(game_pad.bLeftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, 255);
			float RightTrigger_Val = Get_Stick_Val(game_pad.bRightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, 255);
			if (LeftTrigger_Val > 0.1f)
			{
				if (g_Log_GamePad_Imput_Stat == true)
					LOG(VERBOSE) << "-> Gamepad LeftTrigger_Val : " << LeftTrigger_Val;

				g_Enable_CONTROL_ACTION = true;
			}
			if (RightTrigger_Val > 0.1f)
			{
				if (g_Log_GamePad_Imput_Stat == true)
					LOG(VERBOSE) << "-> Gamepad RightTrigger_Val : " << RightTrigger_Val;

				g_Enable_CONTROL_ACTION = true;
			}

			float RX_moin         = Get_Stick_Val(game_pad.sThumbRX, -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, -32768);
			float RX_plus         = Get_Stick_Val(game_pad.sThumbRX, +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, +32767);
			float RY_plus         = Get_Stick_Val(game_pad.sThumbRY, +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, +32767);
			float RY_moin         = Get_Stick_Val(game_pad.sThumbRY, -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, -32768);
			bool RJ_limit_depasse = false;

			if (RX_moin > 0.1f)
			{
				if (g_Log_GamePad_Imput_Stat == true)
					LOG(VERBOSE) << "-> Gamepad RX_moin : " << RX_moin;
				RJ_limit_depasse = true;
			}
			if (RX_plus > 0.1f)
			{
				if (g_Log_GamePad_Imput_Stat == true)
					LOG(VERBOSE) << "-> Gamepad RX_plus : " << RX_plus;
				RJ_limit_depasse = true;
			}
			if (RY_plus > 0.1f)
			{
				if (g_Log_GamePad_Imput_Stat == true)
					LOG(VERBOSE) << "-> Gamepad RY_plus : " << RY_plus;
				RJ_limit_depasse = true;
			}
			if (RY_moin > 0.1f)
			{
				if (g_Log_GamePad_Imput_Stat == true)
					LOG(VERBOSE) << "-> Gamepad RY_moin : " << RY_moin;
				RJ_limit_depasse = true;
			}

			if (RJ_limit_depasse)
			{
				g_Enable_CONTROL_ACTION = true;
			}


			float LX_moin = Get_Stick_Val(game_pad.sThumbLX, -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, -32768);
			float LX_plus = Get_Stick_Val(game_pad.sThumbLX, +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, +32767);
			float LY_plus = Get_Stick_Val(game_pad.sThumbLY, +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, +32767);
			float LY_moin = Get_Stick_Val(game_pad.sThumbLY, -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, -32768);

			bool LJ_limit_depasse = false;

			if (LX_moin > 0.1f)
			{
				LJ_limit_depasse = true;
				if (g_Log_GamePad_Imput_Stat == true)
					LOG(VERBOSE) << "-> Gamepad LX_moin : " << LX_moin;
			}
			if (LX_plus > 0.1f)
			{
				LJ_limit_depasse = true;
				if (g_Log_GamePad_Imput_Stat == true)
					LOG(VERBOSE) << "-> Gamepad LX_plus : " << LX_plus;
			}
			if (LY_plus > 0.1f)
			{
				LJ_limit_depasse = true;
				if (g_Log_GamePad_Imput_Stat == true)
					LOG(VERBOSE) << "-> Gamepad LY_plus : " << LY_plus;
			}
			if (LY_moin > 0.1f)
			{
				LJ_limit_depasse = true;
				if (g_Log_GamePad_Imput_Stat == true)
					LOG(VERBOSE) << "-> Gamepad LY_moin : " << LY_moin;
			}

			if (LJ_limit_depasse)
			{
				if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) != 0 /*== 2*/)
				{
					io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;//Disabled
					if (g_Log_GamePad_Imput_Stat == true)
						LOG(VERBOSE) << "-> Gamepad Disabled : ";
				}

				g_Enable_CONTROL_ACTION = true;
			}

			if (((xinput_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) || (xinput_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
			        || (xinput_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) || (xinput_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT))
					&& !LJ_limit_depasse)
			{
				if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) == 0)
				{
					io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;//Enabled
					if (g_Log_GamePad_Imput_Stat == true)
						LOG(VERBOSE) << "->DPAD Enabled Gamepad ";
				}
			}

			if ((xinput_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) && (xinput_state.Gamepad.wButtons & XINPUT_GAMEPAD_B) && elapsed_time_in_ms >= 500ms)
			{
				g_gui->m_is_open = !g_gui->m_is_open;
				g_gui->toggle_mouse();

				//Persist and restore the cursor position between menu instances.
				static POINT cursor_coords{};
				if (g_gui->m_is_open == true)
				{
					if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) == 0)
					{
						io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; //Enabled
						if (g_Log_GamePad_Imput_Stat == true)
							LOG(VERBOSE) << "->DPAD Enabled Gamepad ";
					}

					if (g_Log_GamePad_Imput_Stat == true)
						LOG(VERBOSE) << "-> Gamepad Open Menu <-";

					GetCursorPos(&cursor_coords);
					//vehicle::set_waite_preview(false);
				}
				else if (cursor_coords.x + cursor_coords.y != 0)
				{
					if (g_Log_GamePad_Imput_Stat == true)
						LOG(VERBOSE) << "-> Gamepad Close Menu <-";

					SetCursorPos(cursor_coords.x, cursor_coords.y);


					if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) != 0 /*== 2*/)
					{
						io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad; //Disabled
						if (g_Log_GamePad_Imput_Stat == true)
							LOG(VERBOSE) << "-> Gamepad Disabled : ";
					}
					g_Enable_CONTROL_ACTION = true;

				}

				if (g.settings.hotkeys.editing_menu_toggle)
					g.settings.hotkeys.editing_menu_toggle = false;

				last_time = time_now;
			}

			else if (xinput_state.Gamepad.wButtons & XINPUT_GAMEPAD_B && elapsed_time_in_ms >= 500ms)
			{
				g_gui->m_is_active_view_open = false;
				//LOG(INFO) << "->->-> GAMEPAD_B : active_view_open = false";
				
				if (g_model_preview_service->is_runing()) //IsAnyItemFocused IsAnyItemActive
				{
					g_model_preview_service->stop_preview();
					LOG(INFO) << "->->-> GAMEPAD_B : g_model_preview_service->stop_preview()";
				}
				if (g_gui->m_is_open)
				{
					ImGui::SetWindowFocus("navigation");
					//LOG(INFO) << "->->-> GAMEPAD_B : SetWindowFocus(navigation)";
				}
				last_time = time_now;
			}
			
			if (xinput_state.Gamepad.wButtons & XINPUT_GAMEPAD_A)
			{
			}
			if (xinput_state.Gamepad.wButtons & XINPUT_GAMEPAD_X)
			{
			}
			if (xinput_state.Gamepad.wButtons & XINPUT_GAMEPAD_Y)
			{
				//if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) != 0 /*== 2*/)
				//{
				//	    io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad; //Disabled
				//	    if (g_Log_GamePad_Imput_Stat == true)
				//		    LOG(VERBOSE) << "-> Gamepad Disabled : ";
				//}
				//g_Enable_CONTROL_ACTION = true;
			}
		}


		if (msg == WM_KEYUP && wparam == g.settings.hotkeys.menu_toggle)
		{
			//Persist and restore the cursor position between menu instances.
			static POINT cursor_coords{};
			if (g_gui->m_is_open)
			{
				GetCursorPos(&cursor_coords);
			}
			else if (cursor_coords.x + cursor_coords.y != 0)
			{
				SetCursorPos(cursor_coords.x, cursor_coords.y);
			}

			toggle(g.settings.hotkeys.editing_menu_toggle || !m_is_open);
			if (g.settings.hotkeys.editing_menu_toggle)
				g.settings.hotkeys.editing_menu_toggle = false;
		}
	}

	void gui::toggle_mouse()
	{
		if (m_is_open || g_gui->m_override_mouse)
		{
			ImGui::GetIO().MouseDrawCursor = true;
			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
		}
		else
		{
			ImGui::GetIO().MouseDrawCursor = false;
			ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
		}
	}
}
