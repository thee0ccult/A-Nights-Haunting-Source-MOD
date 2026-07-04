#include "cbase.h"

#ifdef CLIENT_DLL

#include "achievementmgr.h"
#include "baseachievement.h"
#include "convar.h"  // Add this include
#include "usermessages.h"  // Add this
#include "tier1/bitbuf.h"  // Add this
#include "saverestore.h"        // Add this
#include "saverestoretypes.h"   // Add this
#include <ctime> // <-- ADD THIS for holiday achievements
#include "leaderboard_sync.h" // leaderboard handling

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
#define ACHIEVEMENT_MOD_GOT_COP_KILLS7 40
#define ACHIEVEMENT_MOD_STORY_BEGINS 41
#define ACHIEVEMENT_MOD_BODY_BAG 42
#define ACHIEVEMENT_MOD_REPEAT_OFFENDER 43
#define ACHIEVEMENT_MOD_TARGET_SHOOTER 44
#define ACHIEVEMENT_MOD_BUCK_HUNTER 45
#define ACHIEVEMENT_MOD_DUCK_HUNTER 46
#define ACHIEVEMENT_MOD_GOT_COP_KILLS2 47
#define ACHIEVEMENT_MOD_GOT_COP_KILLS3 48
#define ACHIEVEMENT_MOD_GOT_COP_KILLS4 49
#define ACHIEVEMENT_MOD_GOT_COP_KILLS5 50
#define ACHIEVEMENT_MOD_GOT_COP_KILLS6 51
#define ACHIEVEMENT_MOD_GOT_ZOP_KILLS 52
#define ACHIEVEMENT_MOD_GOT_ZOP_KILLS2 53
#define ACHIEVEMENT_MOD_GOT_ZOP_KILLS3 54
#define ACHIEVEMENT_MOD_GOT_ZOP_KILLS4 55
#define ACHIEVEMENT_MOD_GOT_ZOP_KILLS5 56
#define ACHIEVEMENT_MOD_GOT_ZOP_KILLS6 57
#define ACHIEVEMENT_MOD_GOT_ZOP_KILLS7 58
#define ACHIEVEMENT_MOD_GOT_ZOP_KILLS8 59
#define ACHIEVEMENT_MOD_GOT_POP_KILLS 60
#define ACHIEVEMENT_MOD_GOT_POP_KILLS2 61
#define ACHIEVEMENT_MOD_GOT_POP_KILLS3 62
#define ACHIEVEMENT_MOD_GOT_POP_KILLS4 63
#define ACHIEVEMENT_MOD_GOT_POP_KILLS5 64
#define ACHIEVEMENT_MOD_GOT_POP_KILLS6 65
#define ACHIEVEMENT_MOD_GOT_TOP_KILLS 66
#define ACHIEVEMENT_MOD_GOT_TOP_KILLS2 67
#define ACHIEVEMENT_MOD_GOT_TOP_KILLS3 68
#define ACHIEVEMENT_MOD_GOT_TOP_KILLS4 69
#define ACHIEVEMENT_MOD_GOT_TOP_KILLS5 70
#define ACHIEVEMENT_MOD_GOT_TOP_KILLS6 71
#define ACHIEVEMENT_MOD_GOT_TOP_KILLS7 72
#define ACHIEVEMENT_MOD_GOT_TOP_KILLS8 73
#define ACHIEVEMENT_MOD_GOT_TOP_KILLS9 74
#define ACHIEVEMENT_MOD_GOT_BOP_KILLS 75
#define ACHIEVEMENT_MOD_GOT_BOP_KILLS2 76
#define ACHIEVEMENT_MOD_GOT_BOP_KILLS3 77
#define ACHIEVEMENT_MOD_GOT_BOP_KILLS4 78
#define ACHIEVEMENT_MOD_GOT_BOP_KILLS5 79
#define ACHIEVEMENT_MOD_GOT_BOP_KILLS6 80
#define ACHIEVEMENT_MOD_GOT_BOP_KILLS7 81
#define ACHIEVEMENT_MOD_GOT_BOP_KILLS8 82
#define ACHIEVEMENT_MOD_GOT_BOP_KILLS9 83
#define ACHIEVEMENT_MOD_TRAINING_DAY 84
#define ACHIEVEMENT_MOD_PUMPKIN_SMASHER 85
#define ACHIEVEMENT_MOD_HAPPY_HALLOWICKED 86
#define ACHIEVEMENT_MOD_TOBE_CONTINUED 87
#define ACHIEVEMENT_MOD_WEAPON_SHED 88
#define ACHIEVEMENT_MOD_DOLL_HOUSE 89
#define ACHIEVEMENT_MOD_MEDIC_ME 90
#define ACHIEVEMENT_MOD_WEAPON_CACHE 91
#define ACHIEVEMENT_MOD_DEVELOPER 92
#define ACHIEVEMENT_MOD_BOTTLE_SMASHER 93
#define ACHIEVEMENT_MOD_GUARD_TOWER 94
#define ACHIEVEMENT_MOD_CREDIT_ROLL 95
#define ACHIEVEMENT_MOD_CREDITROLL_END 96
#define ACHIEVEMENT_MOD_DOLL_COLLECTOR1 97
#define ACHIEVEMENT_MOD_DOLL_COLLECTOR2 98
#define ACHIEVEMENT_MOD_DOLL_COLLECTOR3 99
#define ACHIEVEMENT_MOD_DOLL_COLLECTOR4 100
#define ACHIEVEMENT_MOD_DOLL_COLLECTOR5 101
#define ACHIEVEMENT_MOD_CREDIT_CONJURE 102
#define ACHIEVEMENT_MOD_CONJURE_COMPLETE 103


