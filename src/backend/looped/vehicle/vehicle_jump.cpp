#include "backend/looped_command.hpp"
#include "gta/enums.hpp"
#include "natives.hpp"

#include "gui.hpp"

namespace big
{
	class vehicle_jump : looped_command
	{
		using looped_command::looped_command;

		virtual void on_tick() override
		{
			PAD::DISABLE_CONTROL_ACTION(0, (int)ControllerInputs::INPUT_VEH_HANDBRAKE, false);
			if (self::veh && PAD::IS_DISABLED_CONTROL_JUST_PRESSED(0, (int)ControllerInputs::INPUT_VEH_HANDBRAKE))
			{
				ENTITY::APPLY_FORCE_TO_ENTITY(self::veh, 1, 0.0, 0.0, 20, 0.0, 0.0, 0.0, 0, 0, 1, 1, 0, 1);
			}
		}
	};

	vehicle_jump
	    g_vehicle_jump("vehjump", "VEHICLE_JUMP", "VEHICLE_JUMP_DESC", g.vehicle.vehicle_jump);



	
//#pragma region VoitureCool
//
//	bool ASgrav  = false;
//	bool Aosol   = false;
//	bool Levmod  = false;
//	bool ASboost = false;
//
//	int raction        = 0;
//	int evmod          = 0;
//	int boostNum       = 0;
//	int gravitNum      = 0;
//	float coefmultiF   = 0.5;
//	int CoehForceRLoop = 1000;
//	int CoehForce1Loop = 1400;
//
//	int Vehcool;
//
//	bool Tablllun = false;
//	bool reglde   = false;
//
//
//	const char* gravit[] = {"~g~Avec ~w~Gravite", "~r~Sans ~w~Gravite"};
//	const char* boost[]  = {"~g~Sans ~w~Boost Fly", "~r~Avec ~w~Boost Fly"};
//	const char* atraction[] = {"~g~Sans ~w~Spider Car", "~r~Avec ~w~Spider Car"};
//	const char* modevwmod[] = {"~g~Sans ~w~Welling", "~r~Avec ~w~Welling"};
//
//	float coefmultidee           = 1;
//	float puissance              = 0;
//	float coefmultidee_puissance = 11;
//
//	bool GoodmodVeh = false;
//
//	int Attenmess = 0;
//
//	#pragma endregion




	//class vehicle_cool : looped_command
	//{
	//	using looped_command::looped_command;

	//	virtual void on_tick() override
	//	{
	//		if (g_gui->is_open())
	//		{
	//			
	//		}
	//		//PAD::DISABLE_CONTROL_ACTION(0, (int)ControllerInputs::INPUT_VEH_HANDBRAKE, false);
	//		//if (self::veh && PAD::IS_DISABLED_CONTROL_JUST_PRESSED(0, (int)ControllerInputs::INPUT_VEH_HANDBRAKE))
	//		//{
	//		//	ENTITY::APPLY_FORCE_TO_ENTITY(self::veh, 1, 0.0, 0.0, 20, 0.0, 0.0, 0.0, 0, 0, 1, 1, 0, 1);
	//		//}

	//		ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_FirstUseEver, ImVec2(0.0f, 0.0f));
	//		ImGui::SetNextWindowBgAlpha(0.5f);

	//		auto window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	//		if (!g_gui->is_open())
	//		{
	//			window_flags |= ImGuiWindowFlags_NoMouseInputs;
	//		}
	//		if (ImGui::Begin("vehiclecool", nullptr, window_flags))
	//		{
	//			ImGui::Text("YimMenu");
	//			if (ASgrav)
	//			{
	//				ImGui::Text("SG");
	//			}
	//			else
	//			{
	//				ImGui::Text("AG");
	//			}

	//			if (ASboost)
	//			{
	//				ImGui::Text("ABF");
	//			}
	//			else
	//			{
	//				ImGui::Text("SBF");
	//			}

	//			/////////////////////////////////////////////////////
	//			if (Levmod)
	//			{
	//				ImGui::Text("AW");					
	//			}
	//			else
	//			{
	//				ImGui::Text("SW");					
	//			}
	//		}

	//		


	//		if (g.vehicle.vehicle_cool)
	//		{

