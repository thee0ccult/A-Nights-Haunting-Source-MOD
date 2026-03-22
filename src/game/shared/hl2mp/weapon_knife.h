//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef HL2MP_WEAPON_CROWBAR_H
#define HL2MP_WEAPON_CROWBAR_H
#pragma once


#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "weapon_hl2mpbasebasebludgeon.h"


#ifdef CLIENT_DLL
#define CWeaponKnife C_WeaponKnife
#endif

//-----------------------------------------------------------------------------
// CWeaponKnife
//-----------------------------------------------------------------------------

class CWeaponKnife : public CBaseHL2MPBludgeonWeapon
{
public:
	DECLARE_CLASS( CWeaponKnife, CBaseHL2MPBludgeonWeapon );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL
	DECLARE_ACTTABLE();
#endif

	CWeaponKnife();

	float		GetRange( void );
	float		GetFireRate( void );
	virtual void PrimaryAttack(); //zombie
	void		AddViewKick( void );
	float		GetDamageForActivity( Activity hitActivity );
	void		SecondaryAttack( void )	{	return;	}

	void		Drop( const Vector &vecVelocity );

	CWeaponKnife(const CWeaponKnife&);

#ifndef CLIENT_DLL //-lun4r
	virtual void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	virtual int WeaponMeleeAttack1Condition(float flDot, float flDist);

private:
	// Animation event handlers
	void HandleAnimEventMeleeHit(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
#endif
		
};


#endif // HL2MP_WEAPON_Knife_H

