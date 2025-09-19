#include "cbase.h"

#ifdef CLIENT_DLL

#include "achievementmgr.h"
#include "baseachievement.h"
#include "convar.h"  // Add this include
#include "usermessages.h"  // Add this
#include "tier1/bitbuf.h"  // Add this
#include "saverestore.h"        // Add this
#include "saverestoretypes.h"   // Add this

CAchievementMgr g_AchievementMgrMod; // Global achievement mgr for mod

// Add forward declaration here:
void __MsgFunc_ZombieKilled(bf_read& msg);
#define ACHIEVEMENT_MOD_HIT_TRIGGER 1
#define ACHIEVEMENT_MOD_GOT_SECRET_WEAPON 2
#define ACHIEVEMENT_MOD_GOT_COP_KILLS 3
#define ACHIEVEMENT_MOD_KILL_BIRDIE 4
#define ACHIEVEMENT_MOD_FIND_BATTERY 5
#define ACHIEVEMENT_MOD_JUMP_GAP 6
#define ACHIEVEMENT_MOD_BOX_BREAKER 7
#define ACHIEVEMENT_MOD_MYSTERY_BOX 8
#define ACHIEVEMENT_MOD_POWER_TOWER 9
#define ACHIEVEMENT_MOD_CODE_TOWER 10
#define ACHIEVEMENT_MOD_LOCK_BUSTER 11
#define ACHIEVEMENT_MOD_GREAT_ESCAPE 12
#define ACHIEVEMENT_MOD_HOLY_BREAD 13
#define ACHIEVEMENT_MOD_FIRE_CRACKER 14
#define ACHIEVEMENT_MOD_IAM_DEVELOPER 15
#define ACHIEVEMENT_MOD_TEAM_DEATHMATCH 16
#define ACHIEVEMENT_MOD_HELLO_OPERATOR 17
#define ACHIEVEMENT_MOD_CONTROL_GAINER 18
#define ACHIEVEMENT_MOD_CONTROL_GAINER2 19
#define ACHIEVEMENT_MOD_SPECIAL_DELIVERY 20
#define ACHIEVEMENT_MOD_GRAVE_DIGGING 21
#define ACHIEVEMENT_MOD_GNOME_HOME 22
#define ACHIEVEMENT_MOD_GNOME_ALONE 23
#define ACHIEVEMENT_MOD_LITTLE_DIGGER 24
#define ACHIEVEMENT_MOD_BOMB_CARRIER 25
#define ACHIEVEMENT_MOD_BOMB_DELIVERY 26
#define ACHIEVEMENT_MOD_ROPE_HOTEL 27
#define ACHIEVEMENT_MOD_PLAY_HOTEL 28
#define ACHIEVEMENT_MOD_FIRE_HOTEL 29
#define ACHIEVEMENT_MOD_PLAY_DEATHMATCH 30
#define ACHIEVEMENT_MOD_MOVIN_UP 31
#define ACHIEVEMENT_MOD_BEAR_TRAP 32
#define ACHIEVEMENT_MOD_REIGN_ABOVE 33
#define ACHIEVEMENT_MOD_CAUGHT_SPOTLIGHT 34
#define ACHIEVEMENT_MOD_CAT_CHASE 35
#define ACHIEVEMENT_MOD_CAT_KILL 36
#define ACHIEVEMENT_MOD_FIRING_SQUAD 37
#define ACHIEVEMENT_MOD_AIM_SMALL 38
#define ACHIEVEMENT_MOD_DUMPSTER_DIVER 39
#define ACHIEVEMENT_MOD_POP_WEASEL 40
#define ACHIEVEMENT_MOD_STORY_BEGINS 41
#define ACHIEVEMENT_MOD_BODY_BAG 42
#define ACHIEVEMENT_MOD_REPEAT_OFFENDER 43
#define ACHIEVEMENT_MOD_TARGET_SHOOTER 44
#define ACHIEVEMENT_MOD_BUCK_HUNTER 45
#define ACHIEVEMENT_MOD_DUCK_HUNTER 46
#define ACHIEVEMENT_MOD_GOT_COP_KILLS2 47
#define ACHIEVEMENT_MOD_GOT_COP_KILLS3 48
#define ACHIEVEMENT_MOD_TRAINING_DAY 49
#define ACHIEVEMENT_MOD_GOT_COP_KILLS4 50
#define ACHIEVEMENT_MOD_GOT_COP_KILLS5 51
#define ACHIEVEMENT_MOD_GOT_COP_KILLS6 52
#define ACHIEVEMENT_MOD_GOT_ZOP_KILLS 53
#define ACHIEVEMENT_MOD_GOT_POP_KILLS 54
#define ACHIEVEMENT_MOD_GOT_TOP_KILLS 55
#define ACHIEVEMENT_MOD_GOT_ZOP_KILLS2 56
#define ACHIEVEMENT_MOD_GOT_ZOP_KILLS3 57
#define ACHIEVEMENT_MOD_GOT_ZOP_KILLS4 58
#define ACHIEVEMENT_MOD_GOT_ZOP_KILLS5 59
#define ACHIEVEMENT_MOD_GOT_ZOP_KILLS6 60
#define ACHIEVEMENT_MOD_GOT_POP_KILLS2 61
#define ACHIEVEMENT_MOD_GOT_POP_KILLS3 62
#define ACHIEVEMENT_MOD_GOT_POP_KILLS4 63
#define ACHIEVEMENT_MOD_GOT_POP_KILLS5 64
#define ACHIEVEMENT_MOD_GOT_POP_KILLS6 65
#define ACHIEVEMENT_MOD_GOT_TOP_KILLS2 66
#define ACHIEVEMENT_MOD_GOT_TOP_KILLS3 67
#define ACHIEVEMENT_MOD_GOT_TOP_KILLS4 68
#define ACHIEVEMENT_MOD_GOT_TOP_KILLS5 69
#define ACHIEVEMENT_MOD_GOT_TOP_KILLS6 70
#define ACHIEVEMENT_MOD_GOT_BOP_KILLS 71
#define ACHIEVEMENT_MOD_GOT_BOP_KILLS2 72
#define ACHIEVEMENT_MOD_GOT_BOP_KILLS3 73
#define ACHIEVEMENT_MOD_GOT_BOP_KILLS4 74
#define ACHIEVEMENT_MOD_GOT_BOP_KILLS5 75
#define ACHIEVEMENT_MOD_GOT_BOP_KILLS6 76
#define ACHIEVEMENT_MOD_HOOLIGAN_TOOLERY 77
#define ACHIEVEMENT_MOD_HOOLIGAN_TOOLERY2 78
#define ACHIEVEMENT_MOD_HOOLIGAN_TOOLERY3 79
#define ACHIEVEMENT_MOD_HOOLIGAN_TOOLERY4 80
#define ACHIEVEMENT_MOD_HOOLIGAN_TOOLERY5 81
#define ACHIEVEMENT_MOD_HOOLIGAN_TOOLERY6 82

ConVar zombie_kills("zombie_kills", "0", FCVAR_ARCHIVE);

