//========= Copyright Valve Corporation, All rights reserved. ============//
//
//  Header for the logic_achievement entity
//
//===========================================================================//

#ifndef LOGIC_ACHIEVEMENT_H
#define LOGIC_ACHIEVEMENT_H
#ifdef _WIN32
#pragma once
#endif

#include "baseentity.h"
#include "entityoutput.h"

class CLogicAchievement : public CLogicalEntity
{
public:
    DECLARE_CLASS(CLogicAchievement, CLogicalEntity);
    DECLARE_DATADESC();

    CLogicAchievement(void);

    virtual void Spawn();

    // Inputs
    void InputFireEvent(inputdata_t& inputdata);
    void InputEnable(inputdata_t& inputdata);
    void InputDisable(inputdata_t& inputdata);
    void InputToggle(inputdata_t& inputdata);

    // Helper
    const char* GetAchievementEventName() const;

    // Called from logic_auto
    void FireEventFromLogicAuto(CBaseEntity* pCaller);
    inline bool ShouldAutoFire() const { return m_bAutoFire; }
#ifndef CLIENT_DLL
    void RetryFireThink();
#endif

private:
    // Keyvalues
    bool        m_bDisabled;             // Start disabled flag
    string_t    m_iszAchievementEventID; // Raw FGD choice string
    bool        m_bAutoFire;             // Auto-fire on map spawn

    // Outputs
    COutputEvent m_OnFired;

#ifndef CLIENT_DLL
    // Deferred fire handling
    CUtlString   m_sPendingEvent;   // Which achievement is waiting to fire
    bool         m_bRefirePending;  // Are we currently scheduled to retry?
#endif
};

#endif // LOGIC_ACHIEVEMENT_H