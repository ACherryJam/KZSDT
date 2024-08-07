#include "KZSDT_Enemy.h"

#include <algorithm>

#include "Debug.h"


using namespace KZSDT;

std::map<InstanceId, Enemy> KZSDT::enemies = {};
std::map<InstanceId, InstanceId> KZSDT::staircase_gangster_relations {};


Enemy::Enemy(GameMaker::CInstance *_inst, int _hp)
	: inst(_inst), hp(_hp)
{
	
}

void Enemy::Hurt(InstanceId inst_id)
{
	bool inst_found = std::find(hit_inst.begin(), hit_inst.end(), inst_id) != hit_inst.end();

	if (inst_found)
		return;

	hp--;
	hit_inst.push_back(inst_id);

	dcout << "[Enemy::Hurt] inst=" << inst->i_id << ", hit=" << inst_id << '\n';
}