// (stored across sessions via FCVAR_ARCHIVE)
//Zombie Kills
ConVar zombie_kills("zombie_kills", "0", FCVAR_ARCHIVE);
//Manhack Kills
ConVar manhack_kills("manhack_kills", "0", FCVAR_ARCHIVE);
// MetroPolice kills
ConVar metropolice_kills("metropolice_kills", "0", FCVAR_ARCHIVE, "Number of MetroPolice kills");
// Combine kills
ConVar combines_kills("combines_kills", "0", FCVAR_ARCHIVE, "Number of Combine soldier kills");
// Player kills
ConVar player_kills("player_kills", "0", FCVAR_ARCHIVE, "Number of player-vs-player kills");
class CZombieKillAchievement : public CBaseAchievement
{
public:
    virtual void Init() override
    {
        SetFlags(ACH_SAVE_GLOBAL);

        int32 statVal = 0;

        if (steamapicontext &&
            steamapicontext->SteamUserStats() &&
            steamapicontext->SteamUserStats()->GetStat("zombie_kills", &statVal))
        {
            zombie_kills.SetValue(statVal);
            SetCount(statVal);

            Msg("[Achievement] Init: Steam zombie_kills = %d\n", statVal);
        }
        else
        {
            SetCount(zombie_kills.GetInt());

            Msg("[Achievement] Init: Fallback zombie_kills = %d\n",
                zombie_kills.GetInt());
        }
    }
    virtual void HandleZombieKill()
    {
        if (!IsAchieved())
        {
            IncrementCount();
            int32 newCount = GetCount();
            if (steamapicontext && steamapicontext->SteamUserStats())
            {
                steamapicontext->SteamUserStats()->SetStat("zombie_kills", newCount);
                steamapicontext->SteamUserStats()->StoreStats();
                IAchievementMgr* pBaseMgr = engine->GetAchievementMgr();
                CAchievementMgr* pMgr = dynamic_cast<CAchievementMgr*>(pBaseMgr);
                if (pMgr) pMgr->SetDirty(true);
                zombie_kills.SetValue(newCount);
            }
            // NEW: mirror stat to leaderboard
            g_LeaderboardSync.PushStatToLeaderboard("zombie_kills", "zombie_kills");
            if (newCount >= GetGoal())
            {
                Msg("[Achievement] %s completed!\n", GetName()); // Use GetName() here
            }
        }
    }

    virtual int GetSteamStat() const
    {
        if (steamapicontext && steamapicontext->SteamUserStats())
        {
            int32 val = -1;
            if (steamapicontext->SteamUserStats()->GetStat("zombie_kills", &val))
                return val;
        }
        return -1;
    }

    virtual void ForceSteamSync()
    {
        if (steamapicontext && steamapicontext->SteamUserStats())
        {
            steamapicontext->SteamUserStats()->StoreStats();
            Msg("[Achievement] ForceSteamSync executed for %s\n", GetName());
        }
    }
};

// Generic Zombie Kill Achievement Template
#define DECLARE_ZOMBIE_KILL_ACHIEVEMENT(CLASSNAME, ID, SHORTNAME, GOAL) \
class CLASSNAME : public CZombieKillAchievement \
{ \
public: \
    void Init() override \
    { \
        CZombieKillAchievement::Init(); \
        SetGoal(GOAL); \
        Msg("[Achievement] Init: Local zombie kills = %d/%d (" SHORTNAME ")\n", \
            GetCount(), GetGoal()); \
    } \
}; \
DECLARE_ACHIEVEMENT(CLASSNAME, ID, SHORTNAME, 5)
// 9400 zombies (original one)
DECLARE_ZOMBIE_KILL_ACHIEVEMENT(CAchievementModCopKills,
    ACHIEVEMENT_MOD_GOT_COP_KILLS, "MOD_GOT_COP_KILLS", 9400);

// 666 zombies
DECLARE_ZOMBIE_KILL_ACHIEVEMENT(CAchievementModCopKills2,
    ACHIEVEMENT_MOD_GOT_COP_KILLS2, "MOD_GOT_COP_KILLS2", 666);

// 333 zombies
DECLARE_ZOMBIE_KILL_ACHIEVEMENT(CAchievementModCopKills3,
    ACHIEVEMENT_MOD_GOT_COP_KILLS3, "MOD_GOT_COP_KILLS3", 333);

// 999 zombies
DECLARE_ZOMBIE_KILL_ACHIEVEMENT(CAchievementModCopKills4,
    ACHIEVEMENT_MOD_GOT_COP_KILLS4, "MOD_GOT_COP_KILLS4", 999);

// 1966 zombies
DECLARE_ZOMBIE_KILL_ACHIEVEMENT(CAchievementModCopKills5,
    ACHIEVEMENT_MOD_GOT_COP_KILLS5, "MOD_GOT_COP_KILLS5", 1966);

// 6 zombies
DECLARE_ZOMBIE_KILL_ACHIEVEMENT(CAchievementModCopKills6,
    ACHIEVEMENT_MOD_GOT_COP_KILLS6, "MOD_GOT_COP_KILLS6", 6);

// 69 zombies
DECLARE_ZOMBIE_KILL_ACHIEVEMENT(CAchievementModCopKills7,
    ACHIEVEMENT_MOD_GOT_COP_KILLS7, "MOD_GOT_COP_KILLS7", 69);

// Achievement class for crow kills (backed by Steam stat "crow_kills")
// =======================================================
// Manhack (Crow) Kill Achievement Base ? Steam stat sync
// =======================================================
class CManhackKillAchievement : public CBaseAchievement
{
public:
    virtual void HandleManhackKill()
    {
        Msg(
            "[Achievement] %s progress = %d/%d\n",
            GetName(),
            GetCount(),
            GetGoal()
        );
    }

    virtual int GetSteamStat() const
    {
        if (steamapicontext && steamapicontext->SteamUserStats())
        {
            int32 val = -1;
            if (steamapicontext->SteamUserStats()->GetStat("crow_kills", &val))
                return val;
        }
        return -1;
    }