// Storyline get 6 zombie kills achievement
class CAchievementModCopKills : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(6);

		// Restore progress from Steam stat
		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 zombieKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("zombie_kills", &zombieKills))
			{
				SetCount(zombieKills);
				Msg("[Achievement] Restored zombie kills from Steam: %d/%d\n", zombieKills, GetGoal());
			}
		}
	}

	void HandleZombieKill()
	{
		if (!IsAchieved())
		{
			// Increment both the achievement and Steam stat
			IncrementCount();

			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("zombie_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Zombie kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

DECLARE_ACHIEVEMENT(CAchievementModCopKills, ACHIEVEMENT_MOD_GOT_COP_KILLS, "MOD_GOT_COP_KILLS", 5);

// Storyline get 69 zombie kills achievement

class CAchievementModPopWeasel : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(69);

		// Restore progress from Steam stat
		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 zombieKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("zombie_kills", &zombieKills))
			{
				SetCount(zombieKills);
				Msg("[Achievement] Restored zombie kills from Steam: %d/%d\n", zombieKills, GetGoal());
			}
		}
	}

	void HandleZombieKill()
	{
		if (!IsAchieved())
		{
			// Increment both the achievement and Steam stat
			IncrementCount();

			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("zombie_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Zombie kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

DECLARE_ACHIEVEMENT(CAchievementModPopWeasel, ACHIEVEMENT_MOD_POP_WEASEL, "MOD_POP_WEASEL", 5);

// Storyline get 666 zombie kills achievement
class CAchievementModCopKills2 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(666);

		// Restore progress from Steam stat
		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 zombieKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("zombie_kills", &zombieKills))
			{
				SetCount(zombieKills);
				Msg("[Achievement] Restored zombie kills from Steam: %d/%d\n", zombieKills, GetGoal());
			}
		}
	}

	void HandleZombieKill()
	{
		if (!IsAchieved())
		{
			// Increment both the achievement and Steam stat
			IncrementCount();

			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("zombie_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Zombie kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

DECLARE_ACHIEVEMENT(CAchievementModCopKills2, ACHIEVEMENT_MOD_GOT_COP_KILLS2, "MOD_GOT_COP_KILLS2", 5);

// Storyline get 333 zombie kills achievement
class CAchievementModCopKills3 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(333);

		// Restore progress from Steam stat
		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 zombieKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("zombie_kills", &zombieKills))
			{
				SetCount(zombieKills);
				Msg("[Achievement] Restored zombie kills from Steam: %d/%d\n", zombieKills, GetGoal());
			}
		}
	}

	void HandleZombieKill()
	{
		if (!IsAchieved())
		{
			// Increment both the achievement and Steam stat
			IncrementCount();

			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("zombie_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Zombie kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

DECLARE_ACHIEVEMENT(CAchievementModCopKills3, ACHIEVEMENT_MOD_GOT_COP_KILLS3, "MOD_GOT_COP_KILLS3", 5);

// Storyline get 999 zombie kills achievement
class CAchievementModCopKills4 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(999);

		// Restore progress from Steam stat
		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 zombieKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("zombie_kills", &zombieKills))
			{
				SetCount(zombieKills);
				Msg("[Achievement] Restored zombie kills from Steam: %d/%d\n", zombieKills, GetGoal());
			}
		}
	}

	void HandleZombieKill()
	{
		if (!IsAchieved())
		{
			// Increment both the achievement and Steam stat
			IncrementCount();

			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("zombie_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Zombie kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

DECLARE_ACHIEVEMENT(CAchievementModCopKills4, ACHIEVEMENT_MOD_GOT_COP_KILLS4, "MOD_GOT_COP_KILLS4", 5);

// Storyline get 1966 zombie kills achievement
class CAchievementModCopKills5 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(1966);

		// Restore progress from Steam stat
		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 zombieKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("zombie_kills", &zombieKills))
			{
				SetCount(zombieKills);
				Msg("[Achievement] Restored zombie kills from Steam: %d/%d\n", zombieKills, GetGoal());
			}
		}
	}

	void HandleZombieKill()
	{
		if (!IsAchieved())
		{
			// Increment both the achievement and Steam stat
			IncrementCount();

			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("zombie_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Zombie kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

DECLARE_ACHIEVEMENT(CAchievementModCopKills5, ACHIEVEMENT_MOD_GOT_COP_KILLS5, "MOD_GOT_COP_KILLS5", 5);

// Storyline get 9400 zombie kills achievement
class CAchievementModCopKills6 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(9400);

		// Restore progress from Steam stat
		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 zombieKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("zombie_kills", &zombieKills))
			{
				SetCount(zombieKills);
				Msg("[Achievement] Restored zombie kills from Steam: %d/%d\n", zombieKills, GetGoal());
			}
		}
	}

	void HandleZombieKill()
	{
		if (!IsAchieved())
		{
			// Increment both the achievement and Steam stat
			IncrementCount();

			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("zombie_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Zombie kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

DECLARE_ACHIEVEMENT(CAchievementModCopKills6, ACHIEVEMENT_MOD_GOT_COP_KILLS6, "MOD_GOT_COP_KILLS6", 5);

// Storyline get 6 crow kills achievement
class CAchievementModZopKills : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(6);

		// Restore progress from Steam stat
		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 zombieKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("crow_kills", &zombieKills))
			{
				SetCount(zombieKills);
				Msg("[Achievement] Restored crow kills from Steam: %d/%d\n", zombieKills, GetGoal());
			}
		}
	}

	void HandleZombieKill()
	{
		if (!IsAchieved())
		{
			// Increment both the achievement and Steam stat
			IncrementCount();

			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("crow_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Crow kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

DECLARE_ACHIEVEMENT(CAchievementModZopKills, ACHIEVEMENT_MOD_GOT_ZOP_KILLS, "MOD_GOT_ZOP_KILLS", 5);

// Storyline get 33 crow kills achievement
class CAchievementModZopKills2 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(33);

		// Restore progress from Steam stat
		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 zombieKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("crow_kills", &zombieKills))
			{
				SetCount(zombieKills);
				Msg("[Achievement] Restored crow kills from Steam: %d/%d\n", zombieKills, GetGoal());
			}
		}
	}

	void HandleZombieKill()
	{
		if (!IsAchieved())
		{
			// Increment both the achievement and Steam stat
			IncrementCount();

			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("crow_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Crow kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

DECLARE_ACHIEVEMENT(CAchievementModZopKills2, ACHIEVEMENT_MOD_GOT_ZOP_KILLS2, "MOD_GOT_ZOP_KILLS2", 5);

// Storyline get 66 crow kills achievement
class CAchievementModZopKills3 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(66);

		// Restore progress from Steam stat
		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 zombieKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("crow_kills", &zombieKills))
			{
				SetCount(zombieKills);
				Msg("[Achievement] Restored crow kills from Steam: %d/%d\n", zombieKills, GetGoal());
			}
		}
	}

	void HandleZombieKill()
	{
		if (!IsAchieved())
		{
			// Increment both the achievement and Steam stat
			IncrementCount();

			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("crow_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Crow kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

DECLARE_ACHIEVEMENT(CAchievementModZopKills3, ACHIEVEMENT_MOD_GOT_ZOP_KILLS3, "MOD_GOT_ZOP_KILLS3", 5);

// Storyline get 222 crow kills achievement
class CAchievementModZopKills4 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(222);

		// Restore progress from Steam stat
		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 zombieKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("crow_kills", &zombieKills))
			{
				SetCount(zombieKills);
				Msg("[Achievement] Restored crow kills from Steam: %d/%d\n", zombieKills, GetGoal());
			}
		}
	}

	void HandleZombieKill()
	{
		if (!IsAchieved())
		{
			// Increment both the achievement and Steam stat
			IncrementCount();

			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("crow_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Crow kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

DECLARE_ACHIEVEMENT(CAchievementModZopKills4, ACHIEVEMENT_MOD_GOT_ZOP_KILLS4, "MOD_GOT_ZOP_KILLS4", 5);

// Storyline get 444 crow kills achievement
class CAchievementModZopKills5 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(444);

		// Restore progress from Steam stat
		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 zombieKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("crow_kills", &zombieKills))
			{
				SetCount(zombieKills);
				Msg("[Achievement] Restored crow kills from Steam: %d/%d\n", zombieKills, GetGoal());
			}
		}
	}

	void HandleZombieKill()
	{
		if (!IsAchieved())
		{
			// Increment both the achievement and Steam stat
			IncrementCount();

			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("crow_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Crow kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

DECLARE_ACHIEVEMENT(CAchievementModZopKills5, ACHIEVEMENT_MOD_GOT_ZOP_KILLS5, "MOD_GOT_ZOP_KILLS5", 5);

// Storyline get 666 crow kills achievement
class CAchievementModZopKills6 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(666);

		// Restore progress from Steam stat
		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 zombieKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("crow_kills", &zombieKills))
			{
				SetCount(zombieKills);
				Msg("[Achievement] Restored crow kills from Steam: %d/%d\n", zombieKills, GetGoal());
			}
		}
	}

	void HandleZombieKill()
	{
		if (!IsAchieved())
		{
			// Increment both the achievement and Steam stat
			IncrementCount();

			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("crow_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Crow kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

DECLARE_ACHIEVEMENT(CAchievementModZopKills6, ACHIEVEMENT_MOD_GOT_ZOP_KILLS6, "MOD_GOT_ZOP_KILLS6", 5);


// storyline get 6 metropolice kills
class CAchievementModPopKills : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(6); // Set your desired goal - example: 10 metropolice kills

		// Restore progress from Steam stat
		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 metropoliceKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("metropolice_kills", &metropoliceKills))
			{
				SetCount(metropoliceKills);
				Msg("[Achievement] Restored metropolice kills from Steam: %d/%d\n", metropoliceKills, GetGoal());
			}
		}
	}

	void HandleMetropoliceKill()
	{
		if (!IsAchieved())
		{
			// Increment both the achievement and Steam stat
			IncrementCount();

			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("metropolice_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Metropolice kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

DECLARE_ACHIEVEMENT(CAchievementModPopKills, ACHIEVEMENT_MOD_GOT_POP_KILLS, "MOD_GOT_POP_KILLS", 5);

// storyline get 66 metropolice kills
class CAchievementModPopKills2 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(66); // Set your desired goal - example: 10 metropolice kills

		// Restore progress from Steam stat
		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 metropoliceKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("metropolice_kills", &metropoliceKills))
			{
				SetCount(metropoliceKills);
				Msg("[Achievement] Restored metropolice kills from Steam: %d/%d\n", metropoliceKills, GetGoal());
			}
		}
	}

	void HandleMetropoliceKill()
	{
		if (!IsAchieved())
		{
			// Increment both the achievement and Steam stat
			IncrementCount();

			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("metropolice_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Metropolice kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

DECLARE_ACHIEVEMENT(CAchievementModPopKills2, ACHIEVEMENT_MOD_GOT_POP_KILLS2, "MOD_GOT_POP_KILLS2", 5);

// storyline get 333 metropolice kills
class CAchievementModPopKills3 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(333); // Set your desired goal - example: 10 metropolice kills

		// Restore progress from Steam stat
		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 metropoliceKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("metropolice_kills", &metropoliceKills))
			{
				SetCount(metropoliceKills);
				Msg("[Achievement] Restored metropolice kills from Steam: %d/%d\n", metropoliceKills, GetGoal());
			}
		}
	}

	void HandleMetropoliceKill()
	{
		if (!IsAchieved())
		{
			// Increment both the achievement and Steam stat
			IncrementCount();

			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("metropolice_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Metropolice kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

DECLARE_ACHIEVEMENT(CAchievementModPopKills3, ACHIEVEMENT_MOD_GOT_POP_KILLS3, "MOD_GOT_POP_KILLS3", 5);

// storyline get 666 metropolice kills
class CAchievementModPopKills4 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(666); // Set your desired goal - example: 10 metropolice kills

		// Restore progress from Steam stat
		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 metropoliceKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("metropolice_kills", &metropoliceKills))
			{
				SetCount(metropoliceKills);
				Msg("[Achievement] Restored metropolice kills from Steam: %d/%d\n", metropoliceKills, GetGoal());
			}
		}
	}

	void HandleMetropoliceKill()
	{
		if (!IsAchieved())
		{
			// Increment both the achievement and Steam stat
			IncrementCount();

			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("metropolice_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Metropolice kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

DECLARE_ACHIEVEMENT(CAchievementModPopKills4, ACHIEVEMENT_MOD_GOT_POP_KILLS4, "MOD_GOT_POP_KILLS4", 5);

// storyline get 999 metropolice kills
class CAchievementModPopKills5 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(999); // Set your desired goal - example: 10 metropolice kills

		// Restore progress from Steam stat
		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 metropoliceKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("metropolice_kills", &metropoliceKills))
			{
				SetCount(metropoliceKills);
				Msg("[Achievement] Restored metropolice kills from Steam: %d/%d\n", metropoliceKills, GetGoal());
			}
		}
	}

	void HandleMetropoliceKill()
	{
		if (!IsAchieved())
		{
			// Increment both the achievement and Steam stat
			IncrementCount();

			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("metropolice_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Metropolice kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

DECLARE_ACHIEVEMENT(CAchievementModPopKills5, ACHIEVEMENT_MOD_GOT_POP_KILLS5, "MOD_GOT_POP_KILLS5", 5);

// storyline get 1666 metropolice kills
class CAchievementModPopKills6 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(1666); // Set your desired goal - example: 10 metropolice kills

		// Restore progress from Steam stat
		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 metropoliceKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("metropolice_kills", &metropoliceKills))
			{
				SetCount(metropoliceKills);
				Msg("[Achievement] Restored metropolice kills from Steam: %d/%d\n", metropoliceKills, GetGoal());
			}
		}
	}

	void HandleMetropoliceKill()
	{
		if (!IsAchieved())
		{
			// Increment both the achievement and Steam stat
			IncrementCount();

			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("metropolice_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Metropolice kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

DECLARE_ACHIEVEMENT(CAchievementModPopKills6, ACHIEVEMENT_MOD_GOT_POP_KILLS6, "MOD_GOT_POP_KILLS6", 5);

class CAchievementModTopKills : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(6); // Adjust this number as needed
		
		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 combineKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("combines_kills", &combineKills))
			{
				SetCount(combineKills);
				Msg("[Achievement] Restored combine kills from Steam: %d/%d\n", combineKills, GetGoal());
			}
		}
	}

	void HandleCombineKill()
	{
		if (!IsAchieved())
		{
			IncrementCount();
			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("combines_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Combine kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};


DECLARE_ACHIEVEMENT(CAchievementModTopKills, ACHIEVEMENT_MOD_GOT_TOP_KILLS, "MOD_GOT_TOP_KILLS", 5);

// storyline kill 88 guards
class CAchievementModTopKills2 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(88); // Adjust this number as needed

		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 combineKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("combines_kills", &combineKills))
			{
				SetCount(combineKills);
				Msg("[Achievement] Restored combine kills from Steam: %d/%d\n", combineKills, GetGoal());
			}
		}
	}

	void HandleCombineKill()
	{
		if (!IsAchieved())
		{
			IncrementCount();
			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("combines_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Combine kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};


DECLARE_ACHIEVEMENT(CAchievementModTopKills2, ACHIEVEMENT_MOD_GOT_TOP_KILLS2, "MOD_GOT_TOP_KILLS2", 5);

// kill 222 guards
class CAchievementModTopKills3 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(222); // Adjust this number as needed

		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 combineKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("combines_kills", &combineKills))
			{
				SetCount(combineKills);
				Msg("[Achievement] Restored combine kills from Steam: %d/%d\n", combineKills, GetGoal());
			}
		}
	}

	void HandleCombineKill()
	{
		if (!IsAchieved())
		{
			IncrementCount();
			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("combines_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Combine kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};


DECLARE_ACHIEVEMENT(CAchievementModTopKills3, ACHIEVEMENT_MOD_GOT_TOP_KILLS3, "MOD_GOT_TOP_KILLS3", 5);

// kill 444 guards
class CAchievementModTopKills4 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(444); // Adjust this number as needed

		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 combineKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("combines_kills", &combineKills))
			{
				SetCount(combineKills);
				Msg("[Achievement] Restored combine kills from Steam: %d/%d\n", combineKills, GetGoal());
			}
		}
	}

	void HandleCombineKill()
	{
		if (!IsAchieved())
		{
			IncrementCount();
			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("combines_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Combine kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};


DECLARE_ACHIEVEMENT(CAchievementModTopKills4, ACHIEVEMENT_MOD_GOT_TOP_KILLS4, "MOD_GOT_TOP_KILLS4", 5);

// kill 666 guards
class CAchievementModTopKills5 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(666); // Adjust this number as needed

		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 combineKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("combines_kills", &combineKills))
			{
				SetCount(combineKills);
				Msg("[Achievement] Restored combine kills from Steam: %d/%d\n", combineKills, GetGoal());
			}
		}
	}

	void HandleCombineKill()
	{
		if (!IsAchieved())
		{
			IncrementCount();
			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("combines_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Combine kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};


DECLARE_ACHIEVEMENT(CAchievementModTopKills5, ACHIEVEMENT_MOD_GOT_TOP_KILLS5, "MOD_GOT_TOP_KILLS5", 5);

//kill 1111 guards
class CAchievementModTopKills6 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(1111); // Adjust this number as needed

		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 combineKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("combines_kills", &combineKills))
			{
				SetCount(combineKills);
				Msg("[Achievement] Restored combine kills from Steam: %d/%d\n", combineKills, GetGoal());
			}
		}
	}

	void HandleCombineKill()
	{
		if (!IsAchieved())
		{
			IncrementCount();
			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("combines_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Combine kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

DECLARE_ACHIEVEMENT(CAchievementModTopKills6, ACHIEVEMENT_MOD_GOT_TOP_KILLS6, "MOD_GOT_TOP_KILLS6", 5);

// player vs player kills 6
class CAchievementModBopKills : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(6); // Adjust this number as needed for PvP kills

		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 playerKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("player_kills", &playerKills))
			{
				SetCount(playerKills);
				Msg("[Achievement] Restored player kills from Steam: %d/%d\n", playerKills, GetGoal());
			}
		}
	}

	void HandlePlayerKill()
	{
		if (!IsAchieved())
		{
			IncrementCount();
			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("player_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Player kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

// 3. Declare the achievement:
DECLARE_ACHIEVEMENT(CAchievementModBopKills, ACHIEVEMENT_MOD_GOT_BOP_KILLS, "MOD_GOT_BOP_KILLS", 5);

// player vs player kills 69
class CAchievementModBopKills2 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(69); // Adjust this number as needed for PvP kills

		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 playerKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("player_kills", &playerKills))
			{
				SetCount(playerKills);
				Msg("[Achievement] Restored player kills from Steam: %d/%d\n", playerKills, GetGoal());
			}
		}
	}

	void HandlePlayerKill()
	{
		if (!IsAchieved())
		{
			IncrementCount();
			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("player_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Player kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

// 3. Declare the achievement:
DECLARE_ACHIEVEMENT(CAchievementModBopKills2, ACHIEVEMENT_MOD_GOT_BOP_KILLS2, "MOD_GOT_BOP_KILLS2", 5);

// player vs player kills 333
class CAchievementModBopKills3 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(333); // Adjust this number as needed for PvP kills

		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 playerKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("player_kills", &playerKills))
			{
				SetCount(playerKills);
				Msg("[Achievement] Restored player kills from Steam: %d/%d\n", playerKills, GetGoal());
			}
		}
	}

	void HandlePlayerKill()
	{
		if (!IsAchieved())
		{
			IncrementCount();
			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("player_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Player kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

// 3. Declare the achievement:
DECLARE_ACHIEVEMENT(CAchievementModBopKills3, ACHIEVEMENT_MOD_GOT_BOP_KILLS3, "MOD_GOT_BOP_KILLS3", 5);

// player vs player kills 666
class CAchievementModBopKills4 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(666); // Adjust this number as needed for PvP kills

		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 playerKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("player_kills", &playerKills))
			{
				SetCount(playerKills);
				Msg("[Achievement] Restored player kills from Steam: %d/%d\n", playerKills, GetGoal());
			}
		}
	}

	void HandlePlayerKill()
	{
		if (!IsAchieved())
		{
			IncrementCount();
			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("player_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Player kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

// 3. Declare the achievement:
DECLARE_ACHIEVEMENT(CAchievementModBopKills4, ACHIEVEMENT_MOD_GOT_BOP_KILLS4, "MOD_GOT_BOP_KILLS4", 5);

// player vs player kills 999
class CAchievementModBopKills5 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(999); // Adjust this number as needed for PvP kills

		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 playerKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("player_kills", &playerKills))
			{
				SetCount(playerKills);
				Msg("[Achievement] Restored player kills from Steam: %d/%d\n", playerKills, GetGoal());
			}
		}
	}

	void HandlePlayerKill()
	{
		if (!IsAchieved())
		{
			IncrementCount();
			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("player_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Player kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

// 3. Declare the achievement:
DECLARE_ACHIEVEMENT(CAchievementModBopKills5, ACHIEVEMENT_MOD_GOT_BOP_KILLS5, "MOD_GOT_BOP_KILLS5", 5);

// player vs player kills 1666
class CAchievementModBopKills6 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(1666); // Adjust this number as needed for PvP kills

		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 playerKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("player_kills", &playerKills))
			{
				SetCount(playerKills);
				Msg("[Achievement] Restored player kills from Steam: %d/%d\n", playerKills, GetGoal());
			}
		}
	}

	void HandlePlayerKill()
	{
		if (!IsAchieved())
		{
			IncrementCount();
			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("player_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Player kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

// 3. Declare the achievement:
DECLARE_ACHIEVEMENT(CAchievementModBopKills6, ACHIEVEMENT_MOD_GOT_BOP_KILLS6, "MOD_GOT_BOP_KILLS6", 5);

// get 6 kills with melee weapons
class CAchievementModHooliganToolery : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(6); // 6 tool weapon kills

		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 toolKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("tool_kills", &toolKills))
			{
				SetCount(toolKills);
				Msg("[Achievement] Restored tool kills from Steam: %d/%d\n", toolKills, GetGoal());
			}
		}
	}

	void HandleToolKill()
	{
		if (!IsAchieved())
		{
			IncrementCount();
			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("tool_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Tool kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

// 3. Declare the achievement:
DECLARE_ACHIEVEMENT(CAchievementModHooliganToolery, ACHIEVEMENT_MOD_HOOLIGAN_TOOLERY, "MOD_HOOLIGAN_TOOLERY", 5);

// get 69 kills with melee weapons
class CAchievementModHooliganToolery2 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(69); // 69 tool weapon kills

		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 toolKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("tool_kills", &toolKills))
			{
				SetCount(toolKills);
				Msg("[Achievement] Restored tool kills from Steam: %d/%d\n", toolKills, GetGoal());
			}
		}
	}

	void HandleToolKill()
	{
		if (!IsAchieved())
		{
			IncrementCount();
			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("tool_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Tool kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

// 3. Declare the achievement:
DECLARE_ACHIEVEMENT(CAchievementModHooliganToolery2, ACHIEVEMENT_MOD_HOOLIGAN_TOOLERY2, "MOD_HOOLIGAN_TOOLERY2", 5);

// get 222 kills with melee weapons
class CAchievementModHooliganToolery3 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(222); // 222 tool weapon kills

		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 toolKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("tool_kills", &toolKills))
			{
				SetCount(toolKills);
				Msg("[Achievement] Restored tool kills from Steam: %d/%d\n", toolKills, GetGoal());
			}
		}
	}

	void HandleToolKill()
	{
		if (!IsAchieved())
		{
			IncrementCount();
			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("tool_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Tool kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

// 3. Declare the achievement:
DECLARE_ACHIEVEMENT(CAchievementModHooliganToolery3, ACHIEVEMENT_MOD_HOOLIGAN_TOOLERY3, "MOD_HOOLIGAN_TOOLERY3", 5);

// get 444 kills with melee weapons
class CAchievementModHooliganToolery4 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(444); // 444 tool weapon kills

		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 toolKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("tool_kills", &toolKills))
			{
				SetCount(toolKills);
				Msg("[Achievement] Restored tool kills from Steam: %d/%d\n", toolKills, GetGoal());
			}
		}
	}

	void HandleToolKill()
	{
		if (!IsAchieved())
		{
			IncrementCount();
			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("tool_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Tool kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

// 3. Declare the achievement:
DECLARE_ACHIEVEMENT(CAchievementModHooliganToolery4, ACHIEVEMENT_MOD_HOOLIGAN_TOOLERY4, "MOD_HOOLIGAN_TOOLERY4", 5);

// get 666 kills with melee weapons
class CAchievementModHooliganToolery5 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(666); // 666 tool weapon kills

		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 toolKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("tool_kills", &toolKills))
			{
				SetCount(toolKills);
				Msg("[Achievement] Restored tool kills from Steam: %d/%d\n", toolKills, GetGoal());
			}
		}
	}

	void HandleToolKill()
	{
		if (!IsAchieved())
		{
			IncrementCount();
			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("tool_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Tool kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

// 3. Declare the achievement:
DECLARE_ACHIEVEMENT(CAchievementModHooliganToolery5, ACHIEVEMENT_MOD_HOOLIGAN_TOOLERY5, "MOD_HOOLIGAN_TOOLERY5", 5);

// get 999 kills with melee weapons
class CAchievementModHooliganToolery6 : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(999); // 999 tool weapon kills

		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 toolKills = 0;
			if (steamapicontext->SteamUserStats()->GetStat("tool_kills", &toolKills))
			{
				SetCount(toolKills);
				Msg("[Achievement] Restored tool kills from Steam: %d/%d\n", toolKills, GetGoal());
			}
		}
	}

	void HandleToolKill()
	{
		if (!IsAchieved())
		{
			IncrementCount();
			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				int32 newCount = GetCount();
				steamapicontext->SteamUserStats()->SetStat("tool_kills", newCount);
				steamapicontext->SteamUserStats()->StoreStats();
				Msg("[Achievement] Tool kill count: %d/%d (saved to Steam)\n", newCount, GetGoal());
			}
		}
	}
};

