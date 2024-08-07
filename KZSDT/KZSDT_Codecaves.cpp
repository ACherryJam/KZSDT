#include "KZSDT_Codecaves.h"

#include <map>
#include <vector>
#include <iostream>
#include <Windows.h>

#include "Debug.h"
#include "Global.h"

#include "KZSDT_Enemy.h"
#include "GameMaker.h"
#include "Manipulation.h"
#include "KatanaZero.h"

#define MH_ASSERT(status) if (status != MH_OK) abort();

using namespace GameMaker;
using namespace Manipulation;


DWORD ReturnAddress;

typedef void(_cdecl *YYGML_instance_destroy_t)(CInstance *, CInstance *, int, RValue **);
YYGML_instance_destroy_t YYGML_instance_destroy;

RValue *VariableInstanceSetWrapper(CInstance *self, CInstance *other, int inst_id, char *name, RValue value)
{
	RValue *result = new RValue;

	RValue *args = new RValue[3] {};
	args[0].m_flags = 0;
	args[0].m_kind = VALUE_REAL;
	args[0].u.v = inst_id;

	args[1].m_flags = 0;
	args[1].m_kind = VALUE_STRING;

	GMStringRef *str = new GMStringRef;
	str->m_refCount = 0;
	str->m_size = strlen(name);
	str->m_pString = name;
	args[1].u.s = str;

	args[2] = value;

	VariableInstanceSet(result, self, other, 3, args);
	
	delete[] args;
	delete str;

	return result;
}


#define ProjectileHandlingAddress   Global::exe_base+0x1c4bfbu
#define ExplosionHandlingAddress    Global::exe_base+0x1c5405u
#define FlamethrowerHandlingAddress Global::exe_base+0x1c944bu
#define ExitScriptAddress           Global::exe_base+0x1ccc15u

CInstance *g_self, *g_other;

void HandleSlashHit()
{
	KZSDT::Enemy &enemy = KZSDT::enemies.at(g_self->i_id);

	RValue &hit = g_self->GetYYVarRef(0x5ae);
	InstanceId hit_id = (int)hit.u.v;
	
	bool register_hit = !enemy.dragon_dashed;
	
	if (register_hit) {
		enemy.Hurt(hit_id);
	}

	if (enemy.GetHP() > 0) {
		dcout << "[HandleSlashHit] Jumping to projectile check\n";
		ReturnAddress = ProjectileHandlingAddress;
	}
	else {
		dcout << "[HandleSlashHit] Enemy killed by slash.\n";
	}	
}

void HandleBulletHit()
{
	KZSDT::Enemy &enemy = KZSDT::enemies.at(g_self->i_id);

	RValue &hit = g_self->GetYYVarRef(0x5ae);  // 0x5ae is index for "hit" variable
	InstanceId hit_id = (int)hit.u.v;

	bool register_hit = true;
	
	RValue object_index = VariableInstanceGetWrapper(g_self, g_other, hit_id, (char *)"object_index");
	int object_index_value = (int)object_index.u.v;

	int target_value = -1;

	dcout << "[HandleBulletHit] hit object_index = " << object_index_value << '\n';

	switch (object_index_value)
	{
		case (int)GameObjects::obj_killer_event:
			{
				RValue target = VariableInstanceGetWrapper(g_self, g_other, hit_id, (char*)"target");
				target_value = (int)target.u.v;

				RValue parent = VariableInstanceGetWrapper(g_self, g_other, hit_id, (char *)"parent");
				InstanceId parent_id = (int)parent.u.v;

				RValue dragon_dash_state = VariableInstanceGetWrapper(g_self, g_other, parent_id, (char*)"dragon_dash_state");
				dcout << "dragon_dash_state = " << dragon_dash_state.u.v << ' ' << dragon_dash_state.m_kind << '\n';
				if (dragon_dash_state.m_kind != VALUE_UNDEFINED && dragon_dash_state.u.v == 2.0) {
					enemy.dragon_dashed = true;
					dcout << "Enemy " << g_self->i_id << " is dragon-dashed.\n";
				}

				dcout << "[HandleBulletHit] hit target=" << target_value << '\n';

				if (target_value != g_self->i_id)
					break;
			}
			[[fallthrough]];
		default:
			enemy.Hurt(hit_id);
	}


	if (enemy.GetHP() > 0) {
		if (object_index_value != (int)GameObjects::obj_killer_event) {
			InstanceDestroyWrapper(g_self, g_other, hit_id);
		}
		else {
			if (target_value == g_self->i_id)
				InstanceDestroyWrapper(g_self, g_other, hit_id, false);
		}

		dcout << "[HandleBulletHit] Jumping to explosion check\n";
		ReturnAddress = ExplosionHandlingAddress;
	}
	else {
		dcout << "[HandleSlashHit] Enemy killed by projectile.\n";
	}
}

