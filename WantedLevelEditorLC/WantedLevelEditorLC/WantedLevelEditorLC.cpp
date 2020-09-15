//	https://github.com/DK22Pac/plugin-sdk
//	https://github.com/leethomason/tinyxml2
//	https://github.com/HollywoodJack/WantedLevelEditorLC

#include "plugin.h"
#include "CCamera.h"
#include "CCarAI.h"
#include "CCopPed.h"
#include "CGeneral.h"
#include "CModelInfo.h"
#include "CPedModelInfo.h"
#include "CPed.h"
#include "CPlayerPed.h"
#include "CStreaming.h"
#include "CTimer.h"
#include "CVehicle.h"
#include "CVehicleModelInfo.h"
#include "CWanted.h"
#include <array>
#include <CWorld.h>

#include "../injector/injector.hpp"
#include "DynAddress.h"

#include "extensions\ScriptCommands.h"

#include "ePedModel.h"
#include "eVehicleModel.h"
#include "ePedType.h"
#include "eObjective.h"

#include "tinyxml2.h"

using namespace plugin;
using namespace tinyxml2;

int MaxStarsLimit = 12;
int MaxChaos = 0xFFFF;
int vehicleModel = MODEL_PATRIOT;
int driverModel = MODEL_B_MAN3;
int passenger_0_model = MODEL_MALE01;
int passenger_1_model = MODEL_PIMP;
int passenger_2_model = MODEL_B_MAN1;
int driverWeapon = WEAPONTYPE_COLT45;
int passenger_0_weapon = WEAPONTYPE_AK47;
int passenger_1_weapon = WEAPONTYPE_M16;
int passenger_2_weapon = WEAPONTYPE_UZI;

static int FindPlayerPed__Fv = 0x4A1150;
static int UpdateWantedLevel__7CWantedFv = 0x4AD900;
static int CWanted__ClearQdCrimes = 0x4ADF20;
static int CPed__CPed = 0x4C41C0;
static int CPed__GiveWeapon = 0x4CF9B0;
static int CPed__SetCurrentWeapon = 0x4CFA60;
static int SetUpDriver = 0x5520C0;
static int SetupPassenger = 0x552160;
static int IsLawEnforcementVehicle__8CVehicleFv = 0x552880;
static int maxWantedLevel = 0x5F7714;
static int _maxWantedChaos = 0x5F7718;
int m_snTimeInMilliseconds = plugin::patch::GetUInt(0x885B48);

std::array<DWORD, 0xFFFF> TABLE_1;	//	OccupantCases
std::array<DWORD, 0xFFFF> TABLE_2;
std::array<DWORD, 0xFFFF> TABLE_3;
std::array<DWORD, 0xFFFF> TABLE_4;
std::array<DWORD, 0xFFFF> TABLE_5;
std::array<DWORD, 0xFFFF> TABLE_6;
std::array<DWORD, 0xFFFF> RadioTable;
std::array<DWORD, 0xFFFF> PaySpray;
std::array<DWORD, 0xFFFF> CopPedTable;
std::array<DWORD, 0xFFFF> highestCases;
std::array<DWORD, 0xFF> SetWantedCases;
std::array<DWORD, 0xFFFF> OccupantCases;
std::array<DWORD, 0xFFFF> KendyBox;
std::array<DWORD, 13> MaxWantedChaosArray;

DWORD fnc1 = 0x415C60;	//	CCarAI::AddPoliceCarOccupants(veh, true);
DWORD fnc2 = 0x4F5800;	//	CPopulation::AddPedInCar();
DWORD fnc3 = 0x4376A0;	//	CRoadblocks::GenerateRoadblockCopsForCar();
DWORD fnc5 = 0x535B40;	//	CAutomobile::PreRender();
DWORD fnc6 = 0x4C11B0;	//	CCopPed::CCopPed(eCopType copType);

DWORD copsInCar = 0x4F5870;
DWORD swatInCar = 0x4F587E;
DWORD fbiInCar = 0x4F588F;
DWORD armyInCar = 0x4F58A0;
DWORD medicsInCar = 0x4F58B1;
DWORD firemenInCar = 0x4F58BF;
DWORD taxiDriver = 0x4F58CD;
DWORD randomPedModel = 0x4F58FF;

DWORD oneOccupant = 0;
DWORD twoOccupants = 0;
DWORD threeOccupants = 0;
DWORD fourOccupants = 0;
DWORD emptyVehicle = 0;

DWORD swatRoadblock = fnc3 + 0x23A;
DWORD fbiRoadblock = fnc3 + 0x246;
DWORD armyRoadblock = fnc3 + 0x252;
DWORD civilRoadblock = fnc3 + 0x260;

byte lowestCase = 0x5A;

DWORD vehModelID = MODEL_LANDSTAL;
DWORD pedModelID = MODEL_MALE01;
DWORD pedWeapon = WEAPONTYPE_AK47;
DWORD Occupants = 4;

DWORD loc_4F5870 = 0x4F5870;
DWORD loc_4F5908 = 0x4F5908;
DWORD loc_437907 = 0x437907;
DWORD loc_4378DA = 0x4378DA;
DWORD loc_537F82 = 0x537F82;
DWORD loc_537641 = 0x537641;
DWORD loc_550C9D = 0x550C9D;
DWORD label_6ADC34 = 0x6ADC34;

DWORD _maxWantedChaos_2 = 0xFFFF;

__declspec(naked) void UpdateChoppers() {
	_asm {
		mov al, [ecx + 0x16]
		shr al, 1
		and al, 1
		jne loc_6ADC10
		mov al, [ecx + 0x16]
		and al, 1
		je loc_6ADC13
		loc_6ADC10 :
		xor eax, eax
			ret
			loc_6ADC13 :
		mov eax, [ecx + 0x18]
			mov edx, [ecx + 0x18]
			sub eax, 3
			cmp eax, 3
			ja loc_6ADC34
			jmp dword ptr[eax * 4 + 0x5F781C]
			mov eax, 1	//	heliNum	//	+ 0x29
			ret
			loc_6ADC34 :
		mov eax, 2	//	heliNum	//	+ 0x2F
			ret
	}
}

__declspec(naked) void SetWanted() {
	_asm {
		push ebx
		push ebp
		mov ebp, [esp + 0x0C]
		mov ebx, ecx
		cmp ebp, [maxWantedLevel]
		jle loc_4ADA66
		mov ebp, [maxWantedLevel]
		loc_4ADA66:
		mov ecx, ebx
			call CWanted__ClearQdCrimes
			mov eax, ebp
			mov edx, ebp
			cmp eax, 12
			ja loc_4ADAB3
			jmp dword ptr[eax * 4 + 0x5F77E4]
			mov dword ptr[ebx], 60
			jmp loc_4ADAB3
			mov dword ptr[ebx], 220
			jmp loc_4ADAB3
			mov dword ptr[ebx], 420
			jmp loc_4ADAB3
			mov dword ptr[ebx], 820
			jmp loc_4ADAB3
			mov dword ptr[ebx], 1620
			jmp loc_4ADAB3
			mov dword ptr[ebx], 3220
			jmp loc_4ADAB3
			mov dword ptr[ebx], 4820
			jmp loc_4ADAB3
			mov dword ptr[ebx], 6420
			jmp loc_4ADAB3
			mov dword ptr[ebx], 8020
			jmp loc_4ADAB3
			mov dword ptr[ebx], 9620
			jmp loc_4ADAB3
			mov dword ptr[ebx], 11220
			jmp loc_4ADAB3
			mov dword ptr[ebx], 12820
			jmp loc_4ADAB3
			mov dword ptr[ebx], 0
			loc_4ADAB3:
		mov ecx, ebx
			call UpdateWantedLevel__7CWantedFv
			pop ebp
			pop ebx
			ret 4
	}
}

__declspec(naked) void UpdateWanted() {
	_asm {
		mov eax, dword ptr[_maxWantedChaos_2]
		mov edx, [ecx]
		cmp edx, eax
		push ebx
		mov ebx, [ecx + 0x18]
		jle MAX_STARS
		mov[ecx], eax
		MAX_STARS :
		mov eax, [ecx]
			cmp eax, 12800
			jnge STAR_11
			mov[ecx + 0x18], 12
			//	mov byte ptr [ecx+0x12],3
			//	byte ptr [ecx+0x11],10
			//	word ptr [ecx+0x14],12
			jmp FINISH
			STAR_11 :
		cmp eax, 11200
			jnge STAR_10
			mov[ecx + 0x18], 11
			jmp FINISH
			STAR_10 :
		cmp eax, 9600
			jnge STAR_09
			mov[ecx + 0x18], 10
			jmp FINISH
			STAR_09 :
		cmp eax, 8000
			jnge STAR_08
			mov[ecx + 0x18], 9
			jmp FINISH
			STAR_08 :
		cmp eax, 6400
			jnge STAR_07
			mov[ecx + 0x18], 8
			jmp FINISH
			STAR_07 :
		cmp eax, 5400
			jnge STAR_06
			mov[ecx + 0x18], 7
			jmp FINISH
			STAR_06 :
		cmp eax, 3200
			jnge STAR_05
			mov[ecx + 0x18], 6
			jmp FINISH
			STAR_05 :
		cmp eax, 1600
			jnge STAR_04
			mov[ecx + 0x18], 5
			jmp FINISH
			STAR_04 :
		cmp eax, 800
			jnge STAR_03
			mov[ecx + 0x18], 4
			jmp FINISH
			STAR_03 :
		cmp eax, 400
			jnge STAR_02
			mov[ecx + 0x18], 3
			jmp FINISH
			STAR_02 :
		cmp eax, 200
			jnge STAR_01
			mov[ecx + 0x18], 2
			jmp loc_4AD9D5
			lea eax, [eax]
			STAR_01 :
			cmp eax, 28
			jnge STAR_00
			mov[ecx + 0x18], 1
			jmp loc_4AD9D5
			STAR_00 :
		mov[ecx + 0x18], 0
			loc_4AD9D5 :

			FINISH :
			cmp ebx, [ecx + 0x18]
			je loc_4AD9E8
			mov eax, m_snTimeInMilliseconds
			mov[ecx + 8], eax
			loc_4AD9E8 :
		pop ebx
			ret
	}
}

