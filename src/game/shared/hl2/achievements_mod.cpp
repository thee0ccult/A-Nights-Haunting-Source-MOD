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
ConVar zombie_kills("zombie_kills", "0", FCVAR_ARCHIVE);

// Storyline get 9400 zombies kills achievement
class CAchievementModCopKills : public CBaseAchievement
{
public:
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(9400);

		// Just start with local value, don’t query Steam yet
		SetCount(zombie_kills.GetInt());
		Msg("[Achievement] Init: Local zombie kills = %d/%d (waiting for Steam sync)\n", GetCount(), GetGoal());
	}

	void HandleZombieKill()
	{
		if (!IsAchieved())
		{
			IncrementCount();
			int32 newCount = GetCount();

			if (steamapicontext && steamapicontext->SteamUserStats())
			{
				bool statSet = steamapicontext->SteamUserStats()->SetStat("zombie_kills", newCount);
				bool statsStored = steamapicontext->SteamUserStats()->StoreStats();

				// Mark the real achievement manager dirty
				IAchievementMgr* pBaseMgr = engine->GetAchievementMgr();
				CAchievementMgr* pAchievementMgr = dynamic_cast<CAchievementMgr*>(pBaseMgr);
				if (pAchievementMgr)
				{
					pAchievementMgr->SetDirty(true);
				}

				Msg("[Achievement] Zombie kill: %d/%d (Steam: %s/%s)\n",
					newCount, GetGoal(),
					statSet ? "SET" : "FAIL",
					statsStored ? "STORE" : "FAIL");

				zombie_kills.SetValue(newCount);
			}

			if (newCount >= GetGoal())
			{
				Msg("[Achievement] MOD_GOT_COP_KILLS completed!\n");
			}
		}
	}

	// Debug methods
	int32 GetSteamStat()
	{
		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 steamStat = 0;
			if (steamapicontext->SteamUserStats()->GetStat("zombie_kills", &steamStat))
				return steamStat;
		}
		return -1;
	}

	void ForceSteamSync()
	{
		if (steamapicontext && steamapicontext->SteamUserStats())
		{
			int32 currentCount = GetCount();
			steamapicontext->SteamUserStats()->SetStat("zombie_kills", currentCount);
			steamapicontext->SteamUserStats()->StoreStats();
			zombie_kills.SetValue(currentCount);
			Msg("[Achievement] Force sync: %d zombie kills\n", currentCount);
		}
	}
};

DECLARE_ACHIEVEMENT(CAchievementModCopKills, ACHIEVEMENT_MOD_GOT_COP_KILLS, "MOD_GOT_COP_KILLS", 5);

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

// secret achievement secret buck hunter bulletgallery
DECLARE_MAP_EVENT_ACHIEVEMENT_HIDDEN(ACHIEVEMENT_MOD_BUCK_HUNTER, "MOD_BUCK_HUNTER", 5);

// secret achievement secret duck hunter bulletgallery
DECLARE_MAP_EVENT_ACHIEVEMENT_HIDDEN(ACHIEVEMENT_MOD_DUCK_HUNTER, "MOD_DUCK_HUNTER", 5);

// secret achievement secret plunderbread storyline
DECLARE_MAP_EVENT_ACHIEVEMENT_HIDDEN(ACHIEVEMENT_MOD_HOLY_BREAD, "MOD_HOLY_BREAD", 5);

CON_COMMAND(zombie_kill_increment, "Increment zombie kill count for achievement")
{
	// Use the main global achievement manager
	IAchievementMgr* pBaseMgr = engine->GetAchievementMgr();
	CAchievementMgr* pAchievementMgr = dynamic_cast<CAchievementMgr*>(pBaseMgr);

	if (!pAchievementMgr)
	{
		Msg("[Console] ERROR: Could not get main achievement manager!\n");
		return;
	}

	CBaseAchievement* pAchievement = pAchievementMgr->GetAchievementByID(ACHIEVEMENT_MOD_GOT_COP_KILLS);
	CAchievementModCopKills* pCopAchievement = dynamic_cast<CAchievementModCopKills*>(pAchievement);
	if (pCopAchievement)
	{
		Msg("[Console] Before increment: %d/%d\n", pCopAchievement->GetCount(), pCopAchievement->GetGoal());
		pCopAchievement->HandleZombieKill();
		Msg("[Console] After increment: %d/%d\n", pCopAchievement->GetCount(), pCopAchievement->GetGoal());
	}
	else
	{
		Msg("[Console] ERROR: Could not find zombie kill achievement!\n");
	}
}