    // NEW: initialize local count from Steam at game startup
    virtual void Init() override
    {
        SetFlags(ACH_SAVE_GLOBAL);

        int32 statVal = 0;

        if (steamapicontext &&
            steamapicontext->SteamUserStats() &&
            steamapicontext->SteamUserStats()->GetStat("crow_kills", &statVal))
        {
            manhack_kills.SetValue(statVal);

            if (GetCount() < statVal)
            {
                SetCount(statVal);
            }
        }
        else
        {
            if (GetCount() < manhack_kills.GetInt())
            {
                SetCount(manhack_kills.GetInt());
            }
        }

        Msg(
            "[Achievement] Init: %s crow count=%d goal=%d\n",
            GetName(),
            GetCount(),
            GetGoal()
        );
    }

    virtual void ForceSteamSync()
    {
        if (steamapicontext && steamapicontext->SteamUserStats())
        {
            steamapicontext->SteamUserStats()->StoreStats();
            Msg("[Achievement] ForceSteamSync executed for %s\n", GetName());
        }
    }
};

// =======================================================
// Manhack (Crow) Kill Achievement Macro
// =======================================================
#define DECLARE_MANHACK_KILL_ACHIEVEMENT(CLASSNAME, ID, SHORTNAME, GOAL) \
class CLASSNAME : public CManhackKillAchievement \
{ \
public: \
    void Init() override \
    { \
        CManhackKillAchievement::Init(); \
        SetGoal(GOAL); \
        Msg("[Achievement] Init: Local crow kills = %d/%d (" SHORTNAME ")\n", \
            GetCount(), GetGoal()); \
    } \
}; \
DECLARE_ACHIEVEMENT(CLASSNAME, ID, SHORTNAME, 5)


// =======================================================
// Manhack (Crow) Kill Achievements
// =======================================================

// 6 crows
DECLARE_MANHACK_KILL_ACHIEVEMENT(CAchievementModZopKills,
    ACHIEVEMENT_MOD_GOT_ZOP_KILLS, "MOD_GOT_ZOP_KILLS", 6);

// 33 crows
DECLARE_MANHACK_KILL_ACHIEVEMENT(CAchievementModZopKills2,
    ACHIEVEMENT_MOD_GOT_ZOP_KILLS2, "MOD_GOT_ZOP_KILLS2", 33);

// 66 crows
DECLARE_MANHACK_KILL_ACHIEVEMENT(CAchievementModZopKills3,
    ACHIEVEMENT_MOD_GOT_ZOP_KILLS3, "MOD_GOT_ZOP_KILLS3", 66);

// 222 crows
DECLARE_MANHACK_KILL_ACHIEVEMENT(CAchievementModZopKills4,
    ACHIEVEMENT_MOD_GOT_ZOP_KILLS4, "MOD_GOT_ZOP_KILLS4", 222);

// 444 crows
DECLARE_MANHACK_KILL_ACHIEVEMENT(CAchievementModZopKills5,
    ACHIEVEMENT_MOD_GOT_ZOP_KILLS5, "MOD_GOT_ZOP_KILLS5", 444);

// 666 crows
DECLARE_MANHACK_KILL_ACHIEVEMENT(CAchievementModZopKills6,
    ACHIEVEMENT_MOD_GOT_ZOP_KILLS6, "MOD_GOT_ZOP_KILLS6", 666);

// 1666 crows
DECLARE_MANHACK_KILL_ACHIEVEMENT(CAchievementModZopKills7,
    ACHIEVEMENT_MOD_GOT_ZOP_KILLS7, "MOD_GOT_ZOP_KILLS7", 1666);

// 2666 crows
DECLARE_MANHACK_KILL_ACHIEVEMENT(CAchievementModZopKills8,
    ACHIEVEMENT_MOD_GOT_ZOP_KILLS8, "MOD_GOT_ZOP_KILLS8", 2666);

// =======================================================
// MetroPolice Kill Achievement Macro
// Backed by Steam stat "metropolice_kills"
// =======================================================
class CMetropoliceKillAchievement : public CBaseAchievement
{
public:
    virtual void HandleMetropoliceKill()
    {
        if (!IsAchieved())
        {
            IncrementCount();
            int32 newCount = GetCount();

            if (steamapicontext && steamapicontext->SteamUserStats())
            {
                steamapicontext->SteamUserStats()->SetStat("metropolice_kills", newCount);
                steamapicontext->SteamUserStats()->StoreStats();

                IAchievementMgr* pBaseMgr = engine->GetAchievementMgr();
                CAchievementMgr* pMgr = dynamic_cast<CAchievementMgr*>(pBaseMgr);
                if (pMgr) pMgr->SetDirty(true);

                ConVarRef metropolice_kills("metropolice_kills");
                if (metropolice_kills.IsValid())
                    metropolice_kills.SetValue(newCount);
            }
            g_LeaderboardSync.PushStatToLeaderboard("metropolice_kills", "metropolice_kills");
            if (newCount >= GetGoal())
            {
                Msg("[Achievement] %s completed!\n", GetName());
            }
        }
    }

    virtual void Init() override
    {
        SetFlags(ACH_SAVE_GLOBAL);

        int32 statVal = 0;
        if (steamapicontext && steamapicontext->SteamUserStats() &&
            steamapicontext->SteamUserStats()->GetStat("metropolice_kills", &statVal))
        {
            SetCount(statVal);
            ConVarRef metropolice_kills("metropolice_kills");
            if (metropolice_kills.IsValid())
                metropolice_kills.SetValue(statVal);
        }
        else
        {
            ConVarRef metropolice_kills("metropolice_kills");
            if (metropolice_kills.IsValid())
                SetCount(metropolice_kills.GetInt());
        }

        Msg("[Achievement] Init: Local metropolice kills = %d\n", GetCount());
    }

    virtual void ForceSteamSync()
    {
        if (steamapicontext && steamapicontext->SteamUserStats())
        {
            steamapicontext->SteamUserStats()->StoreStats();
            Msg("[Achievement] ForceSteamSync executed for %s\n", GetName());
        }
    }
};