	//			/////////////////////////////////////////////////////
	//			if (Aosol)
	//			{
	//				Levmod = false;
	//				Messagetxt(0.3, 0.4, "ASC", PixelX(Menu_X - 100 + infoMX), PixelY(613 + infoMY), 230, 0, 255, 100, true);
	//			}
	//			else //infoMX 110  1100  infoMY -313  310
	//			{
	//				Messagetxt(0.3, 0.4, "SSC", PixelX(Menu_X - 100 + infoMX), PixelY(613 + infoMY), 240, 240, 240, 50, true);
	//			}
	//			////////////////////////////////////////////////////////////////////////////////////////////////////

	//			////////////////////////////////////////////////////////////////////////////////////////////////////

	//			////////////////////////////////////////////////////////////////////////////////////////////////////

	//			if (PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), false))
	//			{
	//				Vehcool = PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), true);

	//				ENTITY::SET_ENTITY_HAS_GRAVITY(Vehcool, !ASgrav);
	//				VEHICLE::SET_VEHICLE_GRAVITY(Vehcool, !ASgrav);
	//			}
	//			Vector3 PrevRotation = ENTITY::GET_ENTITY_ROTATION(Vehcool, false);
	//			float currentSpeed   = ENTITY::GET_ENTITY_SPEED(Vehcool);
	//			float RYavance       = 0;
	//			float RX             = 0;
	//			float RYde           = 0;
	//			float RYarrier       = 0;
	//			float rotzzz;

	//			RX       = ((float)(CONTROLS::GET_CONTROL_VALUE(0, 9) - 127) / 1250);      //-127 to 127
	//			RYde     = ((float)(CONTROLS::GET_CONTROL_VALUE(0, 8) - 127) / 1250) * -1; //-127 to 127
	//			RYavance = ((float)(CONTROLS::GET_CONTROL_VALUE(0, 11) - 127) / 1250);     //-127 to 127  11: Gas Pedal
	//			RYarrier = ((float)(CONTROLS::GET_CONTROL_VALUE(0, 10) - 127) / 1250);     //-127 to 127  11: Gas Pedal

	//			//Vector3 HautJoueur = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), false);
	//			float HautSole;
	//			GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(MaCoordsNow.x, MaCoordsNow.y, 800, &HautSole, 0);
	//			float HautrrJ = MaCoordsNow.z - HautSole;

	//			if (IsKeyDown(VK_NUMPAD5) || CONTROLS::IS_CONTROL_PRESSED(0, INPUT_FRONTEND_RS) && MainTimer.IsTimerGood())
	//			{
	//				Tablllun = !Tablllun;
	//				MainTimer.Delay(200);
	//			}

	//			if (Tablllun)
	//			{
	//				if (IsKeyDown(VK_NUMPAD0) || CONTROLS::IS_CONTROL_PRESSED(0, INPUT_FRONTEND_LS) && MainTimer.IsTimerGood())
	//				{
	//					reglde = !reglde;
	//					MainTimer.Delay(200);
	//				}

	//				if (reglde == false)
	//				{
	//					TypeComandeAffiche = Aff_choixun;
	//					CONTROLS::DISABLE_CONTROL_ACTION(0, 19, true); //CharacterWheel = 19, Select persos
	//					CONTROLS::DISABLE_CONTROL_ACTION(0, 27, true); //Phone = 27,
	//					CONTROLS::DISABLE_CONTROL_ACTION(0, 74, true); //Fleche droite (desactive changement phars)
	//					CONTROLS::DISABLE_CONTROL_ACTION(0, 79, true); //R3 (desactive look back)
	//					CONTROLS::DISABLE_CONTROL_ACTION(0, 85, true); //Fleche gauche (desactive changement radio)
	//					CONTROLS::DISABLE_CONTROL_ACTION(0, 80, true); //O(desactive changement Camerra en vehicule)
	//					CONTROLS::DISABLE_CONTROL_ACTION(0, 99, true); //[](desactive changement arme en vehicule)
	//					CONTROLS::DISABLE_CONTROL_ACTION(0, 164, true);
	//					//CONTROLS::DISABLE_CONTROL_ACTION(0, 26, true);//Look en arrier//0	INPUTGROUP_SUB = 26