// 3. Declare the achievement:
DECLARE_ACHIEVEMENT(CAchievementModHooliganToolery6, ACHIEVEMENT_MOD_HOOLIGAN_TOOLERY6, "MOD_HOOLIGAN_TOOLERY6", 5);


// Storyline hop the fence into junk yard achivement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_HIT_TRIGGER, "MOD_HIT_TRIGGER", 5);

// Storyline last cache spot 3 achivement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_REPEAT_OFFENDER, "MOD_REPEAT_OFFENDER", 5);

// Storyline break the vent air out escape achivement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_BODY_BAG, "MOD_BODY_BAG", 5);

// Storyline dumpster achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_DUMPSTER_DIVER, "MOD_DUMPSTER_DIVER", 5);

// Storyline snipe the tower gaurds achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_AIM_SMALL, "MOD_AIM_SMALL", 5);

// Storyline gate gaurds killed you achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_FIRING_SQUAD, "MOD_FIRING_SQUAD", 5);

// Storyline 1st gaurd killed you achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_CAT_KILL, "MOD_CAT_KILL", 5);

// Storyline 1st gaurd caught you achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_CAT_CHASE, "MOD_CAT_CHASE", 5);

// Storyline spotlight achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_CAUGHT_SPOTLIGHT, "MOD_CAUGHT_SPOTLIGHT", 5);

