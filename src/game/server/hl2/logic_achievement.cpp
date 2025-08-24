//========= Copyright Valve Corporation, All rights reserved. ============//
//
//  Defines a logical entity which passes achievement related events to the gamerules system.
//

#include "cbase.h"
#include "gamerules.h"
#include "entityinput.h"
#include "entityoutput.h"
#include "logic_achievement.h"   // our header

#ifndef CLIENT_DLL
#include "hl2mp_player.h"   // server-side player class
#include "tier1/utlstring.h"
#include "tier1/utlvector.h"
#include "eiface.h"         // IVEngineServer / engine->GetPlayerInfo
#include "player.h"         // for CBasePlayer
extern IVEngineServer* engine;
void ServerAwardAchievement(CBasePlayer* pPlayer, const char* pchAchievementName);
#endif

#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(logic_achievement, CLogicAchievement);

BEGIN_DATADESC(CLogicAchievement)

DEFINE_KEYFIELD(m_bDisabled, FIELD_BOOLEAN, "StartDisabled"),
DEFINE_KEYFIELD(m_iszAchievementEventID, FIELD_STRING, "AchievementEvent"),
DEFINE_KEYFIELD(m_bAutoFire, FIELD_BOOLEAN, "AutoFireOnMapSpawn"), // NEW

DEFINE_INPUTFUNC(FIELD_VOID, "FireEvent", InputFireEvent),
DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),
DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle),

DEFINE_OUTPUT(m_OnFired, "OnFired"),

END_DATADESC()

#define ACHIEVEMENT_PREFIX "ACHIEVEMENT_EVENT_"

#ifndef CLIENT_DLL
// Minimal definition of player_info_t since "player_info.h" is not available
struct player_info_s
{
    char            name[128];
    int             userID;
    char            guid[33];
    unsigned int    friendsID;
    char            friendsName[128];
    bool            fakeplayer;
    bool            ishltv;
    unsigned int    customFiles[4];
    unsigned char   filesDownloaded;
};
typedef struct player_info_s player_info_t;
static bool IsValidHumanPlayer(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return false;

    player_info_t pi = {};
    if (!engine->GetPlayerInfo(pPlayer->entindex(), &pi))
        return false;

    if (pi.fakeplayer || pi.ishltv)
        return false;

    if (!pPlayer->IsConnected())
        return false;

    // --- Listen server override ---
    if (gpGlobals->maxClients == 1)
    {
        // Always accept the local player (even before they pick a team)
        if (UTIL_GetLocalPlayer() == pPlayer)
            return true;
    }

    // --- Dedicated server path ---
    // Instead of rejecting UNASSIGNED immediately, allow them if they are real
    if (pPlayer->GetTeamNumber() == TEAM_UNASSIGNED)
    {
        // Treat as valid — achievement system will still reach them later
        return true;
    }

    // Reject only true spectators
    if (pPlayer->GetTeamNumber() <= TEAM_SPECTATOR)
        return false;

    return true;
}

static const float kRetryDelay = 1.0f;
#endif

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CLogicAchievement::CLogicAchievement(void)
{
    m_iszAchievementEventID = NULL_STRING;
    m_bAutoFire = false;
#ifndef CLIENT_DLL
    Msg("[LogicAchievement] Spawned entity instance\n");
#endif
}

//-----------------------------------------------------------------------------
// Spawn
//-----------------------------------------------------------------------------
void CLogicAchievement::Spawn()
{
    BaseClass::Spawn();
#ifndef CLIENT_DLL
    Msg("[LogicAchievement] Spawn() AchievementEvent='%s' (disabled=%d, auto=%d)\n",
        (m_iszAchievementEventID != NULL_STRING) ? STRING(m_iszAchievementEventID) : "<null>",
        m_bDisabled, m_bAutoFire);
#endif
}