__declspec(naked) void AddCopCarOccupants() {
	_asm {
		push ebx
		mov ebx, [esp + 8]
		mov al, [ebx + 0x1F8]
		and al, 1
		je loc_415C71
		pop ebx
		ret

		loc_415C71 :
		mov al, [ebx + 0x1F8]
			and al, -2
			or al, 1
			mov[ebx + 0x1F8], al
			movsx eax, word ptr[ebx + 0x5C]
			sub eax, 0x5A
			cmp eax, 0x7FFFFFFF
			ja loc_415CD4
			jmp dword ptr[eax * 4 + 0x5EC7B0]	//	+ 0x2F + 3 = 0x32

			mov ecx, ebx						//	+ 0x36
			call SetUpDriver
			pop ebx
			ret

			mov ecx, ebx						//	+ 0x40
			call SetUpDriver
			mov ecx, ebx
			push 0
			call SetupPassenger
			pop ebx
			ret

			mov ecx, ebx						//	+ 0x54
			call SetUpDriver
			mov ecx, ebx
			push 0
			call SetupPassenger
			push 1
			call SetupPassenger
			pop ebx
			ret

			mov ecx, ebx						//	+ 0x70
			call SetUpDriver
			mov ecx, ebx
			push 0
			call SetupPassenger
			mov ecx, ebx
			push 1
			call SetupPassenger
			push 2
			call SetupPassenger

			loc_415CD4 :							//	+0x94
		pop ebx
			ret
	}
}

__declspec(naked) void AddPedInVeh() {
	_asm {
		movsx eax, word ptr[ebx + 0x5C]
		pop ecx
		pop ecx
		sub eax, 0x5A
		cmp eax, 0x7FFFFFFF
		ja loc_4F58FF
		jmp dword ptr[eax * 4 + 0x5FA9A0]
		jmp loc_4F5870

		loc_4F58FF :
		xor edi, edi
			mov[esp + 0x0C], 0
			xor ecx, ecx
			jmp loc_4F5908
	}
}

__declspec(naked) void GenerateRoadblockOfficers() {
	_asm {
		movsx eax, word ptr[ebx + 0x5C]		//	0F BF 43 5C
		add esp, 0x0C						//	83 C4 0C
		sub eax, 0x5A						//	83 E8 5A
		cmp eax, 0x7FFFFFFF					//	83 F8 FF FF FF 7F
		ja loc_437900
		jmp dword ptr[eax * 4 + 0x5EE714]
		jmp loc_4378DA
		loc_437900 :
		xor edi, edi
			mov eax, 1
			jmp loc_437907
	}
}

__declspec(naked) void IsLawVehicle() {
	_asm {
		movsx eax, word ptr[ecx + 0x5C]
		sub eax, 0x5A
		cmp eax, 0x7FFFFFFF
		ja loc_452896
		jmp dword ptr[eax * 4 + 0x60284C]
		mov al, 01
		ret
		loc_452896 :
		xor al, al
			ret
	}
}

__declspec(naked) void RadioForCopCars() {	//	0x57E6A0
	_asm {
		sub esp, 8
		mov edx, [esp + 0x0C]
		mov[esp + 4], ecx
		movsx eax, word ptr[edx + 0x5C]
		sub eax, 0x5A
		cmp eax, 0x7FFFFFFF
		ja loc_57E6C6
		jmp dword ptr[eax * 4 + 0x60D560]
		add esp, 8
		mov al, 1
		ret 4
		loc_57E6C6:
		xor al, al
			add esp, 8
			ret 4
	}
}

__declspec(naked) void CAutomobile__PreRender() {	//	0x5373F0
	_asm {
		movsx   eax, word ptr[ebp + 0x5C]
		lea     edx, [eax - 0x5A]
		mov     edi, eax
		cmp     edx, 0x7FFFFFFF
		ja      lab_537F82
		jmp		dword ptr[edx * 4 + 0x600AD0]

		cmp     byte ptr[ebp + 0x22E], 0
		jz      lab_537F82
		mov     ecx, eax
		sub     eax, 0x5A
		cmp     eax, 0x7FFFFFFF
		ja      lab_537641
		jmp		dword ptr[eax * 4 + 0x600A7C]

		lab_537F82:
		jmp loc_537F82
			lab_537641 :
		jmp loc_537641
	}
}

__declspec(naked) void ResprayFnc() {	//	0x426700
	_asm {
		mov ecx, [esp + 4]
		movsx eax, word ptr[ecx + 0x5C]
		sub eax, 0x5A
		cmp eax, 0x7FFFFFFF
		ja loc_426720
		jmp dword ptr[eax * 4 + 0x5ED1FC]
		xor al, al
		ret
		lea eax, [eax]
		loc_426720:
		mov al, 01
			ret
	}
}

__declspec(naked) void CopPedFunc() {	//	0x4C11D3
	_asm {
		mov[eax + 0x550], ebx
		mov eax, ebx
		cmp eax, 3
		ja loc_4C135C
		jmp dword ptr[eax * 4 + 0x5F8268]
		mov ecx, [esp + 4]
		mov ebx, [ecx]
		push 0x7FFFFFFF
		call dword ptr[ebx + 0x0C]
		mov ecx, [esp + 4]
		push 0x3E8
		push 2
		call CPed__GiveWeapon
		mov ecx, [esp + 4]
		push 2
		call CPed__SetCurrentWeapon
		mov eax, [esp + 4]
		mov byte ptr[eax + 0x498], 0
		mov eax, [esp + 4]
		mov dword ptr[eax + 0x2C0], 0x42C80000
		mov eax, [esp + 4]
		mov dword ptr[eax + 0x2C4], 0
		mov eax, [esp + 4]
		mov byte ptr[eax + 0x49A], -0x30
		mov eax, [esp + 4]
		mov byte ptr[eax + 0x49B], 0x3C
		jmp loc_4C135C

		mov ecx, [esp + 4]
		mov ebx, [ecx]
		push 0x7FFFFFFF
		call dword ptr[ebx + 0x0C]
		mov ecx, [esp + 4]
		push 0x3E8
		push 2
		call CPed__GiveWeapon
		mov ecx, [esp + 4]
		push 0x3E8
		push 3
		call CPed__GiveWeapon
		mov ecx, [esp + 4]
		push 3
		call CPed__SetCurrentWeapon
		mov eax, [esp + 4]
		mov dword ptr[eax + 0x2C0], 0x42C80000
		mov eax, [esp + 4]
		mov dword ptr[eax + 0x2C4], 0x42480000
		mov eax, [esp + 4]
		mov byte ptr[eax + 0x49A], 0x20
		mov eax, [esp + 4]
		mov byte ptr[eax + 0x49B], 0x44
		jmp loc_4C135C

		mov ecx, [esp + 4]
		mov ebx, [ecx]
		push 0x7FFFFFFF
		call dword ptr[ebx + 0x0C]
		mov ecx, [esp + 4]
		push 0x3E8
		push 2
		call CPed__GiveWeapon
		mov ecx, [esp + 4]
		push 0x3E8
		push 5
		call CPed__GiveWeapon
		mov ecx, [esp + 4]
		push 5
		call CPed__SetCurrentWeapon
		mov eax, [esp + 4]
		mov dword ptr[eax + 0x2C0], 0x42C80000
		mov eax, [esp + 4]
		mov dword ptr[eax + 0x2C4], 0x42C80000
		mov eax, [esp + 4]
		mov byte ptr[eax + 0x49A], -0x50
		mov eax, [esp + 4]
		mov byte ptr[eax + 0x49B], 0x4C
		jmp loc_4C135C

		mov ecx, [esp + 4]
		mov ebx, [ecx]
		push 0x7FFFFFFF
		call dword ptr[ebx + 0x0C]
		mov ecx, [esp + 4]
		push 0x3E8
		push 2
		call CPed__GiveWeapon
		mov ecx, [esp + 4]
		push 0x3E8
		push 6
		call CPed__GiveWeapon
		mov ecx, [esp + 4]
		push 0x0A
		push 0x0B
		call CPed__GiveWeapon
		mov ecx, [esp + 4]
		push 6
		call CPed__SetCurrentWeapon
		mov eax, [esp + 4]
		mov dword ptr[eax + 0x2C0], 0x42C80000
		mov eax, [esp + 4]
		mov dword ptr[eax + 0x2C4], 0x42C80000
		mov eax, [esp + 4]
		mov byte ptr[eax + 0x49A], 0x20
		mov eax, [esp + 4]
		mov byte ptr[eax + 0x49B], 0x54

		loc_4C135C:
		mov eax, [esp + 4]
			mov byte ptr[eax + 0x544], 0
			mov eax, [esp + 4]
			mov byte ptr[eax + 0x546], 1
			mov eax, [esp + 4]
			mov byte ptr[eax + 0x545], 0
			mov eax, [esp + 4]
			mov[eax + 0x54C], 0
			mov eax, [esp + 4]
			mov[eax + 0x4D4], 0
			mov eax, [esp + 4]
			mov byte ptr[eax + 0x547], 0
			mov eax, [esp + 4]
			mov byte ptr[eax + 0x548], 0
			mov eax, [esp + 4]
			mov byte ptr[eax + 0x549], 0
			mov eax, [esp + 4]
			mov byte ptr[eax + 0x554], -1
			mov eax, [esp + 4]
			mov[eax + 0x49C], 0
			mov eax, [esp + 4]
			add esp, 8
			pop ebx
			ret 4
	}
}

