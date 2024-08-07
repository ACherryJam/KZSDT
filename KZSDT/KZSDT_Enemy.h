#pragma once

#include <map>

#include "GameMaker.h"

typedef int InstanceId;

namespace KZSDT
{
	class Enemy
	{
	public:
		bool dragon_dashed = false;

		Enemy() = default;
		Enemy(GameMaker::CInstance *_inst, int _hp);
		
		void Hurt(InstanceId inst_id);

		int GetHP() { return hp; };

	private:
		int hp = -1;
		GameMaker::CInstance *inst = nullptr;

		std::vector<InstanceId> hit_inst {};
	};

	extern std::map<InstanceId, Enemy> enemies;
	extern std::map<InstanceId, InstanceId> staircase_gangster_relations;
}