//-----------------------------------------------------------------------------
// Input: FireEvent
//-----------------------------------------------------------------------------
void CLogicAchievement::InputFireEvent(inputdata_t& inputdata)
{
#ifndef CLIENT_DLL
    Msg("[LogicAchievement] InputFireEvent called, disabled=%d, eventID=%s\n",
        m_bDisabled,
        (m_iszAchievementEventID != NULL_STRING) ? STRING(m_iszAchievementEventID) : "<null>");
#endif

    if (m_bDisabled || m_iszAchievementEventID == NULL_STRING)
        return;

    const char* pszEvent = STRING(m_iszAchievementEventID);
    const int nPrefixLen = Q_strlen(ACHIEVEMENT_PREFIX);

    if (Q_strnicmp(pszEvent, ACHIEVEMENT_PREFIX, nPrefixLen) == 0)
        pszEvent += nPrefixLen;

#ifndef CLIENT_DLL
    if (!pszEvent || !*pszEvent)
    {
        Warning("[LogicAchievement] Empty achievement name after prefix strip\n");
        return;
    }

    // First try activator if it's a valid player
    if (CBasePlayer* pActivatorPlayer = ToBasePlayer(inputdata.pActivator))
    {
        if (IsValidHumanPlayer(pActivatorPlayer))
        {
            ServerAwardAchievement(pActivatorPlayer, pszEvent);
            m_OnFired.FireOutput(pActivatorPlayer, this);
            Msg("[LogicAchievement] Fired: '%s' for activator %s\n",
                pszEvent, pActivatorPlayer->GetPlayerName());
            return;
        }
    }

    // Otherwise check all connected human players
    for (int i = 1; i <= gpGlobals->maxClients; ++i)
    {
        CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
        if (IsValidHumanPlayer(pPlayer))
        {
            ServerAwardAchievement(pPlayer, pszEvent);
            m_OnFired.FireOutput(pPlayer, this);
            Msg("[LogicAchievement] Fired: '%s' for player %s (index %d)\n",
                pszEvent, pPlayer->GetPlayerName(), pPlayer->entindex());
            return;
        }
    }

    // No valid players yet -> queue for retry
    Warning("[LogicAchievement] No valid players yet, queuing '%s'\n", pszEvent);
    m_sPendingEvent = pszEvent;
    if (!m_bRefirePending)
    {
        m_bRefirePending = true;
        SetContextThink(&CLogicAchievement::RetryFireThink,
            gpGlobals->curtime + kRetryDelay,
            "LogicAchievementRetryThink");
    }
#endif
}

//-----------------------------------------------------------------------------
// Enable/Disable/Toggle
//-----------------------------------------------------------------------------
void CLogicAchievement::InputEnable(inputdata_t& inputdata) { m_bDisabled = false; }
void CLogicAchievement::InputDisable(inputdata_t& inputdata) { m_bDisabled = true; }
void CLogicAchievement::InputToggle(inputdata_t& inputdata) { m_bDisabled = !m_bDisabled; }

//-----------------------------------------------------------------------------
// GetAchievementEventName
//-----------------------------------------------------------------------------
const char* CLogicAchievement::GetAchievementEventName() const
{
    return (m_iszAchievementEventID != NULL_STRING) ? STRING(m_iszAchievementEventID) : "<null>";
}

//-----------------------------------------------------------------------------
// FireEventFromLogicAuto
//-----------------------------------------------------------------------------
void CLogicAchievement::FireEventFromLogicAuto(CBaseEntity* pCaller)
{
    if (!m_bAutoFire)
        return; // only fire if flagged

    inputdata_t fakeInput;
    fakeInput.pActivator = NULL;
    fakeInput.pCaller = pCaller;
    fakeInput.value.SetInt(0);

    InputFireEvent(fakeInput);

#ifndef CLIENT_DLL
    Msg("[LogicAchievement] FireEventFromLogicAuto auto-fired '%s'\n", GetAchievementEventName());
#endif
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// RetryFireThink
//-----------------------------------------------------------------------------
void CLogicAchievement::RetryFireThink()
{
    if (!m_bRefirePending || m_sPendingEvent.IsEmpty())
    {
        m_bRefirePending = false;
        return;
    }

    CUtlVector<CBasePlayer*> humans;

    for (int i = 1; i <= gpGlobals->maxClients; ++i)
    {
        CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
        if (IsValidHumanPlayer(pPlayer))
            humans.AddToTail(pPlayer);
    }

    const char* pchName = m_sPendingEvent.String();

    // Special listen-server handling
    if (gpGlobals->maxClients == 1 && humans.Count() == 0)
    {
        if (CBasePlayer* pLocal = UTIL_GetLocalPlayer())
        {
            // Make sure the player is really in-game
            if (pLocal->IsConnected())
            {
                ServerAwardAchievement(pLocal, pchName);
                Msg("[LogicAchievement] (Listen) Fired: '%s' for local player %s\n",
                    pchName, pLocal->GetPlayerName());
                m_sPendingEvent.Clear();
                m_bRefirePending = false;
                return;
            }
        }

        // Player not ready yet retry again
        SetContextThink(&CLogicAchievement::RetryFireThink,
            gpGlobals->curtime + kRetryDelay,
            "LogicAchievementRetryThink");
        return;
    }

    // Normal dedicated path
    if (humans.Count() == 0)
    {
        SetContextThink(&CLogicAchievement::RetryFireThink,
            gpGlobals->curtime + kRetryDelay,
            "LogicAchievementRetryThink");
        return;
    }

    FOR_EACH_VEC(humans, k)
    {
        CBasePlayer* p = humans[k];
        ServerAwardAchievement(p, pchName);
        Msg("[LogicAchievement] (Deferred) Fired: '%s' for player %s (index %d)\n",
            pchName, p->GetPlayerName(), p->entindex());
    }

    m_sPendingEvent.Clear();
    m_bRefirePending = false;
}
#endif