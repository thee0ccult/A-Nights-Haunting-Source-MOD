#include "cbase.h"
#include "baseanimating.h"
#include "player.h"
#include "usermessages.h"
#include "engine/IEngineSound.h"

#include "../shared/item_weapon_locker_shared.h"

class CItemWeaponLocker : public CBaseAnimating
{
public:
    DECLARE_CLASS(CItemWeaponLocker, CBaseAnimating);
    DECLARE_DATADESC();

    CItemWeaponLocker();

    void Spawn() override;
    void Precache() override;
    void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

    int ObjectCaps() override { return BaseClass::ObjectCaps() | FCAP_IMPULSE_USE; }

    void GiveWeaponToPlayer(CBasePlayer* pPlayer, const char* pszWeapon);

    void InputEnable(inputdata_t& data);
    void InputDisable(inputdata_t& data);
    void InputToggle(inputdata_t& data);

private:

    string_t m_iszScriptFile;

    int   m_iUsageLimit;
    int   m_iUsageCount;
    float m_flCooldown;
    float m_flNextUseTime;
    bool  m_bStartDisabled;
    bool  m_bDisabled;
    bool  m_bStripWeapons;

    COutputEvent m_OnOpen;
    COutputEvent m_OnWeaponGiven;
    COutputEvent m_OnLimitReached;
    COutputEvent m_OnCooldown;
};

LINK_ENTITY_TO_CLASS(item_weapon_locker, CItemWeaponLocker);

BEGIN_DATADESC(CItemWeaponLocker)

DEFINE_KEYFIELD(m_iszScriptFile, FIELD_STRING, "ScriptFile"),
DEFINE_KEYFIELD(m_iUsageLimit, FIELD_INTEGER, "UsageLimit"),
DEFINE_KEYFIELD(m_flCooldown, FIELD_FLOAT, "Cooldown"),
DEFINE_KEYFIELD(m_bStartDisabled, FIELD_BOOLEAN, "StartDisabled"),
DEFINE_KEYFIELD(m_bStripWeapons, FIELD_BOOLEAN, "StripWeapons"),

DEFINE_FIELD(m_iUsageCount, FIELD_INTEGER),
DEFINE_FIELD(m_flNextUseTime, FIELD_TIME),
DEFINE_FIELD(m_bDisabled, FIELD_BOOLEAN),

DEFINE_OUTPUT(m_OnOpen, "OnOpen"),
DEFINE_OUTPUT(m_OnWeaponGiven, "OnWeaponGiven"),
DEFINE_OUTPUT(m_OnLimitReached, "OnLimitReached"),
DEFINE_OUTPUT(m_OnCooldown, "OnCooldown"),

DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),
DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle),

END_DATADESC()

// ------------------------------------------------------------

CItemWeaponLocker::CItemWeaponLocker()
{
    m_iUsageLimit = 0;
    m_iUsageCount = 0;
    m_flCooldown = 0.0f;
    m_flNextUseTime = 0.0f;
    m_bStartDisabled = false;
    m_bDisabled = false;
    m_bStripWeapons = false;
}

void CItemWeaponLocker::Precache()
{
    if (GetModelName() != NULL_STRING)
        PrecacheModel(STRING(GetModelName()));
}

void CItemWeaponLocker::Spawn()
{
    Precache();

    if (GetModelName() != NULL_STRING)
    {
        SetModel(STRING(GetModelName()));
        SetSolid(SOLID_VPHYSICS);
        VPhysicsInitStatic();
    }
    else
    {
        SetSolid(SOLID_BBOX);
        UTIL_SetSize(this, -Vector(16, 16, 0), Vector(16, 16, 72));
    }

    SetMoveType(MOVETYPE_NONE);
    SetUse(&CItemWeaponLocker::Use);

    m_bDisabled = m_bStartDisabled;

    if (m_iszScriptFile == NULL_STRING)
        m_iszScriptFile = MAKE_STRING(WEAPON_LOCKER_DEFAULT_SCRIPT);
}

void CItemWeaponLocker::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
    if (m_bDisabled)
        return;

    if (m_iUsageLimit > 0 && m_iUsageCount >= m_iUsageLimit)
    {
        m_OnLimitReached.FireOutput(pActivator, this);
        return;
    }

    if (gpGlobals->curtime < m_flNextUseTime)
    {
        m_OnCooldown.FireOutput(pActivator, this);
        return;
    }

    CBasePlayer* pPlayer = ToBasePlayer(pActivator);
    if (!pPlayer || !pPlayer->IsAlive())
        return;

    m_OnOpen.FireOutput(pPlayer, this);

    CSingleUserRecipientFilter filter(pPlayer);
    filter.MakeReliable();

    UserMessageBegin(filter, WEAPON_LOCKER_OPEN_MSG);
    WRITE_SHORT(entindex());
    WRITE_STRING(STRING(m_iszScriptFile));
    MessageEnd();

    m_iUsageCount++;
    m_flNextUseTime = gpGlobals->curtime + m_flCooldown;
}

void CItemWeaponLocker::GiveWeaponToPlayer(CBasePlayer* pPlayer, const char* pszWeapon)
{
    if (m_bStripWeapons)
        pPlayer->RemoveAllItems(true);

    CBaseEntity* pEnt = CreateEntityByName(pszWeapon);
    if (!pEnt)
        return;

    DispatchSpawn(pEnt);

    CBaseCombatWeapon* pNewWeapon = dynamic_cast<CBaseCombatWeapon*>(pEnt);
    if (!pNewWeapon)
    {
        UTIL_Remove(pEnt);
        return;
    }

    // Bucket replacement
    int bucket = pNewWeapon->GetSlot();

    CBaseCombatWeapon* pExisting = pPlayer->Weapon_GetSlot(bucket);

    if (pExisting)
    {
        pPlayer->Weapon_Drop(pExisting, NULL, NULL);
        UTIL_Remove(pExisting);
    }

    pPlayer->Weapon_Equip(pNewWeapon);

    m_OnWeaponGiven.FireOutput(pPlayer, this);
}

void CItemWeaponLocker::InputEnable(inputdata_t&) { m_bDisabled = false; }
void CItemWeaponLocker::InputDisable(inputdata_t&) { m_bDisabled = true; }
void CItemWeaponLocker::InputToggle(inputdata_t&) { m_bDisabled = !m_bDisabled; }


// ------------------------------------------------------------
// Console command
// ------------------------------------------------------------

CON_COMMAND(anh_select_weapon, "Select weapon from locker")
{
    if (args.ArgC() < 3)
        return;

    CBasePlayer* pPlayer = UTIL_GetCommandClient();
    if (!pPlayer || !pPlayer->IsAlive())
        return;

    int entIndex = atoi(args[1]);
    const char* pszWeapon = args[2];

    CBaseEntity* pEnt = CBaseEntity::Instance(entIndex);
    CItemWeaponLocker* pLocker = dynamic_cast<CItemWeaponLocker*>(pEnt);

    if (!pLocker)
        return;

    pLocker->GiveWeaponToPlayer(pPlayer, pszWeapon);
}