// Storyline backlot rappel achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_REIGN_ABOVE, "MOD_REIGN_ABOVE", 5);

// Storyline begin story achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_STORY_BEGINS, "MOD_STORY_BEGINS", 5);

// Storyline kill birdie achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_KILL_BIRDIE, "MOD_KILL_BIRDIE", 5);

// Storyline Find battery achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_FIND_BATTERY, "MOD_FIND_BATTERY", 5);

// Storyline Jump to ladder achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_JUMP_GAP, "MOD_JUMP_GAP", 5);

// Storyline Break security box achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_BOX_BREAKER, "MOD_BOX_BREAKER", 5);

// Storyline Open second cache shack achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_MYSTERY_BOX, "MOD_MYSTERY_BOX", 5);

// Storyline plugin battery to tower achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_POWER_TOWER, "MOD_POWER_TOWER", 5);

// Storyline type in code to tower achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_CODE_TOWER, "MOD_CODE_TOWER", 5);

// Storyline bust the gate lock tower achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_LOCK_BUSTER, "MOD_LOCK_BUSTER", 5);

// Storyline escape end game achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_GREAT_ESCAPE, "MOD_GREAT_ESCAPE", 5);

// Storyline break fireextinguisher achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_FIRE_CRACKER, "MOD_FIRE_CRACKER", 5);