CON_COMMAND(zombie_kill_debug, "Debug zombie kill achievement and Steam stat")
{
	IAchievementMgr* pBaseMgr = engine->GetAchievementMgr();
	CAchievementMgr* pAchievementMgr = dynamic_cast<CAchievementMgr*>(pBaseMgr);

	if (!pAchievementMgr)
	{
		Msg("[Debug] ERROR: Could not get main achievement manager!\n");
		return;
	}

	CBaseAchievement* pAchievement = pAchievementMgr->GetAchievementByID(ACHIEVEMENT_MOD_GOT_COP_KILLS);
	CAchievementModCopKills* pCopAchievement = dynamic_cast<CAchievementModCopKills*>(pAchievement);

	if (pCopAchievement)
	{
		int32 achievementCount = pCopAchievement->GetCount();
		int32 steamStat = pCopAchievement->GetSteamStat();
		int32 convarValue = zombie_kills.GetInt();

		Msg("[Debug] Achievement count: %d/%d\n", achievementCount, pCopAchievement->GetGoal());

		char szSteamStat[32];
		Q_snprintf(szSteamStat, sizeof(szSteamStat), "%d", steamStat);
		Msg("[Debug] Steam stat: %s\n", steamStat >= 0 ? szSteamStat : "INVALID");
		Msg("[Debug] ConVar value: %d\n", convarValue);
		Msg("[Debug] Achievement status: %s\n", pCopAchievement->IsAchieved() ? "COMPLETE" : "IN PROGRESS");

		if (steamStat >= 0 && achievementCount != steamStat)
		{
			Msg("[Debug] WARNING: Achievement (%d) != Steam stat (%d)\n", achievementCount, steamStat);
		}
		else if (steamStat >= 0)
		{
			Msg("[Debug] Achievement and Steam stat are synced\n");
		}
	}
	else
	{
		Msg("[Debug] ERROR: Could not find achievement!\n");
	}
}

CON_COMMAND(zombie_kill_sync, "Force sync zombie kill achievement with Steam")
{
	IAchievementMgr* pBaseMgr = engine->GetAchievementMgr();
	CAchievementMgr* pAchievementMgr = dynamic_cast<CAchievementMgr*>(pBaseMgr);

	if (!pAchievementMgr)
	{
		Msg("[Console] ERROR: Could not get main achievement manager!\n");
		return;
	}

	CBaseAchievement* pAchievement = pAchievementMgr->GetAchievementByID(ACHIEVEMENT_MOD_GOT_COP_KILLS);
	CAchievementModCopKills* pCopAchievement = dynamic_cast<CAchievementModCopKills*>(pAchievement);
	if (pCopAchievement)
	{
		pCopAchievement->ForceSteamSync();
		Msg("[Console] Forced synchronization complete\n");
	}
	else
	{
		Msg("[Console] ERROR: Could not find achievement!\n");
	}
}

// Message handler - USE THE MAIN ACHIEVEMENT MANAGER
void __MsgFunc_ZombieKilled(bf_read& msg)
{
	Msg("[Achievement] ZombieKilled message received!\n");

	IAchievementMgr* pBaseMgr = engine->GetAchievementMgr();
	CAchievementMgr* pAchievementMgr = dynamic_cast<CAchievementMgr*>(pBaseMgr);

	if (!pAchievementMgr)
	{
		Msg("[Achievement] ERROR: Could not get main achievement manager!\n");
		return;
	}

	CBaseAchievement* pAchievement = pAchievementMgr->GetAchievementByID(ACHIEVEMENT_MOD_GOT_COP_KILLS);
	CAchievementModCopKills* pCopAchievement = dynamic_cast<CAchievementModCopKills*>(pAchievement);

	if (pCopAchievement)
	{
		if (!pCopAchievement->IsAchieved())
		{
			Msg("[Achievement] Processing kill (current: %d/%d)\n",
				pCopAchievement->GetCount(), pCopAchievement->GetGoal());
			pCopAchievement->HandleZombieKill();
		}
		else
		{
			Msg("[Achievement] Already completed - ignoring kill\n");
		}
	}
	else
	{
		Msg("[Achievement] ERROR: Achievement not found!\n");
	}
}

#endif // GAME_DLL