// =======================================================
// MetroPolice Kill Achievement Macro
// Inherits from CMetropoliceKillAchievement so each class
// automatically supports HandleMetropoliceKill()
// =======================================================
#define DECLARE_METROPOLICE_KILL_ACHIEVEMENT(CLASSNAME, ID, SHORTNAME, GOAL) \
class CLASSNAME : public CMetropoliceKillAchievement \
{ \
public: \
    void Init() override \
    { \
        CMetropoliceKillAchievement::Init(); \
        SetGoal(GOAL); \
        Msg("[Achievement] Init: %s goal set to %d\n", SHORTNAME, GOAL); \
    } \
}; \
DECLARE_ACHIEVEMENT(CLASSNAME, ID, SHORTNAME, 5)


// =======================================================
// MetroPolice Kill Achievements
// =======================================================

// 6 kills
DECLARE_METROPOLICE_KILL_ACHIEVEMENT(CAchievementModPopKills,
    ACHIEVEMENT_MOD_GOT_POP_KILLS, "MOD_GOT_POP_KILLS", 6);

// 66 kills
DECLARE_METROPOLICE_KILL_ACHIEVEMENT(CAchievementModPopKills2,
    ACHIEVEMENT_MOD_GOT_POP_KILLS2, "MOD_GOT_POP_KILLS2", 66);

// 333 kills
DECLARE_METROPOLICE_KILL_ACHIEVEMENT(CAchievementModPopKills3,
    ACHIEVEMENT_MOD_GOT_POP_KILLS3, "MOD_GOT_POP_KILLS3", 333);

// 666 kills
DECLARE_METROPOLICE_KILL_ACHIEVEMENT(CAchievementModPopKills4,
    ACHIEVEMENT_MOD_GOT_POP_KILLS4, "MOD_GOT_POP_KILLS4", 666);

// 999 kills
DECLARE_METROPOLICE_KILL_ACHIEVEMENT(CAchievementModPopKills5,
    ACHIEVEMENT_MOD_GOT_POP_KILLS5, "MOD_GOT_POP_KILLS5", 999);

// 1666 kills
DECLARE_METROPOLICE_KILL_ACHIEVEMENT(CAchievementModPopKills6,
    ACHIEVEMENT_MOD_GOT_POP_KILLS6, "MOD_GOT_POP_KILLS6", 1666);

// =======================================================
// Combine Kill Achievement Base
// =======================================================
class CCombineKillAchievement : public CBaseAchievement
{
public:
    void Init() override
    {
        SetFlags(ACH_SAVE_GLOBAL);
        SetStoreProgressInSteam(true);

        ConVarRef combines_kills("combines_kills");
        if (combines_kills.IsValid())
            SetCount(combines_kills.GetInt());
    }

    void HandleCombineKill()
    {
        if (IsAchieved())
            return;

        IncrementCount();

        ConVarRef combines_kills("combines_kills");
        if (combines_kills.IsValid())
            combines_kills.SetValue(GetCount());

        if (steamapicontext && steamapicontext->SteamUserStats())
        {
            steamapicontext->SteamUserStats()->SetStat("combines_kills", GetCount());
            steamapicontext->SteamUserStats()->StoreStats();
            g_LeaderboardSync.PushStatToLeaderboard("combines_kills", "combines_kills");
        }
    }

    void ForceSteamSync()
    {
        if (steamapicontext && steamapicontext->SteamUserStats())
        {
            int val = 0;
            if (steamapicontext->SteamUserStats()->GetStat("combines_kills", &val))
            {
                SetCount(val);
                ConVarRef combines_kills("combines_kills");
                if (combines_kills.IsValid())
                    combines_kills.SetValue(val);
            }
        }
    }
};
#define DECLARE_COMBINE_KILL_ACHIEVEMENT(CLASSNAME, ID, SHORTNAME, GOAL) \
class CLASSNAME : public CCombineKillAchievement \
{ \
public: \
    void Init() override \
    { \
        CCombineKillAchievement::Init(); \
        SetGoal(GOAL); \
    } \
}; \
DECLARE_ACHIEVEMENT(CLASSNAME, ID, SHORTNAME, 5)

// 6 kills
DECLARE_COMBINE_KILL_ACHIEVEMENT(CAchievementModTopKills,
    ACHIEVEMENT_MOD_GOT_TOP_KILLS, "MOD_GOT_TOP_KILLS", 6);

// 88 kills
DECLARE_COMBINE_KILL_ACHIEVEMENT(CAchievementModTopKills2,
    ACHIEVEMENT_MOD_GOT_TOP_KILLS2, "MOD_GOT_TOP_KILLS2", 88);

// 222 kills
DECLARE_COMBINE_KILL_ACHIEVEMENT(CAchievementModTopKills3,
    ACHIEVEMENT_MOD_GOT_TOP_KILLS3, "MOD_GOT_TOP_KILLS3", 222);

// 444 kills
DECLARE_COMBINE_KILL_ACHIEVEMENT(CAchievementModTopKills4,
    ACHIEVEMENT_MOD_GOT_TOP_KILLS4, "MOD_GOT_TOP_KILLS4", 444);

// 666 kills
DECLARE_COMBINE_KILL_ACHIEVEMENT(CAchievementModTopKills5,
    ACHIEVEMENT_MOD_GOT_TOP_KILLS5, "MOD_GOT_TOP_KILLS5", 666);

// 1111 kills
DECLARE_COMBINE_KILL_ACHIEVEMENT(CAchievementModTopKills6,
    ACHIEVEMENT_MOD_GOT_TOP_KILLS6, "MOD_GOT_TOP_KILLS6", 1111);

// 2222 kills
DECLARE_COMBINE_KILL_ACHIEVEMENT(CAchievementModTopKills7,
    ACHIEVEMENT_MOD_GOT_TOP_KILLS7, "MOD_GOT_TOP_KILLS7", 2222);