// Storyline achievement finding crowbar storyline
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_GOT_SECRET_WEAPON, "MOD_GOT_SECRET_WEAPON", 5);

// hammer editor developer achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_IAM_DEVELOPER, "MOD_IAM_DEVELOPER", 5);

// deadend drive team deathmatch achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_TEAM_DEATHMATCH, "MOD_TEAM_DEATHMATCH", 5);

// deadend drive crane achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_HELLO_OPERATOR, "MOD_HELLO_OPERATOR", 5);

// slumington subway battery achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_CONTROL_GAINER, "MOD_CONTROL_GAINER", 5);

// slumington subway keyboard achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_CONTROL_GAINER2, "MOD_CONTROL_GAINER2", 5);

// slumington subway escort achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_SPECIAL_DELIVERY, "MOD_SPECIAL_DELIVERY", 5);

// grave mistakes grave digging achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_GRAVE_DIGGING, "MOD_GRAVE_DIGGING", 5);

// grave mistakes gnome capture achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_GNOME_HOME, "MOD_GNOME_HOME", 5);

// grave mistakes gnome alone achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_GNOME_ALONE, "MOD_GNOME_ALONE", 5);

// grave mistakes digging achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_LITTLE_DIGGER, "MOD_LITTLE_DIGGER", 5);

// hellville hotel bomb grab achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_BOMB_CARRIER, "MOD_BOMB_CARRIER", 5);

// hellville hotel bomb delivery achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_BOMB_DELIVERY, "MOD_BOMB_DELIVERY", 5);

// hellville hotel rope ladder hotel achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_ROPE_HOTEL, "MOD_ROPE_HOTEL", 5);

// hellville hotel play gamemode achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_PLAY_HOTEL, "MOD_PLAY_HOTEL", 5);

// hellville hotel check out fire escapes achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_FIRE_HOTEL, "MOD_FIRE_HOTEL", 5);

// killhaus play a deathmatch achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_PLAY_DEATHMATCH, "MOD_PLAY_DEATHMATCH", 5);

// killhaus second floor reach achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_MOVIN_UP, "MOD_MOVIN_UP", 5);

// killhaus bear trap achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_BEAR_TRAP, "MOD_BEAR_TRAP", 5);

// bulletgallery destroy 10 targets achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_TARGET_SHOOTER, "MOD_TARGET_SHOOTER", 5);

// bulletgallery play gamemode achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_TRAINING_DAY, "MOD_TRAINING_DAY", 5);

// secret achievement secret buck hunter bulletgallery
DECLARE_MAP_EVENT_ACHIEVEMENT_HIDDEN(ACHIEVEMENT_MOD_BUCK_HUNTER, "MOD_BUCK_HUNTER", 5);

// secret achievement secret duck hunter bulletgallery
DECLARE_MAP_EVENT_ACHIEVEMENT_HIDDEN(ACHIEVEMENT_MOD_DUCK_HUNTER, "MOD_DUCK_HUNTER", 5);

// secret achievement secret plunderbread storyline
DECLARE_MAP_EVENT_ACHIEVEMENT_HIDDEN(ACHIEVEMENT_MOD_HOLY_BREAD, "MOD_HOLY_BREAD", 5);