void HandleExplosionHit()
{
	RValue &hit = g_self->GetYYVarRef(0x5ae);  // 0x5ae is index for "hit" variable
	InstanceId hit_id = (int)hit.u.v;

	try {
		KZSDT::Enemy &enemy = KZSDT::enemies.at(g_self->i_id);

		enemy.Hurt(hit_id);
		if (enemy.GetHP() > 0) {
			dcout << "[HandleExplosionHit] Jumping to flamethrower check\n";
			ReturnAddress = FlamethrowerHandlingAddress;
		}
		else {
			dcout << "[HandleSlashHit] Enemy killed by explosion.\n";
		}
	}
	catch (std::exception &e) {
		dcout << "HP SYSTEM FAIL => " << e.what() << '\n';
	}
}

void HandleFlamethrowerHit()
{
	RValue &hit = g_self->GetYYVarRef(0x5ae);  // 0x5ae is index for "hit" variable
	InstanceId hit_id = (int)hit.u.v;
	
	try {
		KZSDT::Enemy &enemy = KZSDT::enemies.at(g_self->i_id);

		enemy.Hurt(hit_id);
		if (enemy.GetHP() > 0) {
			RValue &heat = g_self->GetYYVarRef(0x599);
			heat.u.v = 0;

			dcout << "[HandleFlamethrowerHit] Exiting the script\n";
			ReturnAddress = ExitScriptAddress;
		}
		else {
			dcout << "[HandleSlashHit] Enemy killed by flames.\n";
		}
	}
	catch (std::exception &e) {
		dcout << "HP SYSTEM FAIL => " << e.what() << '\n';
	}
}


__declspec(naked) void CC_HandleSlashHit(void)
{
	__asm
	{
		pop ReturnAddress

		PUSHAD
		PUSHFD
	}

	HandleSlashHit();

	__asm
	{
		POPFD
		POPAD

		mov dword ptr[ESP + 0x139c], 0x5

		push ReturnAddress
		ret
	}
}


__declspec(naked) void CC_HandleBulletHit(void)
{
	__asm
	{
		pop ReturnAddress

		PUSHAD
		PUSHFD
	}

	HandleBulletHit();

	__asm
	{
		POPFD
		POPAD

		mov dword ptr[ESP + 0x13bc], 0x5

		push ReturnAddress
		ret
	}
}

__declspec(naked) void CC_HandleExplosionHit(void)
{
	__asm
	{
		pop ReturnAddress

		PUSHAD
		PUSHFD
	}

	HandleExplosionHit();

	__asm
	{
		POPFD
		POPAD

		mov dword ptr[ESP + 0x13bc], 0x5

		push ReturnAddress
		ret
	}
}

__declspec(naked) void CC_HandleFlamethrowerHit(void)
{
	__asm
	{
		pop ReturnAddress

		PUSHAD
		PUSHFD
	}

	HandleFlamethrowerHit();

	__asm
	{
		POPFD
		POPAD

		;mov dword ptr[ESP + 0x1c48], 0x1e0
		mov ECX, dword ptr[ESP + 0x1c4c]

		push ReturnAddress
		ret
	}
}

void obj_staircase_gangster_Other_17()
{
	try {
		RValue &bb = g_other->InternalGetYYVarRef(0xee);
		InstanceId bb_id = (int)bb.u.v;

		dcout << "[staircase] bb_id = " << bb_id << ", self=" << g_self->i_id << ", other=" << g_other->i_id << '\n';

		InstanceId original_enemy_id = KZSDT::staircase_gangster_relations.at(g_self->i_id);
		KZSDT::staircase_gangster_relations.erase(g_self->i_id);

		KZSDT::Enemy &enemy = KZSDT::enemies.at(original_enemy_id);
		KZSDT::enemies.erase(g_self->i_id);

		KZSDT::enemies[bb_id] = enemy;
	}
	catch (std::exception &e) {
		dcout << "[obj_staircase_gangster_Other_17] " << e.what() << '\n';
	}
}