bool enable_pns_for_cop_cars = false;
bool enable_jurisdiction = false;
bool enable_multiple_models = true;
int cop_gore_level = 1;
bool enable_radio_for_cop_cars = false;
int jurStats = 0;

float cop_health = 100.0f;
float cop_car_health = 1000.0f;
float sensivity_to_crime = 1.0f;
float cop_armour = 0.0f;

int cop_health_int = 0x42C80000;
int cop_car_health_int = 0x447A0000;
int sensivity_to_crime_int = 0x3F800000;
int cop_armour_int = 0;
int inactive_star_colour_red = 0;
int inactive_star_colour_green = 0;
int inactive_star_colour_blue = 0;
int inactive_star_colour_alpha = 0;
int stars_blinking_time = 2000;
int stars_blinking_speed = 4;
int roadblock_chance = 0;
int cop_weapon_accuracy = 60;
int cop_car_arrival_time = 8000;
int cop_heli_arrival_time = 99999;
int cop_heli_respawn_time = 50000;
int max_cop_cars = 1;
int max_cop_boats = 0;
int max_foot_cops = 1;
int max_cop_helicopters = 0;
int active_star_colour_red = 255;
int active_star_colour_green = 255;
int active_star_colour_blue = 255;
int active_star_colour_alpha = 255;
int CopCarModel = MODEL_POLICE;
int CopCarColour = 0;
int CopCarOccupants = 2;
int CopCarDriverSkin = 1;
int CopCarPassenger_0_skin = 1;
int CopCarPassenger_1_skin = 1;
int CopCarPassenger_2_skin = 1;
int CopCarDriverWeapon = WEAPONTYPE_COLT45;
int CopCarPassenger_0_weapon = WEAPONTYPE_COLT45;
int CopCarPassenger_1_weapon = WEAPONTYPE_COLT45;
int CopCarPassenger_2_weapon = WEAPONTYPE_COLT45;
int foot_police_skin = MODEL_COP;
int foot_police_weapon = WEAPONTYPE_COLT45;
int copBoat = MODEL_PREDATOR;
int copBoatDriver = 1;
int copBoatOccupants = 1;
int copHeli = MODEL_CHOPPER;
int slot = 0;
int CMenuManager__Process = 0x485100;
int menuIdPointer = plugin::patch::GetUInt(CMenuManager__Process + 0x1C2);      //  0x8F5E2C
int menuId = plugin::patch::GetUInt(menuIdPointer + 0xF4);                      //  0x8F5F20
int randomNum = 0;
int randomNum_2 = 0;
int NumOfOccupants = 0;
int wantedStars = -1;
int civilVehicle = 0x552896;
int policeVehicle = 0x552893;

int RB_Vehicle_1 = MODEL_POLICE;
int RB_Vehicle_2 = MODEL_POLICE;
int RB_Vehicle_3 = MODEL_POLICE;
int RB_Vehicle_4 = MODEL_POLICE;
int RB_Vehicle_5 = MODEL_POLICE;
int RB_Vehicle_6 = MODEL_POLICE;
int RB_Ped_1 = MODEL_COP;
int RB_Ped_2 = MODEL_COP;
int RB_Ped_3 = MODEL_COP;
int RB_Ped_4 = MODEL_COP;
int RB_Ped_5 = MODEL_COP;
int RB_Ped_6 = MODEL_COP;
int RB_Ped_GUN_1 = WEAPONTYPE_COLT45;
int RB_Ped_GUN_2 = WEAPONTYPE_COLT45;
int RB_Ped_GUN_3 = WEAPONTYPE_COLT45;
int RB_Ped_GUN_4 = WEAPONTYPE_COLT45;
int RB_Ped_GUN_5 = WEAPONTYPE_COLT45;
int RB_Ped_GUN_6 = WEAPONTYPE_COLT45;

int inLoopStar = -1;
int inLoopZone = -1;

float standardPedHealth = plugin::patch::GetFloat(0x4C4225 + 6);
float standardVehHealth = plugin::patch::GetFloat(0x550C93 + 6);