	//					if (IsKeyDown(VK_NUMPAD4)
	//					    || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, INPUT_CELLPHONE_LEFT) && MainTimer.IsTimerGood())
	//					{
	//						ASgrav = !ASgrav;
	//						if (!ASgrav)
	//						{
	//							gravitNum = 0;
	//						}
	//						else if (ASgrav)
	//						{
	//							gravitNum = 1;
	//						}
	//						MainTimer.Delay(300);
	//					}
	//					else if (IsKeyDown(VK_NUMPAD8)
	//					    || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, INPUT_FRONTEND_UP) && MainTimer.IsTimerGood())
	//					{
	//						ASboost = !ASboost;

	//						if (!ASboost)
	//						{
	//							boostNum = 0;
	//						}
	//						else if (ASboost)
	//						{
	//							boostNum = 1;
	//						}
	//						MainTimer.Delay(300);
	//					}
	//					else if (IsKeyDown(VK_NUMPAD6)
	//					    || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, INPUT_CELLPHONE_RIGHT) && MainTimer.IsTimerGood())
	//					{
	//						Levmod = !Levmod;

	//						if (!Levmod)
	//						{
	//							evmod = 0;
	//						}
	//						else if (Levmod)
	//						{
	//							evmod = 1;
	//						}
	//						MainTimer.Delay(300);
	//					}
	//					else if (IsKeyDown(VK_NUMPAD2)
	//					    || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, INPUT_FRONTEND_DOWN) && MainTimer.IsTimerGood())
	//					{
	//						Aosol = !Aosol;
	//						if (!Aosol)
	//						{
	//							raction = 0;
	//						}
	//						else if (Aosol)
	//						{
	//							raction = 1;
	//						}
	//						MainTimer.Delay(300);
	//					}
	//					if (IsKeyDown(VK_NUMPAD9)
	//					    || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, INPUT_FRONTEND_CANCEL) && MainTimer.IsTimerGood())
	//					{
	//						Tablllun           = false;
	//						TypeComandeAffiche = Aff_Vide;
	//						MainTimer.Delay(300);
	//					}
	//					Messagetxt(0.34, 0.475, modevwmod[evmod], PixelX(800), PixelY(510), 255, 255, 255, 240, true);
	//					Messagetxt(0.34, 0.475, atraction[raction], PixelX(640), PixelY(580), 255, 255, 255, 240, true);

	//					Messagetxt(0.34, 0.475, boost[boostNum], PixelX(640), PixelY(450), 255, 255, 255, 240, true);
	//					Messagetxt(0.34, 0.475, gravit[gravitNum], PixelX(500), PixelY(510), 255, 255, 255, 240, true);

	//					Messagetxt(0.34, 0.475, "Reglable Force Hidro/Welling", PixelX(800), PixelY(540), 255, 255, 255, 240, true);
	//					DRAW_float(0.45, 0.45, ((coefmultiF)*100), PixelX(800), PixelY(570), 255, 255, 255, 240, true);

	//					Messagetxt(0.34, 0.475, "Reglable Vitesse", PixelX(640), PixelY(480), 255, 255, 255, 240, true);
	//					DRAW_float(0.45, 0.45, ((coefmultidee)*100), PixelX(640), PixelY(510), 255, 255, 255, 240, true);
	//				}
	//			}

	//			if (reglde)
	//			{
	//				TypeComandeAffiche = Aff_choixde;
	//				CONTROLS::DISABLE_CONTROL_ACTION(0, 19, true); //CharacterWheel = 19, Select persos
	//				CONTROLS::DISABLE_CONTROL_ACTION(0, 27, true); //Phone = 27,
	//				CONTROLS::DISABLE_CONTROL_ACTION(0, 74, true); //Fleche droite (desactive changement phars)
	//				CONTROLS::DISABLE_CONTROL_ACTION(0, 79, true); //R3 (desactive look back)
	//				CONTROLS::DISABLE_CONTROL_ACTION(0, 85, true); //Fleche gauche (desactive changement radio)
	//				CONTROLS::DISABLE_CONTROL_ACTION(0, 80, true); //O(desactive changement Camerra en vehicule)
	//				CONTROLS::DISABLE_CONTROL_ACTION(0, 99, true); //[](desactive changement arme en vehicule)
	//				CONTROLS::DISABLE_CONTROL_ACTION(0, 164, true);