// 3333 kills
DECLARE_COMBINE_KILL_ACHIEVEMENT(CAchievementModTopKills8,
    ACHIEVEMENT_MOD_GOT_TOP_KILLS8, "MOD_GOT_TOP_KILLS8", 3333);

// 6666 kills
DECLARE_COMBINE_KILL_ACHIEVEMENT(CAchievementModTopKills9,
    ACHIEVEMENT_MOD_GOT_TOP_KILLS9, "MOD_GOT_TOP_KILLS9", 6666);

// =======================================================
// Player Kill Achievement Base
// =======================================================
class CPlayerKillAchievement : public CBaseAchievement
{
public:
    void Init() override
    {
        SetFlags(ACH_SAVE_GLOBAL);
        SetStoreProgressInSteam(true);

        ConVarRef player_kills("player_kills");
        if (player_kills.IsValid())
            SetCount(player_kills.GetInt());
    }

    // Public wrapper so client code can safely increment
    // Player kill count is owned by player_kill_increment.
    // This function only logs current synced progress.
    void HandlePlayerKill()
    {
        Msg("[Achievement] %s progress = %d/%d\n",
            GetName(),
            GetCount(),
            GetGoal());
    }
};

// =======================================================
// Macro to define Player Kill Achievements
// =======================================================
#define DECLARE_PLAYER_KILL_ACHIEVEMENT(CLASSNAME, ID, SHORTNAME, GOAL) \
class CLASSNAME : public CPlayerKillAchievement \
{ \
public: \
    void Init() override \
    { \
        CPlayerKillAchievement::Init(); \
        SetGoal(GOAL); \
    } \
}; \
DECLARE_ACHIEVEMENT(CLASSNAME, ID, SHORTNAME, 5)

// =======================================================
// Define Achievements
// =======================================================
// 6 kills
DECLARE_PLAYER_KILL_ACHIEVEMENT(CAchievementModBopKills,
    ACHIEVEMENT_MOD_GOT_BOP_KILLS, "MOD_GOT_BOP_KILLS", 6);

// 69 kills
DECLARE_PLAYER_KILL_ACHIEVEMENT(CAchievementModBopKills2,
    ACHIEVEMENT_MOD_GOT_BOP_KILLS2, "MOD_GOT_BOP_KILLS2", 69);

// 333 kills
DECLARE_PLAYER_KILL_ACHIEVEMENT(CAchievementModBopKills3,
    ACHIEVEMENT_MOD_GOT_BOP_KILLS3, "MOD_GOT_BOP_KILLS3", 333);

// 666 kills
DECLARE_PLAYER_KILL_ACHIEVEMENT(CAchievementModBopKills4,
    ACHIEVEMENT_MOD_GOT_BOP_KILLS4, "MOD_GOT_BOP_KILLS4", 666);

// 999 kills
DECLARE_PLAYER_KILL_ACHIEVEMENT(CAchievementModBopKills5,
    ACHIEVEMENT_MOD_GOT_BOP_KILLS5, "MOD_GOT_BOP_KILLS5", 999);

// 1666 kills
DECLARE_PLAYER_KILL_ACHIEVEMENT(CAchievementModBopKills6,
    ACHIEVEMENT_MOD_GOT_BOP_KILLS6, "MOD_GOT_BOP_KILLS6", 1666);

// 2666 kills
DECLARE_PLAYER_KILL_ACHIEVEMENT(CAchievementModBopKills7,
    ACHIEVEMENT_MOD_GOT_BOP_KILLS7, "MOD_GOT_BOP_KILLS7", 2666);

// 3666 kills
DECLARE_PLAYER_KILL_ACHIEVEMENT(CAchievementModBopKills8,
    ACHIEVEMENT_MOD_GOT_BOP_KILLS8, "MOD_GOT_BOP_KILLS8", 3666);

// 6666 kills
DECLARE_PLAYER_KILL_ACHIEVEMENT(CAchievementModBopKills9,
    ACHIEVEMENT_MOD_GOT_BOP_KILLS9, "MOD_GOT_BOP_KILLS9", 6666);


//-----------------------------------------------------------------------------
// Purpose: Unlocks automatically every year on October 25th (Steam UTC)
//-----------------------------------------------------------------------------
class CAchievementModHappyHallowicked : public CBaseAchievement
{
public:
    void Init() override
    {
        SetFlags(ACH_SAVE_GLOBAL);
        SetGoal(1);
        SetCount(0); // Source SDK 2013 uses SetCount instead of SetProgress
        SetHideUntilAchieved(false);

        Msg("[Achievement Init] Happy Hallowicked initialized - checking Steam UTC date soon.\n");

        // Schedule first check after 10 seconds
        CAchievementMgr* pMgr = dynamic_cast<CAchievementMgr*>(engine->GetAchievementMgr());
        if (pMgr)
            pMgr->SetAchievementThink(this, gpGlobals->curtime + 10.0f);
    }

    void Think() override
    {
        int month = -1, day = -1;
        bool bUsedFallback = false;

#ifndef NO_STEAM
        if (steamapicontext && steamapicontext->SteamUtils())
        {
            uint32 unixTime = steamapicontext->SteamUtils()->GetServerRealTime();
            time_t now = static_cast<time_t>(unixTime);
            struct tm gmTime = { 0 };

#if defined(_WIN32)
            gmtime_s(&gmTime, &now);
#else
            gmtime_r(&now, &gmTime);
#endif

            month = gmTime.tm_mon + 1;
            day = gmTime.tm_mday;
        }
        else
#endif
        {
            // Steam API not available ? fallback to local system time
            time_t now = time(nullptr);
            struct tm localTime = { 0 };
#if defined(_WIN32)
            localtime_s(&localTime, &now);
#else
            localtime_r(&now, &localTime);
#endif
            month = localTime.tm_mon + 1;
            day = localTime.tm_mday;
            bUsedFallback = true;
        }

        if (month == 10 && day == 31)
        {
            if (!IsAchieved())
            {
                AwardAchievement();
                if (bUsedFallback)
                    Msg("[Achievement] Unlocked: Happy Hallowicked (Local System Date Fallback)\n");
                else
                    Msg("[Achievement] Unlocked: Happy Hallowicked (Steam UTC Date)\n");
            }
        }

        // Recheck every 6 hours
        CAchievementMgr* pMgr = dynamic_cast<CAchievementMgr*>(engine->GetAchievementMgr());
        if (pMgr)
            pMgr->SetAchievementThink(this, gpGlobals->curtime + (60 * 60 * 6));
    }
};
DECLARE_ACHIEVEMENT(CAchievementModHappyHallowicked, ACHIEVEMENT_MOD_HAPPY_HALLOWICKED, "MOD_HAPPY_HALLOWICKED", 5)


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