class WantedLevelEditorLC {
public:
	WantedLevelEditorLC() {
		DWORD copsInCar = 0x4F5870;
		DWORD swatInCar = 0x4F587E;
		DWORD fbiInCar = 0x4F588F;
		DWORD armyInCar = 0x4F58A0;
		DWORD medicsInCar = 0x4F58B1;
		DWORD firemenInCar = 0x4F58BF;
		DWORD taxiDriver = 0x4F58CD;
		DWORD randomPedModel = 0x4F58FF;

		plugin::patch::SetUInt(0x415ED8, 0x011F870F);	//	CCarAI::FindPoliceCarSpeedForWantedLevel();
		plugin::patch::SetUShort(0x415EDC, 0);
		plugin::patch::SetUShort(0x415E50, 0x3777);		//	unclear
		plugin::patch::SetUShort(0x4216CA, 0x2A77);		//	CGameLogic::Update(); 
		plugin::patch::SetUShort(0x54927C, 0x2377);		//	CHeli::ProcessControl();
		plugin::patch::SetUChar(0x4ADBE0 + 3, 5);		//	CWanted::AreArmyRequired(();
		plugin::patch::SetUShort(0x4ADBE6, 0x0D77);		//	CWanted::AreArmyRequired(();	

		injector::MakeJMP(0x4ADC00, UpdateChoppers);

		plugin::patch::Nop(0x4F5857, 0x19);
		injector::MakeJMP(0x4F5857, AddPedInVeh);
		int newPedInVehPos = GetJMPLocation(0x4F5857);
		TABLE_2.fill(randomPedModel);	//	defaultPedInVehCase
		TABLE_2[7] = firemenInCar;		//  MODEL_FIRETRUK      
		TABLE_2[17] = fbiInCar;			//  MODEL_FBICAR     
		TABLE_2[16] = medicsInCar;		//  MODEL_AMBULAN
		TABLE_2[20] = taxiDriver;		//  MODEL_TAXI 
		TABLE_2[26] = copsInCar;		//  MODEL_POLICE  
		TABLE_2[27] = swatInCar;		//  MODEL_ENFORCER
		TABLE_2[32] = armyInCar;		//  MODEL_RHINO
		TABLE_2[33] = armyInCar;		//  MODEL_BARRACKS
		TABLE_2[38] = taxiDriver;		//  MODEL_CABBIE
		TABLE_2[58] = taxiDriver;		//  MODEL_BORGNINE
		plugin::patch::SetPointer(newPedInVehPos + 0x13, &TABLE_2);

		TABLE_3.fill(0x437900);
		TABLE_3[17] = 0x4378E6;     //  MODEL_FBICAR   
		TABLE_3[27] = 0x4378DA;     //  MODEL_ENFORCER  
		TABLE_3[33] = 0x4378F2;     //  MODEL_BARRACKS
		plugin::patch::Nop(0x4378C4, 0x16);
		injector::MakeJMP(0x4378C4, GenerateRoadblockOfficers);
		int newRoadblockOfficersPos = GetJMPLocation(0x4378C4);
		plugin::patch::SetPointer(newRoadblockOfficersPos + 0x14, &TABLE_3);

		TABLE_4.fill(civilVehicle);
		TABLE_4[17] = policeVehicle;     //  MODEL_FBICAR   
		TABLE_4[26] = policeVehicle;     //  MODEL_POLICE
		TABLE_4[27] = policeVehicle;     //  MODEL_ENFORCER  
		TABLE_4[30] = policeVehicle;     //  MODEL_PREDATOR  
		TABLE_4[32] = policeVehicle;     //  MODEL_RHINO
		TABLE_4[33] = policeVehicle;     //  MODEL_BARRACKS
		plugin::patch::Nop(0x552880, 0x18);
		injector::MakeJMP(0x552880, IsLawVehicle);
		int newIsLawVehPos = GetJMPLocation(0x552880);
		plugin::patch::SetPointer(newIsLawVehPos + 0x11, &TABLE_4);

		RadioTable.fill(0x57E6C6);
		RadioTable[17] = 0x57E6BE; //  MODEL_FBICAR
		RadioTable[26] = 0x57E6BE; //  MODEL_POLICE
		RadioTable[27] = 0x57E6BE; //  MODEL_ENFORCER
		RadioTable[30] = 0x57E6BE; //  MODEL_PREDATOR
		RadioTable[32] = 0x57E6BE; //  MODEL_RHINO
		RadioTable[33] = 0x57E6BE; //  MODEL_BARRACKS
		plugin::patch::Nop(0x57E6A0, 0x1E);
		injector::MakeJMP(0x57E6A0, RadioForCopCars);
		int newRadioForCopsPos = GetJMPLocation(0x57E6A0);
		plugin::patch::SetPointer(newRadioForCopsPos + 0x1C, &RadioTable);

		PaySpray.fill(0x426720);
		PaySpray[7] = 0x426717;  //  MODEL_FIRETRUK
		PaySpray[16] = 0x426717; //  MODEL_AMBULAN
		PaySpray[26] = 0x426717; //  MODEL_POLICE
		PaySpray[27] = 0x426717; //  MODEL_ENFORCER
		PaySpray[31] = 0x426717; //  MODEL_BUS
		PaySpray[32] = 0x426717; //  MODEL_RHINO
		PaySpray[33] = 0x426717; //  MODEL_BARRACKS
		PaySpray[36] = 0x426717; //  MODEL_DODO
		PaySpray[37] = 0x426717; //  MODEL_COACH   
		plugin::patch::Nop(0x426700, 0x17);
		injector::MakeJMP(0x426700, ResprayFnc);
		int newPaySprayPos = GetJMPLocation(0x426700);
		plugin::patch::SetPointer(newPaySprayPos + 0x15, &PaySpray);

		plugin::patch::Nop(0x4C11D3, 0x18);
		injector::MakeJMP(0x4C11D3, CopPedFunc);
		int newCopPedFuncPos = GetJMPLocation(0x4C11D3);
		CopPedTable.fill(0x4C135C);
		CopPedTable[0] = newCopPedFuncPos + 0x18;
		CopPedTable[1] = newCopPedFuncPos + 0x85;
		CopPedTable[2] = newCopPedFuncPos + 0xF8;
		CopPedTable[3] = newCopPedFuncPos + 0x168;
		plugin::patch::SetPointer(newCopPedFuncPos + 0x14, &CopPedTable);
		plugin::patch::SetUInt(newCopPedFuncPos + 0x1F, MODEL_COP);
		plugin::patch::SetUInt(newCopPedFuncPos + 0x2B, 1000);  //  ammo
		plugin::patch::SetUChar(newCopPedFuncPos + 0x30, WEAPONTYPE_COLT45);
		plugin::patch::SetUChar(newCopPedFuncPos + 0x3C, WEAPONTYPE_COLT45);
		plugin::patch::SetFloat(newCopPedFuncPos + 0x58, 100.0);  //  health
		plugin::patch::SetFloat(newCopPedFuncPos + 0x66, 0.0);  //  armour
		plugin::patch::SetUChar(newCopPedFuncPos + 0x74, -0x30);   //  unclear
		plugin::patch::SetUChar(newCopPedFuncPos + 0x7F, 0x3C);    //  accuracy

		plugin::patch::SetUInt(newCopPedFuncPos + 0x8C, MODEL_SWAT);
		plugin::patch::SetUInt(newCopPedFuncPos + 0x98, 1000);  //  ammo
		plugin::patch::SetUChar(newCopPedFuncPos + 0x9D, WEAPONTYPE_COLT45);
		plugin::patch::SetUInt(newCopPedFuncPos + 0xA9, 1000);  //  ammo
		plugin::patch::SetUChar(newCopPedFuncPos + 0xAE, WEAPONTYPE_UZI);
		plugin::patch::SetUChar(newCopPedFuncPos + 0xBA, WEAPONTYPE_UZI);
		plugin::patch::SetFloat(newCopPedFuncPos + 0xCB, 100.0);  //  health
		plugin::patch::SetFloat(newCopPedFuncPos + 0xD9, 50.0);    //  armour
		plugin::patch::SetUChar(newCopPedFuncPos + 0xE7, 0x20);    //  unclear
		plugin::patch::SetUChar(newCopPedFuncPos + 0xF2, 0x44);    //  accuracy

		plugin::patch::SetUInt(newCopPedFuncPos + 0xFF, MODEL_FBI);
		plugin::patch::SetUInt(newCopPedFuncPos + 0x10B, 1000);  //  ammo
		plugin::patch::SetUChar(newCopPedFuncPos + 0x110, WEAPONTYPE_COLT45);
		plugin::patch::SetUInt(newCopPedFuncPos + 0x11C, 1000);  //  ammo
		plugin::patch::SetUChar(newCopPedFuncPos + 0x121, WEAPONTYPE_AK47);
		plugin::patch::SetUChar(newCopPedFuncPos + 0x12D, WEAPONTYPE_AK47);
		plugin::patch::SetFloat(newCopPedFuncPos + 0x13E, 100.0);  //  health
		plugin::patch::SetFloat(newCopPedFuncPos + 0x14C, 100.0);  //  armour
		plugin::patch::SetUChar(newCopPedFuncPos + 0x15A, -0x50);   //  unclear
		plugin::patch::SetUChar(newCopPedFuncPos + 0x165, 0x4C);    //  accuracy

		plugin::patch::SetUInt(newCopPedFuncPos + 0x16F, MODEL_ARMY);
		plugin::patch::SetUInt(newCopPedFuncPos + 0x17B, 1000);  //  ammo
		plugin::patch::SetUChar(newCopPedFuncPos + 0x180, WEAPONTYPE_COLT45);
		plugin::patch::SetUInt(newCopPedFuncPos + 0x18C, 1000);  //  ammo
		plugin::patch::SetUChar(newCopPedFuncPos + 0x191, WEAPONTYPE_M16);
		plugin::patch::SetUChar(newCopPedFuncPos + 0x19D, WEAPONTYPE_MOLOTOV);
		plugin::patch::SetUChar(newCopPedFuncPos + 0x19F, WEAPONTYPE_GRENADE);
		plugin::patch::SetUChar(newCopPedFuncPos + 0x1AB, WEAPONTYPE_M16);
		plugin::patch::SetFloat(newCopPedFuncPos + 0x1BC, 100.0);  //  health
		plugin::patch::SetFloat(newCopPedFuncPos + 0x1CA, 100.0);  //  armour
		plugin::patch::SetUChar(newCopPedFuncPos + 0x1D8, 0x20);;   //  unclear
		plugin::patch::SetUChar(newCopPedFuncPos + 0x1E3, 0x54);    //  accuracy

		TABLE_5.fill(0x537F82);
		TABLE_5[7] = 0x5373F0;  //  MODEL_FIRETRUK           
		TABLE_5[16] = 0x5373F0; //  MODEL_AMBULAN            
		TABLE_5[17] = 0x537E4F; //  MODEL_FBICAR             
		TABLE_5[20] = 0x537D1F; //  MODEL_TAXI               
		TABLE_5[26] = 0x5373F0; //  MODEL_POLICE             
		TABLE_5[27] = 0x5373F0; //  MODEL_ENFORCER           
		TABLE_5[38] = 0x537D1F; //  MODEL_CABBIE             
		TABLE_5[58] = 0x537D1F; //  MODEL_BORGNINE

		TABLE_6.fill(0x537641);
		TABLE_6[7] = 0x5375B9;  //  MODEL_FIRETRUK             
		TABLE_6[16] = 0x53752C; //  MODEL_AMBULAN              
		TABLE_6[26] = 0x537412; //  MODEL_POLICE               
		TABLE_6[27] = 0x53749F; //  MODEL_ENFORCER             

		//	plugin::patch::Nop(0x5373D7, 0x3B);
		injector::MakeJMP(0x5373D7, CAutomobile__PreRender);
		int NewRenderPos = GetJMPLocation(0x5373D7);
		plugin::patch::SetPointer(NewRenderPos + 0x14, &TABLE_5);
		plugin::patch::SetPointer(NewRenderPos + 0x30, &TABLE_6);

		MaxWantedChaosArray[0] = 0;
		MaxWantedChaosArray[1] = 120;
		MaxWantedChaosArray[2] = 300;
		MaxWantedChaosArray[3] = 600;
		MaxWantedChaosArray[4] = 1200;
		MaxWantedChaosArray[5] = 2400;
		MaxWantedChaosArray[6] = 4800;
		MaxWantedChaosArray[7] = 5500;
		MaxWantedChaosArray[8] = 7200;
		MaxWantedChaosArray[9] = 8800;
		MaxWantedChaosArray[10] = 10400;
		MaxWantedChaosArray[11] = 12000;
		MaxWantedChaosArray[12] = 14000;

		Events::gameProcessEvent += [] {
			_maxWantedChaos_2 = MaxWantedChaosArray[MaxStarsLimit];
			standardPedHealth = plugin::patch::GetFloat(0x4C4225 + 6);
			standardVehHealth = plugin::patch::GetFloat(0x550C93 + 6);
			auto playa = FindPlayerPed();
			if (playa) {

				/////////////////////////////////////////////////////////////////
				int newCopPedFuncPos = GetJMPLocation(0x4C11D3);
				if (enable_jurisdiction) {
					if (playa->m_pWanted->m_nWantedLevel <= 3) {
						jurStats = newCopPedFuncPos + 0x18;
					} else {
						if (playa->m_pWanted->m_nWantedLevel == 4) {
							jurStats = newCopPedFuncPos + 0x85;
						} else {
							if (playa->m_pWanted->m_nWantedLevel == 5) {
								jurStats = newCopPedFuncPos + 0xF8;
							} else {
								if (playa->m_pWanted->m_nWantedLevel >= 6) {
									jurStats = newCopPedFuncPos + 0x168;
								}
							}
						}
					}
				} else {
					jurStats = newCopPedFuncPos + 0x18;
				}

				if (jurStats) {
					CopPedTable[0] = jurStats;
					CopPedTable[1] = jurStats;
					CopPedTable[2] = jurStats;
					CopPedTable[3] = jurStats;
				}
				/////////////////////////////////////////////////////////////////
				if (playa->m_pVehicle) {
					//	if (playa->m_pVehicle->IsLawEnforcementVehicle() == true) {
					if (KeyPressed(VK_CAPITAL)) {
						//	Command<Commands::LOAD_AND_LAUNCH_MISSION_INTERNAL>(13);
					}
				}

				vehModelID = MODEL_LANDSTAL;
				pedModelID = MODEL_MALE01;
				pedWeapon = WEAPONTYPE_AK47;
				Occupants = 4;
				//	

				byte max_stars = MaxStarsLimit;
				plugin::patch::SetUChar(0x506CF7 + 2, max_stars);
				injector::WriteMemory(maxWantedLevel, MaxStarsLimit, false);
				injector::WriteMemory(maxWantedLevel, MaxStarsLimit, false);
				injector::WriteMemory(_maxWantedChaos, MaxChaos, false);
				injector::MakeJMP(0x4AD900, UpdateWanted);
				injector::MakeJMP(0x4ADA50, SetWanted);
				int SetWantedNewPos = GetJMPLocation(0x4ADA50);

				injector::MakeJMP(0x415C60, AddCopCarOccupants);
				int OccupantsNewPos = GetJMPLocation(0x415C60);
				OccupantCases[0] = 0x415CD4;
				OccupantCases[1] = OccupantsNewPos + 0x36;
				OccupantCases[2] = OccupantsNewPos + 0x40;
				OccupantCases[3] = OccupantsNewPos + 0x54;
				OccupantCases[4] = OccupantsNewPos + 0x70;
				plugin::patch::SetPointer(OccupantsNewPos + 0x32, &OccupantCases);
				oneOccupant = OccupantsNewPos + 0x36;
				twoOccupants = OccupantsNewPos + 0x40;
				threeOccupants = OccupantsNewPos + 0x54;
				fourOccupants = OccupantsNewPos + 0x70;
				emptyVehicle = OccupantsNewPos + 0x94;
				OccupantCases.fill(emptyVehicle);
				//	plugin::patch::SetUChar(OccupantsNewPos + 0x67, plugin::Random(1, 2));
				OccupantCases[17] = fourOccupants;    //  case  FBICAR
				OccupantCases[26] = fourOccupants;     //  case  POLICE
				OccupantCases[27] = fourOccupants;    //  case  ENFORCER
				OccupantCases[32] = fourOccupants;     //  case  RHINO
				OccupantCases[33] = fourOccupants;     //  case  BARRACKS


				SetWantedCases[0] = 0x4ADAAD;
				SetWantedCases[1] = 0x4ADA7D;
				SetWantedCases[2] = 0x4ADA85;
				SetWantedCases[3] = 0x4ADA8D;
				SetWantedCases[4] = 0x4ADA95;
				SetWantedCases[5] = 0x4ADA9D;
				SetWantedCases[6] = 0x4ADAA5;
				SetWantedCases[7] = SetWantedNewPos + 0x5E;
				SetWantedCases[8] = SetWantedNewPos + 0x66;
				SetWantedCases[9] = SetWantedNewPos + 0x6E;
				SetWantedCases[10] = SetWantedNewPos + 0x76;
				SetWantedCases[11] = SetWantedNewPos + 0x7E;
				SetWantedCases[12] = SetWantedNewPos + 0x86;
				plugin::patch::SetPointer(SetWantedNewPos + 0x2A, &SetWantedCases);

				m_snTimeInMilliseconds = plugin::patch::GetUInt(0x885B48);

				menuIdPointer = plugin::patch::GetUInt(CMenuManager__Process + 0x1C2);      //  0x8F5E2C
				menuId = plugin::patch::GetUInt(menuIdPointer + 0xF4);                      //  0x8F5F20
				if (menuId != 0 || inLoopStar != playa->m_pWanted->m_nWantedLevel) {
					tinyxml2::XMLDocument xml_doc;
					if (xml_doc.LoadFile(PLUGIN_PATH("WantedLevelEditorForGTA3.xml")) == XML_SUCCESS) {
						xml_doc.FirstChildElement("misc")->FirstChildElement("enable_pns_for_cop_cars")->QueryBoolText(&enable_pns_for_cop_cars);
						xml_doc.FirstChildElement("misc")->FirstChildElement("inactive_star_colour_red")->QueryIntText(&inactive_star_colour_red);
						xml_doc.FirstChildElement("misc")->FirstChildElement("inactive_star_colour_green")->QueryIntText(&inactive_star_colour_green);
						xml_doc.FirstChildElement("misc")->FirstChildElement("inactive_star_colour_blue")->QueryIntText(&inactive_star_colour_blue);
						xml_doc.FirstChildElement("misc")->FirstChildElement("inactive_star_colour_alpha")->QueryIntText(&inactive_star_colour_alpha);
						xml_doc.FirstChildElement("misc")->FirstChildElement("stars_blinking_time")->QueryIntText(&stars_blinking_time);
						xml_doc.FirstChildElement("misc")->FirstChildElement("stars_blinking_speed")->QueryIntText(&stars_blinking_speed);
						xml_doc.FirstChildElement("misc")->FirstChildElement("max_stars_limit")->QueryIntText(&MaxStarsLimit);


						wantedStars = playa->m_pWanted->m_nWantedLevel;
						std::string stars = std::to_string(wantedStars);
						std::string xml_section = "star_" + stars;

						for (int i = 1; i < 7; i++) {
							if (enable_multiple_models == false) {
								randomNum = 1;
							} else {
								randomNum = i;
							}
							std::string randomModel = std::to_string(randomNum);
							std::string cop_car_model = "cop_car_" + randomModel + "_model";
							std::string cop_car_colour = "cop_car_" + randomModel + "_colour";
							std::string cop_car_occupants = "cop_car_" + randomModel + "_occupants";
							std::string cop_car_driver_skin = "cop_car_" + randomModel + "_driver_skin";
							std::string cop_car_passenger_0_skin = "cop_car_" + randomModel + "_passenger_0_skin";
							std::string cop_car_passenger_1_skin = "cop_car_" + randomModel + "_passenger_1_skin";
							std::string cop_car_passenger_2_skin = "cop_car_" + randomModel + "_passenger_2_skin";
							std::string cop_car_driver_weapon = "cop_car_" + randomModel + "_driver_weapon";
							std::string cop_car_passenger_0_weapon = "cop_car_" + randomModel + "_passenger_0_weapon";
							std::string cop_car_passenger_1_weapon = "cop_car_" + randomModel + "_passenger_1_weapon";
							std::string cop_car_passenger_2_weapon = "cop_car_" + randomModel + "_passenger_2_weapon";
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement("cop_health")->QueryFloatText(&cop_health);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement("cop_car_health")->QueryFloatText(&cop_car_health);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement("sensivity_to_crime")->QueryFloatText(&sensivity_to_crime);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement("cop_armour")->QueryFloatText(&cop_armour);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement("enable_jurisdiction")->QueryBoolText(&enable_jurisdiction);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement("enable_multiple_models")->QueryBoolText(&enable_multiple_models);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement("cop_gore_level")->QueryIntText(&cop_gore_level);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement("roadblock_chance")->QueryIntText(&roadblock_chance);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement("cop_weapon_accuracy")->QueryIntText(&cop_weapon_accuracy);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement("cop_car_arrival_time")->QueryIntText(&cop_car_arrival_time);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement("cop_heli_arrival_time")->QueryIntText(&cop_heli_arrival_time);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement("cop_heli_respawn_time")->QueryIntText(&cop_heli_respawn_time);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement("max_cop_cars")->QueryIntText(&max_cop_cars);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement("max_cop_boats")->QueryIntText(&max_cop_boats);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement("max_foot_cops")->QueryIntText(&max_foot_cops);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement("max_cop_helicopters")->QueryIntText(&max_cop_helicopters);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement("active_star_colour_red")->QueryIntText(&active_star_colour_red);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement("active_star_colour_green")->QueryIntText(&active_star_colour_green);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement("active_star_colour_blue")->QueryIntText(&active_star_colour_blue);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement("active_star_colour_alpha")->QueryIntText(&active_star_colour_alpha);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement(cop_car_model.c_str())->QueryIntText(&CopCarModel);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement(cop_car_colour.c_str())->QueryIntText(&CopCarColour);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement(cop_car_occupants.c_str())->QueryIntText(&CopCarOccupants);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement(cop_car_driver_skin.c_str())->QueryIntText(&CopCarDriverSkin);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement(cop_car_passenger_0_skin.c_str())->QueryIntText(&CopCarPassenger_0_skin);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement(cop_car_passenger_1_skin.c_str())->QueryIntText(&CopCarPassenger_1_skin);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement(cop_car_passenger_2_skin.c_str())->QueryIntText(&CopCarPassenger_2_skin);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement(cop_car_driver_weapon.c_str())->QueryIntText(&CopCarDriverWeapon);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement(cop_car_passenger_0_weapon.c_str())->QueryIntText(&CopCarPassenger_0_weapon);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement(cop_car_passenger_1_weapon.c_str())->QueryIntText(&CopCarPassenger_1_weapon);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement(cop_car_passenger_2_weapon.c_str())->QueryIntText(&CopCarPassenger_2_weapon);

							xml_doc.FirstChildElement("misc")->FirstChildElement("enable_radio_for_cop_cars")->QueryBoolText(&enable_radio_for_cop_cars);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement("foot_police_skin")->QueryIntText(&foot_police_skin);
							xml_doc.FirstChildElement(xml_section.c_str())->FirstChildElement("foot_police_weapon")->QueryIntText(&foot_police_weapon);

							if (CopCarModel) {
								inLoopStar = playa->m_pWanted->m_nWantedLevel;
								if (i == 1) {
									RB_Vehicle_1 = CopCarModel;
									RB_Ped_1 = CopCarDriverSkin;
									RB_Ped_GUN_1 = CopCarDriverWeapon;
								} else {
									if (i == 2) {
										RB_Vehicle_2 = CopCarModel;
										RB_Ped_2 = CopCarDriverSkin;
										RB_Ped_GUN_2 = CopCarDriverWeapon;
									} else {
										if (i == 3) {
											RB_Vehicle_3 = CopCarModel;
											RB_Ped_3 = CopCarDriverSkin;
											RB_Ped_GUN_3 = CopCarDriverWeapon;
										} else {
											if (i == 4) {
												RB_Vehicle_4 = CopCarModel;
												RB_Ped_4 = CopCarDriverSkin;
												RB_Ped_GUN_4 = CopCarDriverWeapon;
											} else {
												if (i == 5) {
													RB_Vehicle_5 = CopCarModel;
													RB_Ped_5 = CopCarDriverSkin;
													RB_Ped_GUN_5 = CopCarDriverWeapon;
												} else {
													if (i == 6) {
														RB_Vehicle_6 = CopCarModel;
														RB_Ped_6 = CopCarDriverSkin;
														RB_Ped_GUN_6 = CopCarDriverWeapon;
													}
												}
											}
										}
									}
								}
								slot = i * 34; slot -= 34;
								cop_health_int = (int)cop_health;
								cop_car_health_int = (int)cop_car_health;
								sensivity_to_crime_int = (int)sensivity_to_crime;
								cop_armour_int = (int)cop_armour;

								KendyBox[slot] = cop_health_int; slot++;
								KendyBox[slot] = cop_car_health_int; slot++;
								KendyBox[slot] = sensivity_to_crime_int; slot++;
								KendyBox[slot] = cop_armour_int; slot++;
								KendyBox[slot] = enable_jurisdiction; slot++;
								KendyBox[slot] = enable_multiple_models; slot++;
								KendyBox[slot] = cop_gore_level; slot++;
								KendyBox[slot] = roadblock_chance; slot++;
								KendyBox[slot] = cop_weapon_accuracy; slot++;
								KendyBox[slot] = cop_car_arrival_time; slot++;
								KendyBox[slot] = cop_heli_arrival_time; slot++;
								KendyBox[slot] = cop_heli_respawn_time; slot++;
								KendyBox[slot] = max_cop_cars; slot++;
								KendyBox[slot] = max_cop_boats; slot++;
								KendyBox[slot] = max_foot_cops; slot++;
								KendyBox[slot] = max_cop_helicopters; slot++;
								KendyBox[slot] = active_star_colour_red; slot++;
								KendyBox[slot] = active_star_colour_green; slot++;
								KendyBox[slot] = active_star_colour_blue; slot++;
								KendyBox[slot] = active_star_colour_alpha; slot++;
								KendyBox[slot] = CopCarModel; slot++;
								KendyBox[slot] = CopCarColour; slot++;
								KendyBox[slot] = CopCarOccupants; slot++;
								KendyBox[slot] = CopCarDriverSkin; slot++;
								KendyBox[slot] = CopCarPassenger_0_skin; slot++;
								KendyBox[slot] = CopCarPassenger_1_skin; slot++;
								KendyBox[slot] = CopCarPassenger_2_skin; slot++;
								KendyBox[slot] = CopCarDriverWeapon; slot++;
								KendyBox[slot] = CopCarPassenger_0_weapon; slot++;
								KendyBox[slot] = CopCarPassenger_1_weapon; slot++;
								KendyBox[slot] = CopCarPassenger_2_weapon; slot++;
								KendyBox[slot] = enable_radio_for_cop_cars; slot++;
								KendyBox[slot] = foot_police_skin; slot++;
								KendyBox[slot] = foot_police_weapon; slot++;

								cop_health = (float)cop_health_int;
								cop_car_health = (float)cop_car_health_int;
								sensivity_to_crime = (float)sensivity_to_crime_int;
								cop_armour = (float)cop_armour_int;
							} else {
								continue;
							}
						}
					}
				}	//	end menu check

				if (MaxStarsLimit > 12) {
					MaxStarsLimit = 12;
				}

				slot = plugin::Random(1, 6);
				slot *= 34; slot -= 34;

				cop_health_int = KendyBox[slot]; slot++;			//	000	124	248
				cop_car_health_int = KendyBox[slot]; slot++;		//	004
				sensivity_to_crime_int = KendyBox[slot]; slot++;	//	008
				cop_armour_int = KendyBox[slot]; slot++;			//	012
				enable_jurisdiction = KendyBox[slot]; slot++;		//	016
				enable_multiple_models = KendyBox[slot]; slot++;	//	020
				cop_gore_level = KendyBox[slot]; slot++;			//	024
				roadblock_chance = KendyBox[slot]; slot++;			//	028
				cop_weapon_accuracy = KendyBox[slot]; slot++;		//	032
				cop_car_arrival_time = KendyBox[slot]; slot++;		//	036
				cop_heli_arrival_time = KendyBox[slot]; slot++;		//	040
				cop_heli_respawn_time = KendyBox[slot]; slot++;		//	044
				max_cop_cars = KendyBox[slot]; slot++;				//	048
				max_cop_boats = KendyBox[slot]; slot++;				//	052
				max_foot_cops = KendyBox[slot]; slot++;				//	056
				max_cop_helicopters = KendyBox[slot]; slot++;		//	060
				active_star_colour_red = KendyBox[slot]; slot++;	//	064
				active_star_colour_green = KendyBox[slot]; slot++;	//	068
				active_star_colour_blue = KendyBox[slot]; slot++;	//	072
				active_star_colour_alpha = KendyBox[slot]; slot++;	//	076
				CopCarModel = KendyBox[slot]; slot++;				//	080
				CopCarColour = KendyBox[slot]; slot++;				//	084
				CopCarOccupants = KendyBox[slot]; slot++;			//	088
				CopCarDriverSkin = KendyBox[slot]; slot++;			//	092
				CopCarPassenger_0_skin = KendyBox[slot]; slot++;	//	096
				CopCarPassenger_1_skin = KendyBox[slot]; slot++;	//	100
				CopCarPassenger_2_skin = KendyBox[slot]; slot++;	//	104
				CopCarDriverWeapon = KendyBox[slot]; slot++;		//	108
				CopCarPassenger_0_weapon = KendyBox[slot]; slot++;	//	112
				CopCarPassenger_1_weapon = KendyBox[slot]; slot++;	//	116
				CopCarPassenger_2_weapon = KendyBox[slot]; slot++;	//	120
				enable_radio_for_cop_cars = KendyBox[slot]; slot++;
				foot_police_skin = KendyBox[slot]; slot++;
				foot_police_weapon = KendyBox[slot]; slot++;
				//	plugin::patch::SetPointer(0x506CAF, &KendyBox);	//	forTestingPurposes
				//	CopCarModel = KendyBox[80];
				//	plugin::patch::SetUInt(0x506CAF, CopCarModel);	//	forTestingPurposes

				//	memory patch
				plugin::patch::SetUChar(0x506BF0 + 1, inactive_star_colour_red);
				plugin::patch::SetUChar(0x506BEE + 1, inactive_star_colour_green);
				plugin::patch::SetUChar(0x506BEC + 1, inactive_star_colour_blue);
				plugin::patch::SetUInt(0x506BE7 + 1, inactive_star_colour_alpha);
				plugin::patch::SetUInt(0x506C61 + 1, stars_blinking_time);
				plugin::patch::SetUChar(0x506C73 + 2, stars_blinking_speed);
				int NewCopTypePos = GetJMPLocation(0x4C11D3);
				plugin::patch::SetFloat(NewCopTypePos + 0x58, 100.0);	//	cop_health
				plugin::patch::SetFloat(NewCopTypePos + 0xCB, 100.0);
				plugin::patch::SetFloat(NewCopTypePos + 0x13E, 100.0);
				plugin::patch::SetFloat(NewCopTypePos + 0x1BC, 100.0);
				plugin::patch::SetFloat(NewCopTypePos + 0x66, 0.0);	//	cop_armour
				plugin::patch::SetFloat(NewCopTypePos + 0xD9, 50.0);
				plugin::patch::SetFloat(NewCopTypePos + 0x14C, 100.0);
				plugin::patch::SetFloat(NewCopTypePos + 0x1CA, 100.0);
				playa->m_pWanted->m_fMultiplier = sensivity_to_crime;
				byte maxCops = max_foot_cops; byte maxCopCars = max_cop_cars; byte maxBoats = max_cop_boats; WORD rbChance = roadblock_chance;
				playa->m_pWanted->m_nMaxCopsInPursuit = maxCops;
				playa->m_pWanted->m_nChanceOnRoadBlock = rbChance;
				if (playa->m_pVehicle && playa->m_pVehicle->m_nVehicleClass == VEHICLE_BOAT) {
					playa->m_pWanted->m_nMaxCopCarsInPursuit = maxBoats;
					plugin::patch::SetUInt(0x41678E + 1, cop_car_arrival_time);
					plugin::patch::SetUInt(0x4167A0 + 1, cop_car_arrival_time);
				} else {
					playa->m_pWanted->m_nMaxCopCarsInPursuit = maxCopCars;
					plugin::patch::SetUInt(0x41678E + 1, cop_car_arrival_time);
					plugin::patch::SetUInt(0x4167A0 + 1, cop_car_arrival_time);
				}

				int UpdateChoppersHook = GetJMPLocation(0x4ADC00);
				if (max_cop_helicopters > 2) {
					plugin::patch::SetUInt(0x4ADC28 + 1, 2);
					plugin::patch::SetUInt(0x4ADC2E + 1, 2);

					plugin::patch::SetUInt(UpdateChoppersHook + 0x29, 2);
					plugin::patch::SetUInt(UpdateChoppersHook + 0x2F, 2);
				} else {
					plugin::patch::SetUInt(0x4ADC28 + 1, max_cop_helicopters);
					plugin::patch::SetUInt(0x4ADC2E + 1, max_cop_helicopters);


					plugin::patch::SetUInt(UpdateChoppersHook + 0x29, max_cop_helicopters);
					plugin::patch::SetUInt(UpdateChoppersHook + 0x2F, max_cop_helicopters);
				}
				plugin::patch::SetUInt(0x549285 + 1, cop_heli_arrival_time);    //  999999   cases 0-2
				plugin::patch::SetUInt(0x54928C + 1, cop_heli_arrival_time);    //  10000    case 3
				plugin::patch::SetUInt(0x549293 + 1, cop_heli_arrival_time);    //  5000     case 4
				plugin::patch::SetUInt(0x54929A + 1, cop_heli_arrival_time);    //  3500     case 5
				plugin::patch::SetUInt(0x5492A1 + 1, cop_heli_arrival_time);    //  2000     case 6
				plugin::patch::SetUInt(0x54A13E + 1, cop_heli_respawn_time);	//	50000

				byte activeRed = active_star_colour_red;
				byte activeGreen = active_star_colour_green;
				byte activeBlue = active_star_colour_blue;
				plugin::patch::SetUChar(0x506CAE + 1, activeRed);   //   193
				plugin::patch::SetUChar(0x506CA9 + 1, activeGreen);  //   164
				plugin::patch::SetUChar(0x506CA7 + 1, activeBlue);  //   120

				byte footCopGun = foot_police_weapon;
				int footCopGunModel = CPickups__ModelForWeapon(footCopGun);
				CBaseModelInfo *ModelPed_2 = reinterpret_cast<CBaseModelInfo *>(CModelInfo::ms_modelInfoPtrs[foot_police_skin]);
				CBaseModelInfo *ModelWeapon_2 = reinterpret_cast<CBaseModelInfo *>(CModelInfo::ms_modelInfoPtrs[footCopGunModel]);
				if (ModelPed_2 && ModelWeapon_2) {
					if (ModelPed_2->m_nType == MODEL_INFO_PED && ModelWeapon_2->m_nType == MODEL_INFO_SIMPLE) {
						if (Command<Commands::HAS_MODEL_LOADED>(foot_police_skin)) {	//	if (CStreaming::ms_aInfoForModel[foot_police_skin].m_nLoadState == LOADSTATE_LOADED)
							plugin::patch::SetUInt(NewCopTypePos + 0x1F, foot_police_skin);
							plugin::patch::SetUInt(NewCopTypePos + 0x8C, foot_police_skin);
							plugin::patch::SetUInt(NewCopTypePos + 0xFF, foot_police_skin);
							plugin::patch::SetUInt(NewCopTypePos + 0x16F, foot_police_skin);

							plugin::patch::SetUChar(NewCopTypePos + 0x30, footCopGun);    //  WEAPONTYPE_COLT45
							plugin::patch::SetUChar(NewCopTypePos + 0x3C, footCopGun);    //  WEAPONTYPE_COLT45
							plugin::patch::SetUChar(0x4C1E98 + 1, footCopGun);
							plugin::patch::SetUChar(NewCopTypePos + 0xAE, footCopGun);    //  WEAPONTYPE_UZI   
							plugin::patch::SetUChar(NewCopTypePos + 0xBA, footCopGun);    //  WEAPONTYPE_UZI   
							plugin::patch::SetUChar(NewCopTypePos + 0x121, footCopGun);   //  WEAPONTYPE_AK47  
							plugin::patch::SetUChar(NewCopTypePos + 0x12D, footCopGun);   //  WEAPONTYPE_AK47  
							plugin::patch::SetUChar(NewCopTypePos + 0x191, footCopGun);   //  WEAPONTYPE_M16   
							plugin::patch::SetUChar(NewCopTypePos + 0x1AB, footCopGun);		//  WEAPONTYPE_M16   
						} else {
							Command<Commands::REQUEST_MODEL>(foot_police_skin);
							Command<Commands::REQUEST_MODEL>(footCopGunModel);	//	CStreaming::RequestModel(foot_police_skin, 1);
							//	Command<Commands::LOAD_ALL_MODELS_NOW>();			//	CStreaming::LoadAllRequestedModels(false);
						}
					}
				}

				//	processVehModel
				CVehicleModelInfo *VehiModelInfo = reinterpret_cast<CVehicleModelInfo *>(CModelInfo::ms_modelInfoPtrs[CopCarModel]);
				for (int i = 1; i < 6; i++) {
					int numOfDoors = VehiModelInfo->m_nNumDoors;
					if (numOfDoors < CopCarOccupants) {
						CopCarOccupants--;
					} else {
						break;
					}
				}

				if (CopCarOccupants == 0) {
					NumOfOccupants = OccupantsNewPos + 0x94;
				} else {
					if (CopCarOccupants == 1) {
						NumOfOccupants = OccupantsNewPos + 0x36;
					} else {
						if (CopCarOccupants == 2) {
							NumOfOccupants = OccupantsNewPos + 0x40;
						} else {
							if (CopCarOccupants == 3) {
								NumOfOccupants = OccupantsNewPos + 0x54;
							} else {
								if (CopCarOccupants == 4) {
									NumOfOccupants = OccupantsNewPos + 0x70;
								} else {
									NumOfOccupants = OccupantsNewPos + 0x94;
								}
							}
						}
					}
				}

				int element = CopCarModel - 90;
				TABLE_4[element] = policeVehicle;
				if (CopCarModel != MODEL_FIRETRUK && CopCarModel != MODEL_AMBULAN && CopCarModel != MODEL_TAXI &&
					CopCarModel != MODEL_POLICE && CopCarModel != MODEL_ENFORCER && CopCarModel != MODEL_CABBIE &&
					CopCarModel != MODEL_BORGNINE) {
					TABLE_5[element] = 0x537E4F; //  MODEL_FBICAR  
				}

				if (CopCarModel != MODEL_FIRETRUK && CopCarModel != MODEL_AMBULAN && CopCarModel != MODEL_ENFORCER) {
					TABLE_6[element] = 0x537412; //  MODEL_POLICE
				}

				if (enable_radio_for_cop_cars == false) {
					RadioTable.fill(0x57E6C6);
				} else {
					RadioTable[17] = 0x57E6BE; //  MODEL_FBICAR
					RadioTable[26] = 0x57E6BE; //  MODEL_POLICE
					RadioTable[27] = 0x57E6BE; //  MODEL_ENFORCER
					RadioTable[30] = 0x57E6BE; //  MODEL_PREDATOR
					RadioTable[32] = 0x57E6BE; //  MODEL_RHINO
					RadioTable[33] = 0x57E6BE; //  MODEL_BARRACKS
					RadioTable[element] = 0x57E6BE;
				}

				if (enable_pns_for_cop_cars == true) {
					PaySpray.fill(0x426720);
				} else {
					PaySpray[7] = 0x426717;  //  MODEL_FIRETRUK
					PaySpray[16] = 0x426717; //  MODEL_AMBULAN
					PaySpray[26] = 0x426717; //  MODEL_POLICE
					PaySpray[27] = 0x426717; //  MODEL_ENFORCER
					PaySpray[31] = 0x426717; //  MODEL_BUS
					PaySpray[32] = 0x426717; //  MODEL_RHINO
					PaySpray[33] = 0x426717; //  MODEL_BARRACKS
					PaySpray[36] = 0x426717; //  MODEL_DODO
					PaySpray[37] = 0x426717; //  MODEL_COACH   
					PaySpray[element] = 0x426717;
				}

				OccupantCases[element] = NumOfOccupants;
				if (wantedStars <= 3) {
					TABLE_2[element] = 0x4F5870;
				} else {
					if (wantedStars == 4) {
						TABLE_2[element] = 0x4F587E;
						TABLE_3[element] = 0x4378DA;     //  MODEL_ENFORCER  
					} else {
						if (wantedStars == 5) {
							TABLE_2[element] = 0x4F588F;
							TABLE_3[element] = 0x4378E6;     //  MODEL_FBICAR   
						} else {
							if (wantedStars >= 6) {
								TABLE_2[element] = 0x4F58A0;
								TABLE_3[element] = 0x4378F2;     //  MODEL_BARRACKS
							}
						}
					}
				}

				if (Command<Commands::HAS_MODEL_LOADED>(CopCarModel) == false) {
					Command<Commands::REQUEST_MODEL>(CopCarModel);
					//	Command<Commands::LOAD_ALL_MODELS_NOW>();
				} else {
					if (CopCarModel) {
						plugin::patch::SetUInt(0x418223 + 1, CopCarModel);	//	74
						plugin::patch::SetUInt(0x418230 + 1, CopCarModel);	//	75
						plugin::patch::SetUInt(0x41825C + 1, CopCarModel);	//	6B
						plugin::patch::SetUInt(0x4182A6 + 1, CopCarModel);	//	7B
						plugin::patch::SetUInt(0x4182B0 + 1, CopCarModel);	//	7A
						plugin::patch::SetUInt(0x4182B6 + 1, CopCarModel);	//	74

						//	rb
						plugin::patch::SetUInt(0x437198 + 4, CopCarModel);  //   MODEL_BARRACKS
						plugin::patch::SetUInt(0x4371B6 + 4, CopCarModel);  //   MODEL_FBICAR
						plugin::patch::SetUInt(0x4371D4 + 4, CopCarModel);  //   MODEL_ENFORCER
						plugin::patch::SetUInt(0x4371E0 + 4, CopCarModel);  //   MODEL_POLICE
						plugin::patch::SetUInt(0x4371F9 + 4, CopCarModel);  //   MODEL_POLICE
					}
				}

				//	pools
				for (int i = 0; i < CPools::ms_pVehiclePool->m_nSize; i++) {
					CVehicle *veh = CPools::ms_pVehiclePool->GetAt(i);
					if (veh) {
						if (veh->IsLawEnforcementVehicle() == true) {
							byte CarColor = CopCarColour;
							veh->m_nPrimaryColor = CarColor;
							veh->m_nSecondaryColor = CarColor;
						}
					}
				}

				for (int i = 0; i < CPools::ms_pPedPool->m_nSize; i++) {
					CPed *ped = CPools::ms_pPedPool->GetAt(i);
					if (ped) {
						if (ped->m_nPedType == PEDTYPE_COP) {
							//	ped->m_aWeapons[ped->m_nWepSlot].m_nTotalAmmo = 999;
							//	ped->SetCurrentWeapon(ped->m_aWeapons[ped->m_nWepSlot].m_nType);
							//	Command<Commands::SET_CHAR_SUFFERS_CRITICAL_HITS>(ped, cop_gore_level);	//	0x589B24
							byte copGoreLevel = cop_gore_level;
							if (ped->m_nPedFlags.b66 != copGoreLevel) {
								ped->m_nPedFlags.b66 = copGoreLevel;	//	ped->m_nPedFlags.bNoCriticalHits
							}

							if (ped->m_nWepAccuracy != cop_weapon_accuracy) {
								ped->m_nWepAccuracy = cop_weapon_accuracy;

							}

							if (ped->m_pVehicle) {
								if (ped->m_bInVehicle) {
									if (ped->m_pVehicle->m_nModelIndex == CopCarModel) {
										if (ped == ped->m_pVehicle->m_pDriver) {
											changePedModel(ped, CopCarDriverSkin);
											changePedWeapon(ped, CopCarDriverWeapon);
										}

										if (ped == ped->m_pVehicle->m_pPassenger[0]) {
											changePedModel(ped, CopCarPassenger_0_skin);
											changePedWeapon(ped, CopCarPassenger_0_weapon);
										}

										if (ped == ped->m_pVehicle->m_pPassenger[1]) {
											changePedModel(ped, CopCarPassenger_1_skin);
											changePedWeapon(ped, CopCarPassenger_1_weapon);
										}

										if (ped == ped->m_pVehicle->m_pPassenger[2]) {
											changePedModel(ped, CopCarPassenger_2_skin);
											changePedWeapon(ped, CopCarPassenger_2_weapon);
										}
									}
								} else {
									//	ped not in vehicle but owns the vehicle
									if (ped->m_nPedFlags.b59 == 1) {
										if (ped->m_pVehicle->m_nModelIndex == RB_Vehicle_1) {
											changePedModel(ped, RB_Ped_1);
											changePedWeapon(ped, RB_Ped_GUN_1);
										} else {
											if (ped->m_pVehicle->m_nModelIndex == RB_Vehicle_2) {
												changePedModel(ped, RB_Ped_2);
												changePedWeapon(ped, RB_Ped_GUN_2);
											} else {
												if (ped->m_pVehicle->m_nModelIndex == RB_Vehicle_3) {
													changePedModel(ped, RB_Ped_3);
													changePedWeapon(ped, RB_Ped_GUN_3);
												} else {
													if (ped->m_pVehicle->m_nModelIndex == RB_Vehicle_4) {
														changePedModel(ped, RB_Ped_4);
														changePedWeapon(ped, RB_Ped_GUN_4);
													} else {
														if (ped->m_pVehicle->m_nModelIndex == RB_Vehicle_5) {
															changePedModel(ped, RB_Ped_5);
															changePedWeapon(ped, RB_Ped_GUN_5);
														} else {
															if (ped->m_pVehicle->m_nModelIndex == RB_Vehicle_6) {
																changePedModel(ped, RB_Ped_6);
																changePedWeapon(ped, RB_Ped_GUN_6);
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}	//	end for

			}
		};
	}

	static int GetJMPLocation(unsigned int GivenAddress) {
		int returnedAddress = plugin::patch::GetUInt(GivenAddress + 1);
		returnedAddress += GivenAddress; returnedAddress += 1; returnedAddress += 4;
		return returnedAddress;
	}

	static void changePedModel(CPed *ped, unsigned int NewModelIndex) {
		if (ped->m_nModelIndex != NewModelIndex) {
			CBaseModelInfo *ModelPed = reinterpret_cast<CBaseModelInfo *>(CModelInfo::ms_modelInfoPtrs[NewModelIndex]);
			if (ModelPed) {
				if (ModelPed->m_nType == MODEL_INFO_PED) {
					if (Command<Commands::HAS_MODEL_LOADED>(NewModelIndex) == false) {
						Command<Commands::REQUEST_MODEL>(NewModelIndex);
						//	Command<Commands::LOAD_ALL_MODELS_NOW>();
					} else {
						if (ped->m_fHealth == standardPedHealth) {
							unsigned int savedAnimGroup = ped->m_nAnimGroupId;
							ped->DeleteRwObject(); ped->m_nModelIndex = -1;
							ped->SetModelIndex(NewModelIndex);
							if (ped->m_bInVehicle == true) {
								if (ped == ped->m_pVehicle->m_pDriver) {
									ped->SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER);
								} else {
									ped->SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER);
								}
								ped->WarpPedIntoCar(ped->m_pVehicle);	//	plugin::CallMethod<0x4D7D20, CPed *, CVehicle*>(ped, ped->m_pVehicle);
							}
							ped->m_fHealth -= 0.5;
						}
					}
				}
			}
		}
	}

	static int CPickups__ModelForWeapon(WORD weaponType) {
		return plugin::CallAndReturn<DWORD, 0x430690, WORD>(weaponType);
	}

	static void CPed__ClearWeapons(CPed *ped)
	{
		((void(__thiscall *)(CPed*))0x4CFB70)(ped);
	}

	static void changePedWeapon(CPed *ped, int NewWeaponType) {
		if (ped->m_aWeapons[ped->m_nWepSlot].m_nType != NewWeaponType) {
			int WeaponTypeModel = CPickups__ModelForWeapon(NewWeaponType);
			CBaseModelInfo *ModelWeapon = reinterpret_cast<CBaseModelInfo *>(CModelInfo::ms_modelInfoPtrs[WeaponTypeModel]);
			if (ModelWeapon) {
				if (ModelWeapon->m_nType == MODEL_INFO_SIMPLE) {
					if (Command<Commands::HAS_MODEL_LOADED>(WeaponTypeModel) == false) {
						Command<Commands::REQUEST_MODEL>(WeaponTypeModel);	//	CStreaming::RequestModel(foot_police_skin, 1);
						//	Command<Commands::LOAD_ALL_MODELS_NOW>();
					} else {
						if (ped->m_nPedFlags.b59 == 1) {

						} else {
							CPed__ClearWeapons(ped);
							eWeaponType gunType = static_cast<eWeaponType>(NewWeaponType);
							ped->GiveWeapon(gunType, 999);
							ped->SetCurrentWeapon(NewWeaponType);
						}
					}
				}
			}
		}
	}
} wantedLevelEditorLC;
