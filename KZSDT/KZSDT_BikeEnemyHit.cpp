#include "Global.h"

#include "KZSDT_Enemy.h"
#include "GameMaker.h"
#include "Manipulation.h"

#define BikeBulletExitAddress Global::exe_base+0x13991b7u

using namespace GameMaker;

namespace {
	DWORD ReturnAddress;
	CInstance *g_self, *g_other;
}

typedef void(*CreateEvent_t)(CInstance *, CInstance *);
CreateEvent_t trampoline_obj_enemybike_Step = nullptr;

void Hook_obj_enemybike_Step(CInstance *self, CInstance *other)
{
	g_self = self;
	g_other = other;

	trampoline_obj_enemybike_Step(self, other);
}


constexpr auto HIT = 0x5ae;
void HandleBikeBulletHit()
{
	KZSDT::Enemy &enemy = KZSDT::enemies.at(g_self->i_id);

	RValue &hit = g_self->GetYYVarRef(HIT);
	InstanceId hit_id = (int)hit.u.v;

	enemy.Hurt(hit_id);

	if (enemy.GetHP() > 0) {
		InstanceDestroyWrapper(g_self, g_other, hit_id);

		ReturnAddress = BikeBulletExitAddress;
	}
	else {
		dcout << "[HandleBikeBulletHit] Enemy killed by bullet.";
	}
}

__declspec(naked) void CC_HandleBikeBulletHit(void)
{
	__asm
	{
		pop ReturnAddress

		PUSHAD
		PUSHFD
	}

	HandleBikeBulletHit();

	__asm
	{
		POPFD
		POPAD

		mov dword ptr[ESP + 0x14c8], 0x265

		push ReturnAddress
		ret
	}
}

typedef RValue *(*GMLScript)(CInstance *, CInstance *, RValue *, int, RValue **args);
GMLScript trampoline_check_bikeslash_collision = nullptr;

RValue *Hook_check_bikeslash_collision(CInstance *self, CInstance *other, RValue *result, int arg_count, RValue **args)
{
	RValue *r = trampoline_check_bikeslash_collision(self, other, result, arg_count, args);

	if (KZSDT::enemies.find(self->i_id) != KZSDT::enemies.end()) {
		KZSDT::Enemy &enemy = KZSDT::enemies.at(self->i_id);

		if (r->u.v == true) {
			enemy.Hurt(args[0]->u.v);
			if (enemy.GetHP() > 0) {
				r->u.v = 0;
			}
		}
	}

	dcout << "check_bikeslash_collision result: " << r->u.v << '\n';

	return r;
}


void InitializeBikeEnemyHitHandling()
{
	MH_STATUS result;

	result = MH_CreateHook(
		reinterpret_cast<LPVOID>(Global::exe_base + 0x138caf0u),
		Hook_obj_enemybike_Step,
		reinterpret_cast<LPVOID *>(&trampoline_obj_enemybike_Step)
	);
	Manipulation::Hooks::MinHook_Assert(result);

	result = MH_CreateHook(reinterpret_cast<LPVOID>(Global::exe_base + 0x40d0f0u),
						   Hook_check_bikeslash_collision, reinterpret_cast<LPVOID *>(&trampoline_check_bikeslash_collision));
	Manipulation::Hooks::MinHook_Assert(result);

	Manipulation::Codecave(Global::exe_base + 0x1397932u, CC_HandleBikeBulletHit, 6);
}