// Add this function before your console command:
void UpdateNPCStat(const char* npcType)
{
	if (!steamapicontext || !steamapicontext->SteamUserStats())
		return;

	char statName[64];
	// Special case: manhacks count as crow kills
	if (FStrEq(npcType, "npc_manhack"))
	{
		Q_snprintf(statName, sizeof(statName), "crow_kills");
	}
	else
	{
		// Remove "npc_" prefix and add "_kills" suffix for other types
		Q_snprintf(statName, sizeof(statName), "%s_kills", npcType + 4);
	}

	int32 currentKills = 0;
	steamapicontext->SteamUserStats()->GetStat(statName, &currentKills);
	currentKills++;
	steamapicontext->SteamUserStats()->SetStat(statName, currentKills);
	steamapicontext->SteamUserStats()->StoreStats();
	Msg("[Achievement] %s kills: %d (saved to Steam)\n", npcType, currentKills);
}

// Replace your existing console command with this enhanced version:
CON_COMMAND(zombie_kill_increment, "Increment zombie kill count for achievement")
{
	// Add debug output to see what arguments we're receiving
	Msg("[Console Debug] zombie_kill_increment called with %d args\n", args.ArgC());
	for (int i = 0; i < args.ArgC(); i++)
	{
		Msg("[Console Debug] Arg[%d]: %s\n", i, args.Arg(i));
	}

	extern CAchievementMgr g_AchievementMgrMod;

	// FIXED: Get NPC type from the correct argument position
	const char* npcType = "npc_zombie";
	if (args.ArgC() >= 3)  // Need at least 3 args for: command, userID, npcType
	{
		npcType = args.Arg(2);  // NPC type is the 3rd argument (index 2)
	}
	else if (args.ArgC() >= 2)  // Backwards compatibility for old format
	{
		npcType = args.Arg(1);  // Old format where NPC type was 2nd argument
	}

	Msg("[Console Debug] Processing NPC type: %s\n", npcType);

	// Handle zombie-specific achievements (only for npc_zombie kills)
	if (FStrEq(npcType, "npc_zombie"))
	{
		Msg("[Console Debug] Processing zombie achievements...\n");

		int zombieIDs[] = {
			ACHIEVEMENT_MOD_GOT_COP_KILLS,
			ACHIEVEMENT_MOD_POP_WEASEL,
			ACHIEVEMENT_MOD_GOT_COP_KILLS2,
			ACHIEVEMENT_MOD_GOT_COP_KILLS3,
			ACHIEVEMENT_MOD_GOT_COP_KILLS4,
			ACHIEVEMENT_MOD_GOT_COP_KILLS5,
			ACHIEVEMENT_MOD_GOT_COP_KILLS6
		};

		for (int i = 0; i < ARRAYSIZE(zombieIDs); i++)
		{
			CBaseAchievement* pAchievement = g_AchievementMgrMod.GetAchievementByID(zombieIDs[i]);
			if (pAchievement)
			{
				if (zombieIDs[i] == ACHIEVEMENT_MOD_GOT_COP_KILLS)
				{
					CAchievementModCopKills* pCopAchievement = dynamic_cast<CAchievementModCopKills*>(pAchievement);
					if (pCopAchievement)
					{
						pCopAchievement->HandleZombieKill();
						Msg("[Console Debug] Handled COP_KILLS achievement\n");
					}
				}
				else if (zombieIDs[i] == ACHIEVEMENT_MOD_POP_WEASEL)
				{
					CAchievementModPopWeasel* pWeaselAchievement = dynamic_cast<CAchievementModPopWeasel*>(pAchievement);
					if (pWeaselAchievement)
					{
						pWeaselAchievement->HandleZombieKill();
						Msg("[Console Debug] Handled POP_WEASEL achievement\n");
					}
				}
				else if (zombieIDs[i] == ACHIEVEMENT_MOD_GOT_COP_KILLS2)
				{
					CAchievementModCopKills2* pCop2Achievement = dynamic_cast<CAchievementModCopKills2*>(pAchievement);
					if (pCop2Achievement)
					{
						pCop2Achievement->HandleZombieKill();
						Msg("[Console Debug] Handled COP_KILLS2 achievement\n");
					}
				}
				else if (zombieIDs[i] == ACHIEVEMENT_MOD_GOT_COP_KILLS3)
				{
					CAchievementModCopKills3* pCop3Achievement = dynamic_cast<CAchievementModCopKills3*>(pAchievement);
					if (pCop3Achievement)
					{
						pCop3Achievement->HandleZombieKill();
						Msg("[Console Debug] Handled COP_KILLS3 achievement\n");
					}
				}
				else if (zombieIDs[i] == ACHIEVEMENT_MOD_GOT_COP_KILLS4)
				{
					CAchievementModCopKills4* pCop4Achievement = dynamic_cast<CAchievementModCopKills4*>(pAchievement);
					if (pCop4Achievement)
					{
						pCop4Achievement->HandleZombieKill();
						Msg("[Console Debug] Handled COP_KILLS4 achievement\n");
					}
				}
				else if (zombieIDs[i] == ACHIEVEMENT_MOD_GOT_COP_KILLS5)
				{
					CAchievementModCopKills5* pCop5Achievement = dynamic_cast<CAchievementModCopKills5*>(pAchievement);
					if (pCop5Achievement)
					{
						pCop5Achievement->HandleZombieKill();
						Msg("[Console Debug] Handled COP_KILLS5 achievement\n");
					}
				}
				else if (zombieIDs[i] == ACHIEVEMENT_MOD_GOT_COP_KILLS6)
				{
					CAchievementModCopKills6* pCop6Achievement = dynamic_cast<CAchievementModCopKills6*>(pAchievement);
					if (pCop6Achievement)
					{
						pCop6Achievement->HandleZombieKill();
						Msg("[Console Debug] Handled COP_KILLS6 achievement\n");
					}
				}
			}
		}
	}

	// Handle manhack-specific achievements (for npc_manhack kills)
	if (FStrEq(npcType, "npc_manhack"))
	{
		Msg("[Console Debug] Processing manhack achievements...\n");

		// Array of all manhack achievement IDs
		int manhackIDs[] = {
			ACHIEVEMENT_MOD_GOT_ZOP_KILLS,
			ACHIEVEMENT_MOD_GOT_ZOP_KILLS2,
			ACHIEVEMENT_MOD_GOT_ZOP_KILLS3,
			ACHIEVEMENT_MOD_GOT_ZOP_KILLS4,
			ACHIEVEMENT_MOD_GOT_ZOP_KILLS5,
			ACHIEVEMENT_MOD_GOT_ZOP_KILLS6
		};

		for (int i = 0; i < ARRAYSIZE(manhackIDs); i++)
		{
			CBaseAchievement* pAchievement = g_AchievementMgrMod.GetAchievementByID(manhackIDs[i]);
			if (pAchievement)
			{
				if (manhackIDs[i] == ACHIEVEMENT_MOD_GOT_ZOP_KILLS)
				{
					CAchievementModZopKills* pZopAchievement = dynamic_cast<CAchievementModZopKills*>(pAchievement);
					if (pZopAchievement) pZopAchievement->HandleZombieKill();
				}
				else if (manhackIDs[i] == ACHIEVEMENT_MOD_GOT_ZOP_KILLS2)
				{
					CAchievementModZopKills2* pZop2Achievement = dynamic_cast<CAchievementModZopKills2*>(pAchievement);
					if (pZop2Achievement) pZop2Achievement->HandleZombieKill();
				}
				else if (manhackIDs[i] == ACHIEVEMENT_MOD_GOT_ZOP_KILLS3)
				{
					CAchievementModZopKills3* pZop3Achievement = dynamic_cast<CAchievementModZopKills3*>(pAchievement);
					if (pZop3Achievement) pZop3Achievement->HandleZombieKill();
				}
				else if (manhackIDs[i] == ACHIEVEMENT_MOD_GOT_ZOP_KILLS4)
				{
					CAchievementModZopKills4* pZop4Achievement = dynamic_cast<CAchievementModZopKills4*>(pAchievement);
					if (pZop4Achievement) pZop4Achievement->HandleZombieKill();
				}
				else if (manhackIDs[i] == ACHIEVEMENT_MOD_GOT_ZOP_KILLS5)
				{
					CAchievementModZopKills5* pZop5Achievement = dynamic_cast<CAchievementModZopKills5*>(pAchievement);
					if (pZop5Achievement) pZop5Achievement->HandleZombieKill();
				}
				else if (manhackIDs[i] == ACHIEVEMENT_MOD_GOT_ZOP_KILLS6)
				{
					CAchievementModZopKills6* pZop6Achievement = dynamic_cast<CAchievementModZopKills6*>(pAchievement);
					if (pZop6Achievement) pZop6Achievement->HandleZombieKill();
				}
			}
		}
		Msg("[Console Debug] Handled all ZOP_KILLS (manhack) achievements\n");
	}


	// ADD THIS NEW SECTION: Handle metropolice-specific achievements (for npc_metropolice kills)
	if (FStrEq(npcType, "npc_metropolice"))
	{
		Msg("[Console Debug] Processing metropolice achievements...\n");

		// Array of all metropolice achievement IDs
		int metropoliceIDs[] = {
			ACHIEVEMENT_MOD_GOT_POP_KILLS,
			ACHIEVEMENT_MOD_GOT_POP_KILLS2,
			ACHIEVEMENT_MOD_GOT_POP_KILLS3,
			ACHIEVEMENT_MOD_GOT_POP_KILLS4,
			ACHIEVEMENT_MOD_GOT_POP_KILLS5,
			ACHIEVEMENT_MOD_GOT_POP_KILLS6
		};

		for (int i = 0; i < ARRAYSIZE(metropoliceIDs); i++)
		{
			CBaseAchievement* pAchievement = g_AchievementMgrMod.GetAchievementByID(metropoliceIDs[i]);
			if (pAchievement)
			{
				if (metropoliceIDs[i] == ACHIEVEMENT_MOD_GOT_POP_KILLS)
				{
					CAchievementModPopKills* pPopAchievement = dynamic_cast<CAchievementModPopKills*>(pAchievement);
					if (pPopAchievement) pPopAchievement->HandleMetropoliceKill();
				}
				else if (metropoliceIDs[i] == ACHIEVEMENT_MOD_GOT_POP_KILLS2)
				{
					CAchievementModPopKills2* pPop2Achievement = dynamic_cast<CAchievementModPopKills2*>(pAchievement);
					if (pPop2Achievement) pPop2Achievement->HandleMetropoliceKill();
				}
				else if (metropoliceIDs[i] == ACHIEVEMENT_MOD_GOT_POP_KILLS3)
				{
					CAchievementModPopKills3* pPop3Achievement = dynamic_cast<CAchievementModPopKills3*>(pAchievement);
					if (pPop3Achievement) pPop3Achievement->HandleMetropoliceKill();
				}
				else if (metropoliceIDs[i] == ACHIEVEMENT_MOD_GOT_POP_KILLS4)
				{
					CAchievementModPopKills4* pPop4Achievement = dynamic_cast<CAchievementModPopKills4*>(pAchievement);
					if (pPop4Achievement) pPop4Achievement->HandleMetropoliceKill();
				}
				else if (metropoliceIDs[i] == ACHIEVEMENT_MOD_GOT_POP_KILLS5)
				{
					CAchievementModPopKills5* pPop5Achievement = dynamic_cast<CAchievementModPopKills5*>(pAchievement);
					if (pPop5Achievement) pPop5Achievement->HandleMetropoliceKill();
				}
				else if (metropoliceIDs[i] == ACHIEVEMENT_MOD_GOT_POP_KILLS6)
				{
					CAchievementModPopKills6* pPop6Achievement = dynamic_cast<CAchievementModPopKills6*>(pAchievement);
					if (pPop6Achievement) pPop6Achievement->HandleMetropoliceKill();
				}
			}
		}
		Msg("[Console Debug] Handled all POP_KILLS (metropolice) achievements\n");
	}

	if (FStrEq(npcType, "npc_combine_s"))
	{
		Msg("[Console Debug] Processing combine achievements...\n");

		// Array of all combine achievement IDs
		int combineIDs[] = {
			ACHIEVEMENT_MOD_GOT_TOP_KILLS,
			ACHIEVEMENT_MOD_GOT_TOP_KILLS2,
			ACHIEVEMENT_MOD_GOT_TOP_KILLS3,
			ACHIEVEMENT_MOD_GOT_TOP_KILLS4,
			ACHIEVEMENT_MOD_GOT_TOP_KILLS5,
			ACHIEVEMENT_MOD_GOT_TOP_KILLS6
		};

		for (int i = 0; i < ARRAYSIZE(combineIDs); i++)
		{
			CBaseAchievement* pAchievement = g_AchievementMgrMod.GetAchievementByID(combineIDs[i]);
			if (pAchievement)
			{
				if (combineIDs[i] == ACHIEVEMENT_MOD_GOT_TOP_KILLS)
				{
					CAchievementModTopKills* pTopAchievement = dynamic_cast<CAchievementModTopKills*>(pAchievement);
					if (pTopAchievement) pTopAchievement->HandleCombineKill();
				}
				else if (combineIDs[i] == ACHIEVEMENT_MOD_GOT_TOP_KILLS2)
				{
					CAchievementModTopKills2* pTop2Achievement = dynamic_cast<CAchievementModTopKills2*>(pAchievement);
					if (pTop2Achievement) pTop2Achievement->HandleCombineKill();
				}
				else if (combineIDs[i] == ACHIEVEMENT_MOD_GOT_TOP_KILLS3)
				{
					CAchievementModTopKills3* pTop3Achievement = dynamic_cast<CAchievementModTopKills3*>(pAchievement);
					if (pTop3Achievement) pTop3Achievement->HandleCombineKill();
				}
				else if (combineIDs[i] == ACHIEVEMENT_MOD_GOT_TOP_KILLS4)
				{
					CAchievementModTopKills4* pTop4Achievement = dynamic_cast<CAchievementModTopKills4*>(pAchievement);
					if (pTop4Achievement) pTop4Achievement->HandleCombineKill();
				}
				else if (combineIDs[i] == ACHIEVEMENT_MOD_GOT_TOP_KILLS5)
				{
					CAchievementModTopKills5* pTop5Achievement = dynamic_cast<CAchievementModTopKills5*>(pAchievement);
					if (pTop5Achievement) pTop5Achievement->HandleCombineKill();
				}
				else if (combineIDs[i] == ACHIEVEMENT_MOD_GOT_TOP_KILLS6)
				{
					CAchievementModTopKills6* pTop6Achievement = dynamic_cast<CAchievementModTopKills6*>(pAchievement);
					if (pTop6Achievement) pTop6Achievement->HandleCombineKill();
				}
			}
		}
		Msg("[Console Debug] Handled all TOP_KILLS (combine) achievements\n");
	}
}
// Updated console command for multi-tier player kills:
CON_COMMAND(player_kill_increment, "Increment player kill count for achievement")
{
	Msg("[Console Debug] player_kill_increment called with %d args\n", args.ArgC());
	for (int i = 0; i < args.ArgC(); i++)
	{
		Msg("[Console Debug] Arg[%d]: %s\n", i, args.Arg(i));
	}

	extern CAchievementMgr g_AchievementMgrMod;
	Msg("[Console Debug] Processing player kill...\n");

	// Array of all player kill achievement IDs
	int playerKillIDs[] = {
		ACHIEVEMENT_MOD_GOT_BOP_KILLS,
		ACHIEVEMENT_MOD_GOT_BOP_KILLS2,
		ACHIEVEMENT_MOD_GOT_BOP_KILLS3,
		ACHIEVEMENT_MOD_GOT_BOP_KILLS4,
		ACHIEVEMENT_MOD_GOT_BOP_KILLS5,
		ACHIEVEMENT_MOD_GOT_BOP_KILLS6
	};

	for (int i = 0; i < ARRAYSIZE(playerKillIDs); i++)
	{
		CBaseAchievement* pAchievement = g_AchievementMgrMod.GetAchievementByID(playerKillIDs[i]);
		if (pAchievement)
		{
			if (playerKillIDs[i] == ACHIEVEMENT_MOD_GOT_BOP_KILLS)
			{
				CAchievementModBopKills* pBopAchievement = dynamic_cast<CAchievementModBopKills*>(pAchievement);
				if (pBopAchievement)
				{
					pBopAchievement->HandlePlayerKill();
					Msg("[Console Debug] Handled BOP_KILLS (player) achievement\n");
				}
			}
			else if (playerKillIDs[i] == ACHIEVEMENT_MOD_GOT_BOP_KILLS2)
			{
				CAchievementModBopKills2* pBop2Achievement = dynamic_cast<CAchievementModBopKills2*>(pAchievement);
				if (pBop2Achievement)
				{
					pBop2Achievement->HandlePlayerKill();
					Msg("[Console Debug] Handled BOP_KILLS2 (player) achievement\n");
				}
			}
			else if (playerKillIDs[i] == ACHIEVEMENT_MOD_GOT_BOP_KILLS3)
			{
				CAchievementModBopKills3* pBop3Achievement = dynamic_cast<CAchievementModBopKills3*>(pAchievement);
				if (pBop3Achievement)
				{
					pBop3Achievement->HandlePlayerKill();
					Msg("[Console Debug] Handled BOP_KILLS3 (player) achievement\n");
				}
			}
			else if (playerKillIDs[i] == ACHIEVEMENT_MOD_GOT_BOP_KILLS4)
			{
				CAchievementModBopKills4* pBop4Achievement = dynamic_cast<CAchievementModBopKills4*>(pAchievement);
				if (pBop4Achievement)
				{
					pBop4Achievement->HandlePlayerKill();
					Msg("[Console Debug] Handled BOP_KILLS4 (player) achievement\n");
				}
			}
			else if (playerKillIDs[i] == ACHIEVEMENT_MOD_GOT_BOP_KILLS5)
			{
				CAchievementModBopKills5* pBop5Achievement = dynamic_cast<CAchievementModBopKills5*>(pAchievement);
				if (pBop5Achievement)
				{
					pBop5Achievement->HandlePlayerKill();
					Msg("[Console Debug] Handled BOP_KILLS5 (player) achievement\n");
				}
			}
			else if (playerKillIDs[i] == ACHIEVEMENT_MOD_GOT_BOP_KILLS6)
			{
				CAchievementModBopKills6* pBop6Achievement = dynamic_cast<CAchievementModBopKills6*>(pAchievement);
				if (pBop6Achievement)
				{
					pBop6Achievement->HandlePlayerKill();
					Msg("[Console Debug] Handled BOP_KILLS6 (player) achievement\n");
				}
			}
		}
	}

	// Update Steam stat for player kills (single stat shared by all tiers)
	if (steamapicontext && steamapicontext->SteamUserStats())
	{
		int32 currentKills = 0;
		steamapicontext->SteamUserStats()->GetStat("player_kills", &currentKills);
		currentKills++;
		steamapicontext->SteamUserStats()->SetStat("player_kills", currentKills);
		steamapicontext->SteamUserStats()->StoreStats();
		Msg("[Achievement] player kills: %d (saved to Steam)\n", currentKills);
	}

	Msg("[Console Debug] Processed all BOP_KILLS achievements\n");
}
// 4. Create console command for tool kills:
CON_COMMAND(tool_kill_increment, "Increment tool kill count for achievement")
{
	Msg("[Console Debug] tool_kill_increment called with %d args\n", args.ArgC());
	for (int i = 0; i < args.ArgC(); i++)
	{
		Msg("[Console Debug] Arg[%d]: %s\n", i, args.Arg(i));
	}

	extern CAchievementMgr g_AchievementMgrMod;

	// Get weapon name from arguments
	const char* weaponName = "unknown";
	if (args.ArgC() >= 3)
	{
		weaponName = args.Arg(2);
	}

	Msg("[Console Debug] Processing tool kill with: %s\n", weaponName);

	// Array of all tool kill achievement IDs
	int toolKillIDs[] = {
		ACHIEVEMENT_MOD_HOOLIGAN_TOOLERY,
		ACHIEVEMENT_MOD_HOOLIGAN_TOOLERY2,
		ACHIEVEMENT_MOD_HOOLIGAN_TOOLERY3,
		ACHIEVEMENT_MOD_HOOLIGAN_TOOLERY4,
		ACHIEVEMENT_MOD_HOOLIGAN_TOOLERY5,
		ACHIEVEMENT_MOD_HOOLIGAN_TOOLERY6
	};

	for (int i = 0; i < ARRAYSIZE(toolKillIDs); i++)
	{
		CBaseAchievement* pAchievement = g_AchievementMgrMod.GetAchievementByID(toolKillIDs[i]);
		if (pAchievement)
		{
			if (toolKillIDs[i] == ACHIEVEMENT_MOD_HOOLIGAN_TOOLERY)
			{
				CAchievementModHooliganToolery* pToolAchievement = dynamic_cast<CAchievementModHooliganToolery*>(pAchievement);
				if (pToolAchievement)
				{
					pToolAchievement->HandleToolKill();
					Msg("[Console Debug] Handled HOOLIGAN_TOOLERY achievement\n");
				}
			}
			else if (toolKillIDs[i] == ACHIEVEMENT_MOD_HOOLIGAN_TOOLERY2)
			{
				CAchievementModHooliganToolery2* pTool2Achievement = dynamic_cast<CAchievementModHooliganToolery2*>(pAchievement);
				if (pTool2Achievement)
				{
					pTool2Achievement->HandleToolKill();
					Msg("[Console Debug] Handled HOOLIGAN_TOOLERY2 achievement\n");
				}
			}
			else if (toolKillIDs[i] == ACHIEVEMENT_MOD_HOOLIGAN_TOOLERY3)
			{
				CAchievementModHooliganToolery3* pTool3Achievement = dynamic_cast<CAchievementModHooliganToolery3*>(pAchievement);
				if (pTool3Achievement)
				{
					pTool3Achievement->HandleToolKill();
					Msg("[Console Debug] Handled HOOLIGAN_TOOLERY3 achievement\n");
				}
			}
			else if (toolKillIDs[i] == ACHIEVEMENT_MOD_HOOLIGAN_TOOLERY4)
			{
				CAchievementModHooliganToolery4* pTool4Achievement = dynamic_cast<CAchievementModHooliganToolery4*>(pAchievement);
				if (pTool4Achievement)
				{
					pTool4Achievement->HandleToolKill();
					Msg("[Console Debug] Handled HOOLIGAN_TOOLERY4 achievement\n");
				}
			}
			else if (toolKillIDs[i] == ACHIEVEMENT_MOD_HOOLIGAN_TOOLERY5)
			{
				CAchievementModHooliganToolery5* pTool5Achievement = dynamic_cast<CAchievementModHooliganToolery5*>(pAchievement);
				if (pTool5Achievement)
				{
					pTool5Achievement->HandleToolKill();
					Msg("[Console Debug] Handled HOOLIGAN_TOOLERY5 achievement\n");
				}
			}
			else if (toolKillIDs[i] == ACHIEVEMENT_MOD_HOOLIGAN_TOOLERY6)
			{
				CAchievementModHooliganToolery6* pTool6Achievement = dynamic_cast<CAchievementModHooliganToolery6*>(pAchievement);
				if (pTool6Achievement)
				{
					pTool6Achievement->HandleToolKill();
					Msg("[Console Debug] Handled HOOLIGAN_TOOLERY6 achievement\n");
				}
			}
		}
	}

	// Update Steam stat for tool kills (single stat shared by all tiers)
	if (steamapicontext && steamapicontext->SteamUserStats())
	{
		int32 currentKills = 0;
		steamapicontext->SteamUserStats()->GetStat("tool_kills", &currentKills);
		currentKills++;
		steamapicontext->SteamUserStats()->SetStat("tool_kills", currentKills);
		steamapicontext->SteamUserStats()->StoreStats();
		Msg("[Achievement] tool kills: %d (saved to Steam)\n", currentKills);
	}

	Msg("[Console Debug] Processed all HOOLIGAN_TOOLERY achievements\n");
}

#endif // GAME_DLL