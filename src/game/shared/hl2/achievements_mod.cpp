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


// (stored across sessions via FCVAR_ARCHIVE)
ConVar zombie_kills("zombie_kills", "0", FCVAR_ARCHIVE);
ConVar manhack_kills("manhack_kills", "0", FCVAR_ARCHIVE);

class CZombieKillAchievement : public CBaseAchievement
{
public:
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
        SetFlags(ACH_SAVE_GLOBAL); \
        SetGoal(GOAL); \
        SetCount(zombie_kills.GetInt()); \
        Msg("[Achievement] Init: Local zombie kills = %d/%d (" SHORTNAME ")\n", GetCount(), GetGoal()); \
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

// Achievement class for crow kills (backed by Steam stat "crow_kills")
// =======================================================
// Manhack (Crow) Kill Achievement Base — Steam stat sync
// =======================================================
class CManhackKillAchievement : public CBaseAchievement
{
public:
    virtual void HandleManhackKill()
    {
        if (!IsAchieved())
        {
            IncrementCount();
            int32 newCount = GetCount();

            if (steamapicontext && steamapicontext->SteamUserStats())
            {
                steamapicontext->SteamUserStats()->SetStat("crow_kills", newCount);
                steamapicontext->SteamUserStats()->StoreStats();

                IAchievementMgr* pBaseMgr = engine->GetAchievementMgr();
                CAchievementMgr* pMgr = dynamic_cast<CAchievementMgr*>(pBaseMgr);
                if (pMgr) pMgr->SetDirty(true);

                manhack_kills.SetValue(newCount);
            }

            if (newCount >= GetGoal())
            {
                Msg("[Achievement] %s completed!\n", GetName());
            }
        }
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
        if (steamapicontext && steamapicontext->SteamUserStats())
        {
            if (steamapicontext->SteamUserStats()->GetStat("crow_kills", &statVal))
            {
                manhack_kills.SetValue(statVal);
                SetCount(statVal);
            }
            else
            {
                // fallback to ConVar if Steam stat not found
                SetCount(manhack_kills.GetInt());
            }
        }
        else
        {
            SetCount(manhack_kills.GetInt());
        }

        Msg("[Achievement] Init: Local crow kills = %d (Steam sync)\n", GetCount());
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
        SetFlags(ACH_SAVE_GLOBAL); \
        SetGoal(GOAL); \
        SetCount(manhack_kills.GetInt()); \
        Msg("[Achievement] Init: Local crow kills = %d/%d (" SHORTNAME ")\n", GetCount(), GetGoal()); \
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
        "MOD_GOT_COP_KILLS6"
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
        "MOD_GOT_COP_KILLS6"
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
        "MOD_GOT_COP_KILLS6"
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
        "MOD_GOT_COP_KILLS6"
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
CON_COMMAND_F(manhack_kill_increment, "Increment crow (manhack) kill achievement progress", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    // Walk through all achievements and update any crow-related ones
    int count = g_AchievementMgrMod.GetAchievementCount();
    for (int i = 0; i < count; ++i)
    {
        IAchievement* pAchievement = g_AchievementMgrMod.GetAchievementByIndex(i);
        if (!pAchievement)
            continue;

        // Only update our crow (manhack) kill achievements
        CManhackKillAchievement* pCrowAch = dynamic_cast<CManhackKillAchievement*>(pAchievement);
        if (pCrowAch)
        {
            pCrowAch->HandleManhackKill();
        }
    }

    Msg("[Achievement] Crow (manhack) kill increment processed client-side\n");
}

#endif // GAME_DLL