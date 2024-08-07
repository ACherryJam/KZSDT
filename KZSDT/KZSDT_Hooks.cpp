#include "KZSDT_Hooks.h"

#include <iostream>

#include "minhook/include/MinHook.h"

#include "Debug.h"
#include "Global.h"

#include "KZSDT_Enemy.h"
#include "KatanaZero.h"

#define MH_ASSERT(status) if (status != MH_OK) abort();

using namespace GameMaker;

typedef void(_cdecl *EndRoom)(bool);

EndRoom OriginalEndRoom = nullptr;
void Hook_EndRoom(bool param_1)
{
	dcout << "[Hook] Ended room.\n";
	KZSDT::enemies.clear();

	OriginalEndRoom(param_1);
}

typedef void(*ObjectScript)(CInstance *, CInstance *);

ObjectScript EnemyCreate = nullptr;
ObjectScript EnemyCopCreate = nullptr;
ObjectScript EnemyMeeleCreate = nullptr;
ObjectScript EnemyGruntCreate = nullptr;
ObjectScript EnemySciCreate = nullptr;
ObjectScript EnemyShieldcopCreate = nullptr;
ObjectScript EnemyShotgunCreate = nullptr;
ObjectScript EnemyMachinegunCreate = nullptr;
ObjectScript EnemyRedpompCreate = nullptr;

ObjectScript EnemyStep = nullptr;
ObjectScript EnemyCopStep = nullptr;
ObjectScript EnemyMeeleStep = nullptr;
ObjectScript EnemyGruntStep = nullptr;
ObjectScript EnemySciStep = nullptr;
ObjectScript EnemyShieldcopStep = nullptr;
ObjectScript EnemyShotgunStep = nullptr;
ObjectScript EnemyMachinegunStep = nullptr;
ObjectScript EnemyRedpompStep = nullptr;

ObjectScript EnemyBikeCreate = nullptr;
ObjectScript EnemyBikeBullyCreate = nullptr;
ObjectScript EnemyBikeMachinegunCreate = nullptr;

int *Current_Room;

// IHATEITHEREIHATEITHEREIHATEITHERE
// IHATEITHEREIHATEITHEREIHATEITHERE
#define Hook_CreateScript(OriginalFunction)                                    \
void Hook_##OriginalFunction(CInstance *self, CInstance *other)                \
{                                                                              \
	int hp;                                                                    \
	if (*Current_Room == (int)GameRooms::room_factory_0 ||					   \
		*Current_Room == (int)GameRooms::room_tiedup)                          \
		hp = 1;                                                                \
	else                                                                       \
		hp = 2;                                                                \
	dcout << "[Hook] " << #OriginalFunction << " is run, id=" << self->i_id << "\n"; \
	YYStackTrace::PrintStackTrace(*YYStackTrace::global_trace);				   \
	KZSDT::enemies[self->i_id] = KZSDT::Enemy(self, hp);                       \
	                                                                           \
	OriginalFunction(self, other);                                             \
}

Hook_CreateScript(EnemyCreate);
Hook_CreateScript(EnemyCopCreate);
Hook_CreateScript(EnemyMeeleCreate);
Hook_CreateScript(EnemyGruntCreate);
Hook_CreateScript(EnemySciCreate);
Hook_CreateScript(EnemyShieldcopCreate);
Hook_CreateScript(EnemyShotgunCreate);
Hook_CreateScript(EnemyMachinegunCreate);
Hook_CreateScript(EnemyRedpompCreate);


#define Hook_StepScript(OriginalFunction)																			 \
void Hook_##OriginalFunction(CInstance *self, CInstance *other)														 \
{																													 \
	KZSDT::Enemy &enemy = KZSDT::enemies.at(self->i_id);															 \
																													 \
	if (enemy.dragon_dashed) {																						 \
		RValue player = InstanceFindWrapper(self, other, (int)GameObjects::obj_player, 0);							 \
		int player_id = (int)player.u.v;																			 \
																													 \
		if (player_id != -4) {																						 \
			RValue stopattack_timer = VariableInstanceGetWrapper(self, other, player_id, (char*)"stopattack_timer"); \
			if (stopattack_timer.u.v <= 1e-10) {																	 \
				enemy.dragon_dashed = false;																		 \
				dcout << "Enemy " << self->i_id << " is undragon-dashed.\n";										 \
			}																										 \
		}																											 \
	}																												 \
	OriginalFunction(self, other);																					 \
}