void ai_check_stairdoor()
{
	try {
		RValue &bb = g_self->InternalGetYYVarRef(0xee);
		InstanceId bb_id = (int)bb.u.v;

		dcout << "[ai_check] bb_id = " << bb_id << ", self = " << g_self->i_id << '\n';

		KZSDT::staircase_gangster_relations[bb_id] = g_self->i_id;
	}
	catch (std::exception &e) {
		dcout << "[ai_check] " << e.what() << '\n';
	}
}

__declspec(naked) void CC_ai_check_stairdoor(void)
{
	__asm
	{
		pop ReturnAddress

		PUSHAD
		PUSHFD
	}

	ai_check_stairdoor();

	__asm
	{
		POPFD
		POPAD

		mov dword ptr [ESP + 0x288], 0x8


		push ReturnAddress
		ret
	}
}

__declspec(naked) void CC_obj_staircase_gangster_Other_17(void)
{
	__asm
	{
		pop ReturnAddress

		PUSHAD
		PUSHFD
	}

	obj_staircase_gangster_Other_17();

	__asm
	{
		POPFD
		POPAD

		mov dword ptr[ESP + 0x340], 0xa3

		push ReturnAddress
		ret
	}
}

typedef void(*OtherEvent)(CInstance*, CInstance*);
typedef RValue*(*GMLScript)(CInstance*, CInstance*, RValue*, int, RValue** args);

GMLScript trampoline_EnemyHurt;
RValue *Hook_EnemyHurt(CInstance *self, CInstance *other, RValue *result, int arg_count, RValue **args)
{
	g_self = self;
	g_other = other;

	return trampoline_EnemyHurt(self, other, result, arg_count, args);
}

GMLScript trampoline_AICheckStairdoor;
RValue *Hook_AICheckStairdoor(CInstance *self, CInstance *other, RValue *result, int arg_count, RValue **args)
{
	g_self = self;
	g_other = other;

	return trampoline_AICheckStairdoor(self, other, result, arg_count, args);
}

OtherEvent trampoline_obj_staircase_gangster_Other_17;
void Hook_obj_staircase_gangster_Other_17(CInstance *self, CInstance *other)
{
	g_self = self;
	g_other = other;

	trampoline_obj_staircase_gangster_Other_17(self, other);
}


void InitializeCodecaves()
{
	YYGML_instance_destroy = reinterpret_cast<YYGML_instance_destroy_t>(Global::exe_base + 0x14e48f0u);

	MH_STATUS result;
	result = MH_CreateHook(reinterpret_cast<LPVOID>(Global::exe_base+0x1bf7c0u), Hook_EnemyHurt, reinterpret_cast<LPVOID*>(&trampoline_EnemyHurt));
	MH_ASSERT(result);
	result = MH_CreateHook(reinterpret_cast<LPVOID>(Global::exe_base+0x1e4400u), Hook_AICheckStairdoor, reinterpret_cast<LPVOID*>(&trampoline_AICheckStairdoor));
	MH_ASSERT(result);
	result = MH_CreateHook(reinterpret_cast<LPVOID>(Global::exe_base+0xb76660u), Hook_obj_staircase_gangster_Other_17, reinterpret_cast<LPVOID*>(&trampoline_obj_staircase_gangster_Other_17));
	MH_ASSERT(result);
	//MH_ASSERT( MH_EnableHook((LPVOID)(ExeBase + 0x1bf7c0u)) )
	//MH_ASSERT( MH_EnableHook((LPVOID)(ExeBase + 0x1e4400u)) )
	//MH_ASSERT( MH_EnableHook((LPVOID)(ExeBase + 0xb76660u)) )

	Codecave(Global::exe_base + 0x1c2594u, CC_HandleSlashHit, 6);
	Codecave(Global::exe_base + 0x1c5156u, CC_HandleBulletHit, 6);
	Codecave(Global::exe_base + 0x1c6e99u, CC_HandleExplosionHit, 6);
	Codecave(Global::exe_base + 0x1caf6eu, CC_HandleFlamethrowerHit, 2);

	Codecave(Global::exe_base + 0x1e4e4fu, CC_ai_check_stairdoor, 6);
	Codecave(Global::exe_base + 0xb79687u, CC_obj_staircase_gangster_Other_17, 6);
}