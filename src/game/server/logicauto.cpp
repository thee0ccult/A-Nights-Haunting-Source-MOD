//========= Copyright Valve Corporation, All rights reserved. ============//
//
//  logicauto.cpp
//  Implements logic_auto entity, extended to trigger logic_achievement
//
//=============================================================================

#include "cbase.h"
#include "entityoutput.h"
#include "entitylist.h"
#include "logic_achievement.h"  // new include so we can talk to CLogicAchievement

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Called when a map spawns; automatically fires outputs/events
//-----------------------------------------------------------------------------
class CLogicAuto : public CLogicalEntity
{
public:
    DECLARE_CLASS(CLogicAuto, CLogicalEntity);
    DECLARE_DATADESC();

    void Spawn(void);
    void Think(void);

    void InputKill(inputdata_t& inputdata);

    COutputEvent m_OnMapSpawn;

private:
    void FireOnMapSpawn();

    bool m_bFireOnce;
    EHANDLE m_hEnt;
};

LINK_ENTITY_TO_CLASS(logic_auto, CLogicAuto);

BEGIN_DATADESC(CLogicAuto)

DEFINE_INPUTFUNC(FIELD_VOID, "Kill", InputKill),
DEFINE_OUTPUT(m_OnMapSpawn, "OnMapSpawn"),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Spawn
//-----------------------------------------------------------------------------
void CLogicAuto::Spawn(void)
{
    BaseClass::Spawn();
    SetNextThink(gpGlobals->curtime + 0.1f);

#ifndef CLIENT_DLL
    Msg("[logic_auto] Forced OnMapLoad scheduled for logic_auto\n");
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Think
//-----------------------------------------------------------------------------
void CLogicAuto::Think(void)
{
    FireOnMapSpawn();
}

//-----------------------------------------------------------------------------
// Purpose: Kill input
//-----------------------------------------------------------------------------
void CLogicAuto::InputKill(inputdata_t& inputdata)
{
    UTIL_Remove(this);
}

//-----------------------------------------------------------------------------
// Purpose: Fire OnMapSpawn outputs and trigger any logic_achievement
//-----------------------------------------------------------------------------
void CLogicAuto::FireOnMapSpawn()
{
#ifndef CLIENT_DLL
    Msg("[logic_auto] Delayed OnMapLoad fired for logic_auto\n");
#endif

    //
    // 1. Fire the normal Hammer outputs so existing map logic works
    //
    m_OnMapSpawn.FireOutput(this, this);

#ifndef CLIENT_DLL
    //
    // 2. Then find all logic_achievement entities and auto-fire them
    //
    CBaseEntity* pEnt = NULL;
    while ((pEnt = gEntList.FindEntityByClassname(pEnt, "logic_achievement")) != NULL)
    {
        CLogicAchievement* pAch = dynamic_cast<CLogicAchievement*>(pEnt);
        if (pAch && pAch->ShouldAutoFire())
        {
            pAch->FireEventFromLogicAuto(this);
            Msg("[logic_auto] Auto-fired achievement: '%s'\n", pAch->GetAchievementEventName());
        }
    }
#endif

    if (m_bFireOnce)
    {
        UTIL_Remove(this);
    }
}