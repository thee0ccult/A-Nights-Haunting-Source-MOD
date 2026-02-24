#include "cbase.h"
#include "baseanimating.h"
#include "ammodef.h"
#include "player.h"
#include "entitylist.h"

class CItemAmmoCache : public CBaseAnimating
{
public:
    DECLARE_CLASS(CItemAmmoCache, CBaseAnimating);
    DECLARE_DATADESC();

    // Required for HL2MP +USE detection
    virtual int ObjectCaps() { return BaseClass::ObjectCaps() | FCAP_IMPULSE_USE; }

    void Spawn(void);
    void Precache(void);
    void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

    void RefillPlayerAmmo(CBasePlayer* pPlayer);
    void UpdateVisualState();

    void InputRefillAmmo(inputdata_t& inputData);
    void InputRefillAllPlayers(inputdata_t& inputData);
    void InputEnable(inputdata_t& inputData);
    void InputDisable(inputdata_t& inputData);

private:
    float m_flCooldown;
    float m_flNextUseTime;

    int   m_iMaxUses;
    int   m_iUsesRemaining;

    int   m_iTeamNumber;
    bool  m_bDisabled;
    bool  m_bUseColorOverlay;

    COutputEvent m_OnUsed;
    COutputEvent m_OnOutOfUses;
};

LINK_ENTITY_TO_CLASS(item_ammo_cache, CItemAmmoCache);

BEGIN_DATADESC(CItemAmmoCache)

DEFINE_KEYFIELD(m_flCooldown, FIELD_FLOAT, "Cooldown"),
DEFINE_KEYFIELD(m_iMaxUses, FIELD_INTEGER, "MaxUses"),
DEFINE_KEYFIELD(m_iTeamNumber, FIELD_INTEGER, "TeamNumber"),
DEFINE_KEYFIELD(m_bDisabled, FIELD_BOOLEAN, "StartDisabled"),
DEFINE_KEYFIELD(m_bUseColorOverlay, FIELD_BOOLEAN, "UseColorOverlay"),

DEFINE_FIELD(m_flNextUseTime, FIELD_TIME),
DEFINE_FIELD(m_iUsesRemaining, FIELD_INTEGER),

DEFINE_USEFUNC(Use),

DEFINE_INPUTFUNC(FIELD_VOID, "RefillAmmo", InputRefillAmmo),
DEFINE_INPUTFUNC(FIELD_VOID, "RefillAllPlayers", InputRefillAllPlayers),
DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),

DEFINE_OUTPUT(m_OnUsed, "OnUsed"),
DEFINE_OUTPUT(m_OnOutOfUses, "OnOutOfUses"),

END_DATADESC()


//-----------------------------------------------------------------------------
// Precache
//-----------------------------------------------------------------------------
void CItemAmmoCache::Precache(void)
{
    if (GetModelName() != NULL_STRING)
        PrecacheModel(STRING(GetModelName()));
    else
        PrecacheModel("models/props_junk/wood_crate001a.mdl");

    PrecacheScriptSound("AmmoCrate.Open");

    BaseClass::Precache();
}


//-----------------------------------------------------------------------------
// Spawn
//-----------------------------------------------------------------------------
void CItemAmmoCache::Spawn(void)
{
    Precache();

    if (GetModelName() == NULL_STRING)
        SetModel("models/props_junk/wood_crate001a.mdl");
    else
        SetModel(STRING(GetModelName()));

    SetSolid(SOLID_BBOX);
    SetMoveType(MOVETYPE_NONE);
    SetCollisionGroup(COLLISION_GROUP_NONE);

    UTIL_SetSize(this, Vector(-16, -16, 0), Vector(16, 16, 32));

    SetUse(&CItemAmmoCache::Use);

    m_flNextUseTime = 0.0f;
    m_iUsesRemaining = (m_iMaxUses <= 0) ? -1 : m_iMaxUses;

    UpdateVisualState();
}


//-----------------------------------------------------------------------------
// Visual State
//-----------------------------------------------------------------------------
void CItemAmmoCache::UpdateVisualState()
{
    if (!m_bUseColorOverlay)
    {
        // Force true model appearance (no Hammer tint)
        SetRenderColor(255, 255, 255);
        return;
    }

    if (m_bDisabled || m_iUsesRemaining == 0)
        SetRenderColor(255, 50, 50);   // Red (inactive)
    else
        SetRenderColor(50, 255, 50);   // Green (active)
}


//-----------------------------------------------------------------------------
// Core Ammo Refill Logic
//-----------------------------------------------------------------------------
void CItemAmmoCache::RefillPlayerAmmo(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    CAmmoDef* pAmmoDef = GetAmmoDef();
    if (!pAmmoDef)
        return;

    for (int i = 0; i < MAX_AMMO_TYPES; ++i)
    {
        int maxCarry = pAmmoDef->MaxCarry(i);
        if (maxCarry > 0)
            pPlayer->SetAmmoCount(maxCarry, i);
    }

    EmitSound("AmmoCrate.Open");
}


//-----------------------------------------------------------------------------
// Use
//-----------------------------------------------------------------------------
void CItemAmmoCache::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
    if (m_bDisabled)
        return;

    if (gpGlobals->curtime < m_flNextUseTime)
        return;

    if (!pActivator || !pActivator->IsPlayer())
        return;

    CBasePlayer* pPlayer = static_cast<CBasePlayer*>(pActivator);

    if (m_iTeamNumber > 0 && pPlayer->GetTeamNumber() != m_iTeamNumber)
        return;

    if (m_iUsesRemaining == 0)
        return;

    RefillPlayerAmmo(pPlayer);

    m_OnUsed.FireOutput(pPlayer, this);

    if (m_iUsesRemaining > 0)
    {
        m_iUsesRemaining--;

        if (m_iUsesRemaining == 0)
            m_OnOutOfUses.FireOutput(pPlayer, this);
    }

    m_flNextUseTime = gpGlobals->curtime + m_flCooldown;

    UpdateVisualState();
}


//-----------------------------------------------------------------------------
// Inputs
//-----------------------------------------------------------------------------
void CItemAmmoCache::InputRefillAmmo(inputdata_t& inputData)
{
    if (inputData.pActivator && inputData.pActivator->IsPlayer())
        RefillPlayerAmmo(static_cast<CBasePlayer*>(inputData.pActivator));
}

void CItemAmmoCache::InputRefillAllPlayers(inputdata_t& inputData)
{
    for (int i = 1; i <= gpGlobals->maxClients; ++i)
    {
        CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
        if (pPlayer)
            RefillPlayerAmmo(pPlayer);
    }
}

void CItemAmmoCache::InputEnable(inputdata_t&)
{
    m_bDisabled = false;
    UpdateVisualState();
}

void CItemAmmoCache::InputDisable(inputdata_t&)
{
    m_bDisabled = true;
    UpdateVisualState();
}