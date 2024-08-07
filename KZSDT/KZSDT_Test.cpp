#include "KZSDT_Test.h"

#include "Debug.h"
#include "Global.h"

#include "GameMaker.h"
#include "KatanaZero.h"
#include "Manipulation.h"

#include "minhook/include/MinHook.h"

using namespace GameMaker;
using namespace Manipulation;

typedef void(_cdecl *DoInstanceDestroy_t)(CInstance *, CInstance *, int, bool);
DoInstanceDestroy_t trampoline_DoInstanceDestroy;


void Hook_DoInstanceDestroy(CInstance *self, CInstance *other, int id, bool execute_flag)
{
	if (true) {
		dcout << "[InstanceDestroy] Object " << id << ":\n";
		YYStackTrace::PrintStackTrace(*YYStackTrace::global_trace);
	}

	trampoline_DoInstanceDestroy(self, other, id, execute_flag);
}

typedef void(_cdecl *YYGML_instance_destroy_t)(CInstance *, CInstance *, int, RValue **);
YYGML_instance_destroy_t trampoline_YYGML_instance_destroy;

void Hook_YYGML_instance_destroy(CInstance *self, CInstance *other, int arg_count, RValue **args)
{
	try {
		switch (self->i_object_index)
		{
			case (int)GameObjects::obj_titlerain:
			case (int)GameObjects::obj_motion_trail:
			case (int)GameObjects::obj_dustcloud:
			case (int)GameObjects::obj_blood:
			case (int)GameObjects::obj_sprite_trail:
			case (int)GameObjects::obj_hitflash2:
			case (int)GameObjects::obj_sound:
			case (int)GameObjects::obj_bike_motion_trail:
				break;
			default:
				dcout << "[InstanceDestroy] Self: obj=" << self->i_object_index << ", id=" << self->i_id << ' ';
				dcout << "Other: obj=" << other->i_object_index << ", id=" << other->i_id;
				if (arg_count > 0) {
					dcout << " Args: ";
					for (int i = 0; i < arg_count; i++) {
						RValue *arg = args[i];
						dcout << i << '=' << arg->u.v;
						if (i != arg_count - 1)
							dcout << ", ";
					}
				}
				dcout << ":\n";
				YYStackTrace::PrintStackTrace(*YYStackTrace::global_trace);
		}
	}
	catch (std::exception &e)
	{
		dcout << e.what() << std::endl;
	}

	trampoline_YYGML_instance_destroy(self, other, arg_count, args);
}


typedef void(_cdecl *YYGML_instance_create_depth_t)(RValue*, CInstance*, CInstance*, int, RValue*);
YYGML_instance_create_depth_t trampoline_YYGML_instance_create_depth;

void Hook_YYGML_instance_create_depth(RValue *result, CInstance *self, CInstance *other, int arg_count, RValue *args)
{
	try {
		switch ((int)args[3].u.v)
		{
			case (int)GameObjects::obj_cigsmoke:
			case (int)GameObjects::obj_cigarette:
				dcout << "[instance_create_depth] Self: obj=" << self->i_object_index << ", id=" << self->i_id << ' ';
				dcout << "Other: obj=" << other->i_object_index << ", id=" << other->i_id;
				if (arg_count > 0) {
					dcout << " Args: ";
					for (int i = 0; i < arg_count; i++) {
						RValue arg = args[i];
						dcout << i << '=' << arg.u.v;
						if (i != arg_count - 1)
							dcout << ", ";
					}
				}
				dcout << ":\n";
				YYStackTrace::PrintStackTrace(*YYStackTrace::global_trace);
		}
	}
	catch (std::exception &e)
	{
		dcout << e.what() << std::endl;
	}

	trampoline_YYGML_instance_create_depth(result, self, other, arg_count, args);
}

typedef void(*ObjectScript)(CInstance *, CInstance *);
ObjectScript obj_killer_event_Create;

void Hook_obj_killer_event_Create(CInstance *self, CInstance *other)
{
	dcout << "[obj_killer_event.Create] killer event created.\n";
	YYStackTrace::PrintStackTrace(*YYStackTrace::global_trace);

	obj_killer_event_Create(self, other);
}

ObjectScript obj_staircase_gangster_Create;

void Hook_obj_staircase_gangster_Create(CInstance *self, CInstance *other)
{
	dcout << "[obj_staircase_gangster.Create] obj_staircase_gangster created.\n";
	YYStackTrace::PrintStackTrace(*YYStackTrace::global_trace);

	obj_staircase_gangster_Create(self, other);
}

typedef int(__cdecl *Sprite_Add_t)(char*, int, bool, bool, bool, bool, int, int);
Sprite_Add_t trampoline_Sprite_Add;

int __cdecl Hook_Sprite_Add(char *param_1, int param_2, bool param_3, bool param_4, bool param_5, bool param_6, int param_7, int param_8)
{
	dcout << "[SpriteAdd] " << param_1 << ' ' << param_2 << ' ' << param_3 << ' ' << param_4 << ' ' << param_5 << ' ' << param_6 << ' ' << param_7 << ' ' << param_8 << '\n';
	return trampoline_Sprite_Add(param_1, param_2, param_3, param_4, param_5, param_6, param_7, param_8);
}


void InitializeTests()
{
	MH_STATUS result;
	result = MH_CreateHook(reinterpret_cast<LPVOID>(Global::exe_base + 0x14e48f0u),
									 Hook_YYGML_instance_destroy,
									 reinterpret_cast<LPVOID *>(&trampoline_YYGML_instance_destroy));
	Hooks::MinHook_Assert(result);
	Hooks::MinHook_Assert(MH_EnableHook((LPVOID)(Global::exe_base + 0x14e48f0u)));

	result = MH_CreateHook(reinterpret_cast<LPVOID>(Global::exe_base + 0x14e07a0u),
						   Hook_YYGML_instance_create_depth,
						   reinterpret_cast<LPVOID *>(&trampoline_YYGML_instance_create_depth));
	Hooks::MinHook_Assert(result);
	Hooks::MinHook_Assert(MH_EnableHook((LPVOID)(Global::exe_base + 0x14e07a0u)));

	result = MH_CreateHook(
		reinterpret_cast<LPVOID>(Global::exe_base + 0xab0790u),
		Hook_obj_killer_event_Create,
		reinterpret_cast<LPVOID *>(&obj_killer_event_Create)
	);
	Hooks::MinHook_Assert(result);
	Hooks::MinHook_Assert(MH_EnableHook((LPVOID)(Global::exe_base + 0xab0790u)));

	result = MH_CreateHook(reinterpret_cast<LPVOID>(Global::exe_base + 0xb74d70u),
									 Hook_obj_staircase_gangster_Create,
									 reinterpret_cast<LPVOID *>(&obj_staircase_gangster_Create));
	Hooks::MinHook_Assert(result);
	Hooks::MinHook_Assert(MH_EnableHook((LPVOID)(Global::exe_base + 0xb74d70u)));

	result = MH_CreateHook(reinterpret_cast<LPVOID>(Global::exe_base + 0x153caf0u),
									 Hook_Sprite_Add,
									 reinterpret_cast<LPVOID *>(&trampoline_Sprite_Add));
	Hooks::MinHook_Assert(result);
	Hooks::MinHook_Assert(MH_EnableHook((LPVOID)(Global::exe_base + 0x153caf0u)));


	dcout << "Initialized tests\n";
}