// deadend drive crane achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_PUMPKIN_SMASHER, "MOD_PUMPKIN_SMASHER", 5);

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

// bulletgallery weapon inspector achievement
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_TRAINING_DAY, "MOD_TRAINING_DAY", 5);

// secret achievement secret buck hunter bulletgallery
DECLARE_MAP_EVENT_ACHIEVEMENT_HIDDEN(ACHIEVEMENT_MOD_BUCK_HUNTER, "MOD_BUCK_HUNTER", 5);

// secret achievement secret duck hunter bulletgallery
DECLARE_MAP_EVENT_ACHIEVEMENT_HIDDEN(ACHIEVEMENT_MOD_DUCK_HUNTER, "MOD_DUCK_HUNTER", 5);

// secret achievement secret plunderbread storyline
DECLARE_MAP_EVENT_ACHIEVEMENT_HIDDEN(ACHIEVEMENT_MOD_HOLY_BREAD, "MOD_HOLY_BREAD", 5);

// to be continued storyline
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_TOBE_CONTINUED, "MOD_TOBE_CONTINUED", 5);

// grave mistakes weapon shed entry
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_WEAPON_SHED, "MOD_WEAPON_SHED", 5);

// grave mistakes enter doll house
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_DOLL_HOUSE, "MOD_DOLL_HOUSE", 5);

// open medical kit cache
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_MEDIC_ME, "MOD_MEDIC_ME", 5);

// grave mistakes open weapon cache
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_WEAPON_CACHE, "MOD_WEAPON_CACHE", 5);

// launch tools assetts map
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_DEVELOPER, "MOD_DEVELOPER", 5);

// tbullet gallery bottle smasher
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_BOTTLE_SMASHER, "MOD_BOTTLE_SMASHER", 5);

// bullet gallery climb guard tower
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_GUARD_TOWER, "MOD_GUARD_TOWER", 5);

// go to credits
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_CREDIT_ROLL, "MOD_CREDIT_ROLL", 5);

// credits survived
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_CREDITROLL_END, "MOD_CREDITROLL_END", 5);

// credit doll collecter
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_DOLL_COLLECTOR1, "MOD_DOLL_COLLECTOR1", 5);
// credit doll collecter
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_DOLL_COLLECTOR2, "MOD_DOLL_COLLECTOR2", 5);
// credit doll collecter
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_DOLL_COLLECTOR3, "MOD_DOLL_COLLECTOR3", 5);
// credit doll collecter
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_DOLL_COLLECTOR4, "MOD_DOLL_COLLECTOR4", 5);
// credit doll collecter complete
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_DOLL_COLLECTOR5, "MOD_DOLL_COLLECTOR5", 5);
// credit doll collecter escape
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_CREDIT_CONJURE, "MOD_CREDIT_CONJURE", 5);
// credit completed
DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_MOD_CONJURE_COMPLETE, "MOD_CONJURE_COMPLETE", 5);

CON_COMMAND(zombie_kill_increment, "Increment zombie kill count for all zombie kill achievements")
{
    IAchievementMgr* pBaseMgr = engine->GetAchievementMgr();
    CAchievementMgr* pAchievementMgr = dynamic_cast<CAchievementMgr*>(pBaseMgr);

    if (!pAchievementMgr)
    {
        Msg("[Console] ERROR: Could not get main achievement manager!\n");
        return;
    }

    // List of all zombie-kill achievement names
    const char* zombieAchievements[] = {
        "MOD_GOT_COP_KILLS",
        "MOD_GOT_COP_KILLS2",
        "MOD_GOT_COP_KILLS3",
        "MOD_GOT_COP_KILLS4",
        "MOD_GOT_COP_KILLS5",
        "MOD_GOT_COP_KILLS6",
        "MOD_GOT_COP_KILLS7"
    };

    Msg("[Console] Incrementing zombie kill across all zombie achievements...\n");

    for (int i = 0; i < ARRAYSIZE(zombieAchievements); i++)
    {
        const char* achName = zombieAchievements[i];

        CBaseAchievement* pAchievement = pAchievementMgr->GetAchievementByName(achName);
        auto* pZombieAchievement = dynamic_cast<CZombieKillAchievement*>(pAchievement);

        if (pZombieAchievement)
        {
            Msg("[Console] Before increment: %d/%d (%s)\n",
                pZombieAchievement->GetCount(), pZombieAchievement->GetGoal(), achName);

            pZombieAchievement->HandleZombieKill();

            Msg("[Console] After increment: %d/%d (%s)\n",
                pZombieAchievement->GetCount(), pZombieAchievement->GetGoal(), achName);
        }
        else
        {
            Msg("[Console] ERROR: Could not find zombie achievement %s!\n", achName);
        }
    }
}