	//				if ((IsKeyDown(VK_NUMPAD2) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, INPUT_FRONTEND_DOWN))
	//				    && MainTimer.IsTimerGood())
	//				{
	//					coefmultidee -= 0.01;
	//					if (coefmultidee < 0)
	//						coefmultidee = 1;
	//					MainTimer.Delay(130);
	//				}
	//				else if ((IsKeyDown(VK_NUMPAD8) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, INPUT_FRONTEND_UP))
	//				    && MainTimer.IsTimerGood())
	//				{
	//					coefmultidee += 0.02;
	//					if (coefmultidee > 1)
	//						coefmultidee = 0;
	//					MainTimer.Delay(150);
	//				}
	//				else if ((IsKeyDown(VK_NUMPAD4) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, INPUT_CELLPHONE_LEFT))
	//				    && MainTimer.IsTimerGood())
	//				{
	//					coefmultiF -= 0.01;
	//					if (coefmultiF < 0)
	//						coefmultiF = 1;
	//					MainTimer.Delay(130);
	//				}
	//				else if ((IsKeyDown(VK_NUMPAD6) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, INPUT_CELLPHONE_RIGHT))
	//				    && MainTimer.IsTimerGood())
	//				{
	//					coefmultiF += 0.02;
	//					if (coefmultiF > 1)
	//						coefmultiF = 0;
	//					MainTimer.Delay(150);
	//				}
	//				else if ((IsKeyDown(VK_NUMPAD9) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, INPUT_FRONTEND_CANCEL))
	//				    && MainTimer.IsTimerGood())
	//				{
	//					Tablllun           = false;
	//					reglde             = false;
	//					TypeComandeAffiche = Aff_Vide;
	//					MainTimer.Delay(250);
	//				}
	//				Messagetxt(0.34, 0.475, "Reglable Force Hidro/Welling", PixelX(800), PixelY(540), 245, 0, 0, 180, true);
	//				DRAW_float(0.45, 0.45, ((coefmultiF)*100), PixelX(800), PixelY(570), 255, 255, 255, 240, true);

	//				Messagetxt(0.34, 0.475, "Reglable Vitesse", PixelX(640), PixelY(480), 245, 0, 0, 180, true);
	//				DRAW_float(0.45, 0.45, ((coefmultidee)*100), PixelX(640), PixelY(510), 255, 255, 255, 240, true);

	//				Messagetxt(0.34, 0.475, modevwmod[evmod], PixelX(800), PixelY(510), 255, 255, 255, 240, true);
	//				Messagetxt(0.34, 0.475, atraction[raction], PixelX(640), PixelY(580), 255, 255, 255, 240, true);

	//				Messagetxt(0.34, 0.475, boost[boostNum], PixelX(640), PixelY(450), 255, 255, 255, 240, true);
	//				Messagetxt(0.34, 0.475, gravit[gravitNum], PixelX(500), PixelY(510), 255, 255, 255, 240, true);
	//			}
	//			if (PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), false))
	//			{
	//				CAM::SET_CINEMATIC_BUTTON_ACTIVE(false);
	//				CONTROLS::DISABLE_CONTROL_ACTION(0, 80, true); //O(desactive changement Camerra en vehicule)