Hook_StepScript(EnemyStep);
Hook_StepScript(EnemyCopStep);
Hook_StepScript(EnemyMeeleStep);
Hook_StepScript(EnemyGruntStep);
Hook_StepScript(EnemySciStep);
Hook_StepScript(EnemyShieldcopStep);
Hook_StepScript(EnemyShotgunStep);
Hook_StepScript(EnemyMachinegunStep);
Hook_StepScript(EnemyRedpompStep);

#define Hook_BikeCreateScript(OriginalFunction)																		 \
void Hook_##OriginalFunction(CInstance *self, CInstance *other)														 \
{																													 \
	KZSDT::enemies[self->i_id] = KZSDT::Enemy(self, 2);															     \
	OriginalFunction(self, other);																					 \
}

Hook_BikeCreateScript(EnemyBikeCreate);
Hook_BikeCreateScript(EnemyBikeBullyCreate);
Hook_BikeCreateScript(EnemyBikeMachinegunCreate);

#include "Manipulation.h"
using namespace Manipulation;


void InitializeHooks()
{
	Current_Room = reinterpret_cast<int*>(Global::exe_base+0x1cf8900u);

	std::vector<std::tuple<uintptr_t, LPVOID, ObjectScript*>> create_events {
		{ Global::exe_base + 0xc87cb0u, Hook_EnemyCreate, &EnemyCreate },
		{ Global::exe_base + 0xc9d510u, Hook_EnemyCopCreate, &EnemyCopCreate },
		{ Global::exe_base + 0xcb1cb0u, Hook_EnemyMeeleCreate, &EnemyMeeleCreate },
		{ Global::exe_base + 0xcc2be0u, Hook_EnemyGruntCreate, &EnemyGruntCreate },
		{ Global::exe_base + 0xcd37e0u, Hook_EnemySciCreate, &EnemySciCreate },
		{ Global::exe_base + 0xce25c0u, Hook_EnemyShieldcopCreate, &EnemyShieldcopCreate },
		{ Global::exe_base + 0xcf5030u, Hook_EnemyShotgunCreate, &EnemyShotgunCreate },
		{ Global::exe_base + 0xd0a1f0u, Hook_EnemyMachinegunCreate, &EnemyMachinegunCreate },
		{ Global::exe_base + 0xd1a920u, Hook_EnemyRedpompCreate, &EnemyRedpompCreate },
		{ Global::exe_base + 0xc88ef0u, Hook_EnemyStep, &EnemyStep },
		{ Global::exe_base + 0xc9e7a0u, Hook_EnemyCopStep, &EnemyCopStep },
		{ Global::exe_base + 0xcb2f30u, Hook_EnemyMeeleStep, &EnemyMeeleStep },
		{ Global::exe_base + 0xcc4030u, Hook_EnemyGruntStep, &EnemyGruntStep },
		{ Global::exe_base + 0xcd4650u, Hook_EnemySciStep, &EnemySciStep },
		{ Global::exe_base + 0xce3750u, Hook_EnemyShieldcopStep, &EnemyShieldcopStep },
		{ Global::exe_base + 0xcf6190u, Hook_EnemyShotgunStep, &EnemyShotgunStep },
		{ Global::exe_base + 0xd0b140u, Hook_EnemyMachinegunStep, &EnemyMachinegunStep },
		{ Global::exe_base + 0xd1bdb0u, Hook_EnemyRedpompStep, &EnemyRedpompStep },
		{ Global::exe_base + 0x138a130u, Hook_EnemyBikeCreate, &EnemyBikeCreate },
		{ Global::exe_base + 0x13afb30u, Hook_EnemyBikeBullyCreate, &EnemyBikeBullyCreate },
		{ Global::exe_base + 0x13a2f90u, Hook_EnemyBikeMachinegunCreate, &EnemyBikeMachinegunCreate },
	};

	MH_STATUS result;
	result = MH_CreateHook(reinterpret_cast<LPVOID>(Global::exe_base + 0x154cd10u), Hook_EndRoom, reinterpret_cast<LPVOID *>(&OriginalEndRoom));
	if (result != MH_OK)
		abort();
	if (result != MH_OK)
		abort();

	for (auto event : create_events) {
		uintptr_t address = std::get<0>(event);
		LPVOID hook = std::get<1>(event);
		ObjectScript *trampoline = std::get<2>(event);

		result = MH_CreateHook(reinterpret_cast<LPVOID>(address), static_cast<LPVOID>(hook), reinterpret_cast<LPVOID *>(trampoline));
		if (result != MH_OK)
			dcout << "Failed hook to " << std::hex << address << std::dec << ", error code " << result << '\n';
		else
			dcout << "Hooked " << std::hex << address << std::dec << '\n';
	}
}