CON_COMMAND(zombie_kill_debug, "Debug all zombie kill achievements and Steam stat")
{
    IAchievementMgr* pBaseMgr = engine->GetAchievementMgr();
    CAchievementMgr* pAchievementMgr = dynamic_cast<CAchievementMgr*>(pBaseMgr);

    if (!pAchievementMgr)
    {
        Msg("[Debug] ERROR: Could not get main achievement manager!\n");
        return;
    }

    const char* zombieAchievements[] = {
        "MOD_GOT_COP_KILLS",
        "MOD_GOT_COP_KILLS2",
        "MOD_GOT_COP_KILLS3",
        "MOD_GOT_COP_KILLS4",
        "MOD_GOT_COP_KILLS5",
        "MOD_GOT_COP_KILLS6",
        "MOD_GOT_COP_KILLS7"
    };

    Msg("[Debug] ===== Zombie Kill Achievements =====\n");

    for (int i = 0; i < ARRAYSIZE(zombieAchievements); i++)
    {
        const char* achName = zombieAchievements[i];

        CBaseAchievement* pAchievement = pAchievementMgr->GetAchievementByName(achName);
        auto* pZombieAchievement = dynamic_cast<CZombieKillAchievement*>(pAchievement);

        if (pZombieAchievement)
        {
            int32 achievementCount = pZombieAchievement->GetCount();
            int32 steamStat = pZombieAchievement->GetSteamStat();
            int32 convarValue = zombie_kills.GetInt();

            Msg("[Debug][%s] Count: %d/%d\n",
                achName, achievementCount, pZombieAchievement->GetGoal());

            if (steamStat >= 0)
                Msg("[Debug][%s] Steam stat: %d\n", achName, steamStat);
            else
                Msg("[Debug][%s] Steam stat: INVALID\n", achName);

            Msg("[Debug][%s] ConVar value: %d\n", achName, convarValue);
            Msg("[Debug][%s] Status: %s\n",
                achName, pZombieAchievement->IsAchieved() ? "COMPLETE" : "IN PROGRESS");

            if (steamStat >= 0 && achievementCount != steamStat)
            {
                Msg("[Debug][%s] WARNING: Local count (%d) != Steam stat (%d)\n",
                    achName, achievementCount, steamStat);
            }
            else if (steamStat >= 0)
            {
                Msg("[Debug][%s] Local and Steam are synced\n", achName);
            }
        }
        else
        {
            Msg("[Debug] ERROR: Could not find achievement %s!\n", achName);
        }
    }
}