	//				if (Levmod)
	//				{
	//					if (HautrrJ > 2.8)
	//					{
	//						CONTROLS::DISABLE_CONTROL_ACTION(0, 99, true); //VehiclePassengerAttack = 92,Change arme en voiture
	//						CONTROLS::DISABLE_CONTROL_ACTION(0, 68, true); //VehicleMoveRightOnly = 64,Tirrer en voiture
	//						if (IsKeyDown(VK_MULTIPLY) || CONTROLS::IS_CONTROL_PRESSED(0, INPUT_FRONTEND_RB))
	//						{
	//							rotzzz = (PrevRotation.z) + (-2);
	//							ENTITY::SET_ENTITY_ROTATION(Vehcool, PrevRotation.x, PrevRotation.y, rotzzz, 0, false);
	//						}
	//						else if (IsKeyDown(VK_DIVIDE) || CONTROLS::IS_CONTROL_PRESSED(0, INPUT_FRONTEND_LB))
	//						{
	//							rotzzz = (PrevRotation.z) + (2);
	//							ENTITY::SET_ENTITY_ROTATION(Vehcool, PrevRotation.x, PrevRotation.y, rotzzz, 0, false);
	//						}
	//					}
	//				}
	//				else if (!Levmod)
	//				{
	//					if (!VEHICLE::IS_VEHICLE_ON_ALL_WHEELS(Vehcool))
	//					{
	//						CONTROLS::DISABLE_CONTROL_ACTION(0, 99, true); //VehiclePassengerAttack = 92,Change arme en voiture
	//						CONTROLS::DISABLE_CONTROL_ACTION(0, 68, true); //VehicleMoveRightOnly = 64,Tirrer en voiture
	//						if (IsKeyDown(VK_MULTIPLY) || CONTROLS::IS_CONTROL_PRESSED(0, INPUT_FRONTEND_RB))
	//						{
	//							rotzzz = (PrevRotation.z) + (-2);
	//							ENTITY::SET_ENTITY_ROTATION(Vehcool, PrevRotation.x, PrevRotation.y, rotzzz, 0, false);
	//						}
	//						else if (IsKeyDown(VK_DIVIDE) || CONTROLS::IS_CONTROL_PRESSED(0, INPUT_FRONTEND_LB))
	//						{
	//							rotzzz = (PrevRotation.z) + (2);
	//							ENTITY::SET_ENTITY_ROTATION(Vehcool, PrevRotation.x, PrevRotation.y, rotzzz, 0, false);
	//						}
	//					}
	//				}
	//				if (IsKeyDown(VK_NUMPAD1) || (CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, /*INPUT_FRONTEND_ACCEPT*/ INPUT_FRONTEND_X))) /*&& HydraulicsDelay < GAMEPLAY::GET_GAME_TIMER())*/
	//				{
	//					CONTROLS::DISABLE_CONTROL_ACTION(0, 74, true); //Fleche droite (desactive changement phars)
	//					CONTROLS::DISABLE_CONTROL_ACTION(0, 85, true); //Fleche gauche (desactive changement radio)
	//					CONTROLS::DISABLE_CONTROL_ACTION(1, 19, true); //CharacterWheel = 19, Select persos
	//					CONTROLS::DISABLE_CONTROL_ACTION(0, 27, true); //Phone = 27,
	//				}
	//				if (Aosol)
	//				{
	//					ENTITY::APPLY_FORCE_TO_ENTITY(Vehcool, 1, 0, 0, -(coefmultiF), 0, 0, 0, 0, true, true, true, false, true);
	//				}
	//				if (Levmod)
	//				{
	//					Aosol = false;
	//					CONTROLS::DISABLE_CONTROL_ACTION(0, 76, true); //Vehicle frein a mai R1
	//					CONTROLS::DISABLE_CONTROL_ACTION(0, 99, true); //VehiclePassengerAttack = 92,Change arme en voiture
	//					CONTROLS::DISABLE_CONTROL_ACTION(0, 68, true); //VehicleMoveRightOnly = 64,Tirrer en voiture
	//					if (HautrrJ <= 2.8)
	//					{
	//						if ((PrevRotation.y > -32) && (!CONTROLS::IS_CONTROL_PRESSED(0, INPUT_FRONTEND_RB) && !IsKeyDown(VK_MULTIPLY))) //R1
	//						{
	//							if (CONTROLS::IS_CONTROL_PRESSED(0, INPUT_FRONTEND_LB) || IsKeyDown(VK_DIVIDE)) //L1 (2 roue)
	//							{
	//								ENTITY::APPLY_FORCE_TO_ENTITY(Vehcool, 0, 0, 0, (10 * coefmultiF), (10 * coefmultiF), 0, 0, false, true, true, true, false, true);
	//							}
	//						}
	//						if (PrevRotation.y < 32 && (!CONTROLS::IS_CONTROL_PRESSED(0, INPUT_FRONTEND_LB) && !IsKeyDown(VK_DIVIDE))) //L1
	//						{
	//							if (CONTROLS::IS_CONTROL_PRESSED(0, INPUT_FRONTEND_RB) || IsKeyDown(VK_MULTIPLY)) //R1 (2 roue)
	//							{
	//								ENTITY::APPLY_FORCE_TO_ENTITY(Vehcool, 0, 0, 0, (10 * coefmultiF), -(10 * coefmultiF), 0, 0, false, true, true, true, false, true);
	//							}
	//						}
	//						if (!VEHICLE::IS_VEHICLE_ON_ALL_WHEELS(Vehcool))
	//						{
	//							if (RX < 0)
	//							{
	//								rotzzz = (PrevRotation.z) + (-RX * 20);
	//								ENTITY::SET_ENTITY_ROTATION(Vehcool, PrevRotation.x, PrevRotation.y, rotzzz, 0, false);
	//							}
	//							else if (RX > 0)
	//							{
	//								rotzzz = (PrevRotation.z) + (-RX * 20);
	//								ENTITY::SET_ENTITY_ROTATION(Vehcool, PrevRotation.x, PrevRotation.y, rotzzz, 0, false);
	//							}
	//						}
	//						if (PrevRotation.x < 36)
	//						{
	//							if (RYde < 0)
	//							{
	//								ENTITY::APPLY_FORCE_TO_ENTITY(Vehcool, 0, 0, 0, (/*CoehForce1*/ 40 * RYde * coefmultiF), 0, (/*CoehForceR*/ 400 * RYde * coefmultiF), 0, false, true, true, true, false, true);
	//							}
	//						}
	//						if (PrevRotation.x > -32)
	//						{
	//							if (RYde > 0)
	//							{
	//								ENTITY::APPLY_FORCE_TO_ENTITY(Vehcool, 0, 0, 0, (40 * RYde * coefmultiF), 0, -(400 * RYde * coefmultiF), 0, false, true, true, true, false, true);
	//							}
	//						}
	//					}
	//				}
	//				if (ASboost)
	//				{
	//					if ((CONTROLS::IS_CONTROL_PRESSED(0, INPUT_FRONTEND_LT) || (IsKeyDown(VK_SUBTRACT) && !Open)) && (CONTROLS::IS_CONTROL_PRESSED(0, INPUT_FRONTEND_RT) || (IsKeyDown(VK_ADD) && !Open))) /*&& Attenboost < GAMEPLAY::GET_GAME_TIMER()*/
	//					{
	//						VEHICLE::SET_VEHICLE_FORWARD_SPEED(Vehcool, -0);
	//					}
	//					else if ((CONTROLS::IS_CONTROL_PRESSED(0, INPUT_FRONTEND_LT) || IsKeyDown(VK_SUBTRACT)))
	//					{
	//						if (currentSpeed <= 149)
	//						{
	//							puissance = ((currentSpeed / 4) * RYarrier * coefmultidee);
	//							ENTITY::APPLY_FORCE_TO_ENTITY(Vehcool, 1, 0, -puissance, 0 /*coefmultiF*/, 0, 0, 0, 0, true, true, true, false, true);
	//						}
	//					}
	//					else if ((CONTROLS::IS_CONTROL_PRESSED(0, INPUT_FRONTEND_RT) || IsKeyDown(VK_ADD)))
	//					{
	//						if (currentSpeed < 149)
	//						{
	//							//puissance = currentSpeed + (RYarrier * 2.5f * coefmultidee_puissance/*35*/* coefmultidee);
	//							//VEHICLE::SET_VEHICLE_FORWARD_SPEED(Vehcool, -puissance);
	//							VEHICLE::SET_VEHICLE_FORWARD_SPEED(Vehcool, (ENTITY::GET_ENTITY_SPEED(Vehcool) + (coefmultidee * RYavance * coefmultidee_puissance /*ForceAccel*/)));
	//						}
	//					}
	//				}
	//				if (IsKeyDown(VK_NUMPAD3) || CONTROLS::IS_CONTROL_PRESSED(0, INPUT_FRONTEND_ACCEPT)) //appui X pour s'envoler
	//				{
	//					ENTITY::APPLY_FORCE_TO_ENTITY(Vehcool, 1, 0, 0, (coefmultiF + 0.2), 0, 0, 0, 0, true, true, true, false, true);
	//				}
	//				else if (IsKeyDown(VK_NUMPAD9) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, INPUT_FRONTEND_CANCEL)) //appui O pour se coller o sol
	//				{
	//					if (!Aosol)
	//					{
	//						ENTITY::APPLY_FORCE_TO_ENTITY(Vehcool, 1, 0, 0, -(coefmultiF / 2), 0, 0, 0, 0, true, true, true, false, true);
	//					}
	//				}
	//			}
	//			if (!Tablllun && !reglde)
	//			{
	//				if ((IsKeyDown(VK_NUMPAD0) || CONTROLS::IS_CONTROL_PRESSED(0, INPUT_FRONTEND_LS)) && MainTimer.IsTimerGood()) //L3
	//				{
	//					VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(Vehcool);
	//					GoodmodVeh = true;
	//					MainTimer.Delay(200);
	//				}
	//			}
	//			if (VEHICLE::IS_VEHICLE_ON_ALL_WHEELS(Vehcool))
	//			{
	//				if (!Aosol)
	//				{
	//					if (IsKeyDown(VK_NUMPAD1) || (CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, /*INPUT_FRONTEND_ACCEPT*/ INPUT_FRONTEND_X)))
	//					{
	//						CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_FRONTEND_DOWN, true);
	//						CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_FRONTEND_UP, true);
	//						CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_CELLPHONE_RIGHT, true);
	//						CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_CELLPHONE_LEFT, true);
	//						if (IsKeyDown(VK_NUMPAD8) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, INPUT_FRONTEND_UP)) //Flippes
	//						{
	//							ENTITY::APPLY_FORCE_TO_ENTITY(Vehcool, 0, 0, 0, (CoehForce1Loop * coefmultiF), 0, -(CoehForceRLoop * coefmultiF), 0, false, true, true, true, false, true);
	//						}
	//						if (IsKeyDown(VK_NUMPAD6) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, INPUT_CELLPHONE_RIGHT)) //Flippes
	//						{
	//							ENTITY::APPLY_FORCE_TO_ENTITY(Vehcool, 0, 0, 0, (CoehForceRLoop * coefmultiF), -(CoehForceRLoop * coefmultiF), 0, 0, false, true, true, true, false, true);
	//						}
	//						if (IsKeyDown(VK_NUMPAD2) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, INPUT_FRONTEND_DOWN)) //Flippes
	//						{
	//							ENTITY::APPLY_FORCE_TO_ENTITY(Vehcool, 0, 0, 0, (CoehForce1Loop * coefmultiF), 0, (CoehForceRLoop * coefmultiF), 0, false, true, true, true, false, true);
	//						}
	//						if (IsKeyDown(VK_NUMPAD4) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, INPUT_CELLPHONE_LEFT)) //Flippes
	//						{
	//							ENTITY::APPLY_FORCE_TO_ENTITY(Vehcool, 0, 0, 0, (CoehForceRLoop * coefmultiF), (CoehForceRLoop * coefmultiF), 0, 0, false, true, true, true, false, true);
	//						}
	//					}
	//				}
	//			}
	//			if (((IsKeyDown(VK_NUMPAD1) && IsKeyDown(VK_NUMPAD3)) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, /*INPUT_FRONTEND_ACCEPT*/ INPUT_FRONTEND_X)) && (CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, INPUT_FRONTEND_ACCEPT)))
	//			{
	//				ASgrav  = false;
	//				Levmod  = false;
	//				Aosol   = false;
	//				ASboost = false;

	//				raction   = 0;
	//				evmod     = 0;
	//				boostNum  = 0;
	//				gravitNum = 0;

	//				ENTITY::SET_ENTITY_HAS_GRAVITY(Vehcool, !ASgrav);
	//				VEHICLE::SET_VEHICLE_GRAVITY(Vehcool, !ASgrav);
	//				if (!OptActivOuInactiv(2, "Invincible") && !OptActivOuInactiv(2, "Pas De Chute"))
	//				{
	//					PED::SET_PED_CAN_RAGDOLL(PLAYER::PLAYER_PED_ID(), true);
	//				}
	//				TypeComandeAffiche = Aff_Vide;
	//				ChangeEtatOpt(5, "Voiture Cool", false);
	//			}
	//		}

	//	}
	//};

	//vehicle_cool g_vehicle_cool("vehcool", "Vehicle Cool", "Vehicle Cool Man", g.vehicle.vehicle_cool);









}