CON_COMMAND(zombie_kill_sync, "Force sync all zombie kill achievements with Steam")
{
    IAchievementMgr* pBaseMgr = engine->GetAchievementMgr();
    CAchievementMgr* pAchievementMgr = dynamic_cast<CAchievementMgr*>(pBaseMgr);

    if (!pAchievementMgr)
    {
        Msg("[Console] ERROR: Could not get main achievement manager!\n");
        return;
    }

    const char* zombieAchievements[] = {
        "MOD_GOT_COP_KILLS",
        "MOD_GOT_COP_KILLS2",
        "MOD_GOT_COP_KILLS3",
        "MOD_GOT_COP_KILLS4",
        "MOD_GOT_COP_KILLS5",
        "MOD_GOT_COP_KILLS6",
        "MOD_GOT_COP_KILLS7"
    };

    Msg("[Console] Forcing Steam sync on all zombie kill achievements...\n");

    for (int i = 0; i < ARRAYSIZE(zombieAchievements); i++)
    {
        const char* achName = zombieAchievements[i];

        CBaseAchievement* pAchievement = pAchievementMgr->GetAchievementByName(achName);
        auto* pZombieAchievement = dynamic_cast<CZombieKillAchievement*>(pAchievement);

        if (pZombieAchievement)
        {
            pZombieAchievement->ForceSteamSync();
            Msg("[Console] Sync complete for %s (%d/%d)\n",
                achName, pZombieAchievement->GetCount(), pZombieAchievement->GetGoal());
        }
        else
        {
            Msg("[Console] ERROR: Could not find zombie achievement %s!\n", achName);
        }
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

    const char* zombieAchievements[] = {
        "MOD_GOT_COP_KILLS",
        "MOD_GOT_COP_KILLS2",
        "MOD_GOT_COP_KILLS3",
        "MOD_GOT_COP_KILLS4",
        "MOD_GOT_COP_KILLS5",
        "MOD_GOT_COP_KILLS6",
        "MOD_GOT_COP_KILLS7"
    };

    for (int i = 0; i < ARRAYSIZE(zombieAchievements); i++)
    {
        const char* achName = zombieAchievements[i];

        CBaseAchievement* pAchievement = pAchievementMgr->GetAchievementByName(achName);
        auto* pZombieAchievement = dynamic_cast<CZombieKillAchievement*>(pAchievement);

        if (pZombieAchievement && !pZombieAchievement->IsAchieved())
        {
            Msg("[Achievement] Processing kill for %s (current: %d/%d)\n",
                achName, pZombieAchievement->GetCount(), pZombieAchievement->GetGoal());

            pZombieAchievement->HandleZombieKill();
        }
    }
}

// =======================================================
// Client-side crow (manhack) kill increment handler
// =======================================================
CON_COMMAND_F(manhack_kill_increment,
    "Increment crow (manhack) kill achievement progress",
    FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    int32 steamCount = 0;
    int32 localCount = manhack_kills.GetInt();

    if (steamapicontext && steamapicontext->SteamUserStats())
    {
        steamapicontext->SteamUserStats()->GetStat("crow_kills", &steamCount);
    }

    int32 baseCount = MAX(localCount, steamCount);
    int32 newCount = baseCount + 1;

    manhack_kills.SetValue(newCount);

    if (steamapicontext && steamapicontext->SteamUserStats())
    {
        steamapicontext->SteamUserStats()->SetStat("crow_kills", newCount);
        steamapicontext->SteamUserStats()->StoreStats();

        IAchievementMgr* pBaseMgr = engine->GetAchievementMgr();
        CAchievementMgr* pMgr = dynamic_cast<CAchievementMgr*>(pBaseMgr);
        if (pMgr)
            pMgr->SetDirty(true);
    }

    IAchievementMgr* pBaseMgr = engine->GetAchievementMgr();
    CAchievementMgr* pAchievementMgr =
        dynamic_cast<CAchievementMgr*>(pBaseMgr);

    if (pAchievementMgr)
    {
        int count = pAchievementMgr->GetAchievementCount();

        for (int i = 0; i < count; ++i)
        {
            IAchievement* pAchievement =
                pAchievementMgr->GetAchievementByIndex(i);

            CManhackKillAchievement* pCrowAch =
                dynamic_cast<CManhackKillAchievement*>(pAchievement);

            if (pCrowAch)
            {
                pCrowAch->SetCount(newCount);

                Msg("[Achievement] Synced %s to %d/%d\n",
                    pCrowAch->GetName(),
                    pCrowAch->GetCount(),
                    pCrowAch->GetGoal());

                if (!pCrowAch->IsAchieved() && newCount >= pCrowAch->GetGoal())
                {
                    Msg("[Achievement] Unlocking %s from crow_kills=%d\n",
                        pCrowAch->GetName(),
                        newCount);

                    pAchievementMgr->AwardAchievement(pCrowAch->GetAchievementID());
                }
            }
        }
    }

    g_LeaderboardSync.PushStatToLeaderboard("crow_kills", "crow_kills");

    Msg("[Achievement] Crow kill total now %d (local=%d steam=%d)\n",
        newCount, localCount, steamCount);
}

// =======================================================
// Client-side MetroPolice (Guard) kill increment handler
// =======================================================
CON_COMMAND_F(metropolice_kill_increment, "Increment MetroPolice (guard) kill achievement progress", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    int count = g_AchievementMgrMod.GetAchievementCount();
    for (int i = 0; i < count; ++i)
    {
        IAchievement* pAchievement = g_AchievementMgrMod.GetAchievementByIndex(i);
        if (!pAchievement)
            continue;

        // Only update our MetroPolice kill achievements
        CMetropoliceKillAchievement* pPopAch = dynamic_cast<CMetropoliceKillAchievement*>(pAchievement);
        if (pPopAch)
        {
            pPopAch->HandleMetropoliceKill();
        }
    }

    Msg("[Achievement] MetroPolice kill increment processed client-side\n");

#ifdef CLIENT_DLL
    g_LeaderboardSync.PushStatToLeaderboard("metropolice_kills", "metropolice_kills");
#endif
}

CON_COMMAND_F(combine_kill_increment, "Increment Combine soldier kill achievements", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    int count = g_AchievementMgrMod.GetAchievementCount();
    for (int i = 0; i < count; ++i)
    {
        IAchievement* pAchievement = g_AchievementMgrMod.GetAchievementByIndex(i);
        if (!pAchievement)
            continue;

        if (Q_stristr(pAchievement->GetName(), "MOD_GOT_TOP_KILLS") != nullptr)
        {
            CCombineKillAchievement* pCombineAch = dynamic_cast<CCombineKillAchievement*>(pAchievement);
            if (pCombineAch)
                pCombineAch->HandleCombineKill();
        }
    }

    Msg("[Achievement] Combine soldier kill increment processed\n");

#ifdef CLIENT_DLL
    g_LeaderboardSync.PushStatToLeaderboard("combines_kills", "combines_kills");
#endif
}

// =======================================================
// Client-side Player Kill increment handler
// =======================================================
CON_COMMAND_F(player_kill_increment,
    "Increment Player kill achievement progress",
    FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    int32 steamCount = 0;
    int32 localCount = player_kills.GetInt();

    if (steamapicontext && steamapicontext->SteamUserStats())
    {
        steamapicontext->SteamUserStats()->GetStat("player_kills", &steamCount);
    }

    int32 baseCount = MAX(localCount, steamCount);
    int32 newCount = baseCount + 1;

    player_kills.SetValue(newCount);

    if (steamapicontext && steamapicontext->SteamUserStats())
    {
        steamapicontext->SteamUserStats()->SetStat("player_kills", newCount);
        steamapicontext->SteamUserStats()->StoreStats();

        IAchievementMgr* pBaseMgr = engine->GetAchievementMgr();
        CAchievementMgr* pMgr =
            dynamic_cast<CAchievementMgr*>(pBaseMgr);

        if (pMgr)
            pMgr->SetDirty(true);
    }

    IAchievementMgr* pBaseMgr = engine->GetAchievementMgr();
    CAchievementMgr* pAchievementMgr =
        dynamic_cast<CAchievementMgr*>(pBaseMgr);

    if (pAchievementMgr)
    {
        int achievementCount =
            pAchievementMgr->GetAchievementCount();

        for (int i = 0; i < achievementCount; ++i)
        {
            IAchievement* pAchievement =
                pAchievementMgr->GetAchievementByIndex(i);

            CPlayerKillAchievement* pPlayerAch =
                dynamic_cast<CPlayerKillAchievement*>(pAchievement);

            if (pPlayerAch)
            {
                pPlayerAch->SetCount(newCount);

                Msg("[Achievement] Synced %s to %d/%d\n",
                    pPlayerAch->GetName(),
                    pPlayerAch->GetCount(),
                    pPlayerAch->GetGoal());

                if (!pPlayerAch->IsAchieved() &&
                    newCount >= pPlayerAch->GetGoal())
                {
                    Msg("[Achievement] Unlocking %s from player_kills=%d\n",
                        pPlayerAch->GetName(),
                        newCount);

                    pAchievementMgr->AwardAchievement(
                        pPlayerAch->GetAchievementID());
                }
            }
        }
    }

#ifdef CLIENT_DLL
    g_LeaderboardSync.PushStatToLeaderboard(
        "player_kills",
        "player_kills");
#endif

    Msg("[Achievement] Player kill total now %d (local=%d steam=%d)\n",
        newCount,
        localCount,
        steamCount);
}

#endif // GAME_DLL