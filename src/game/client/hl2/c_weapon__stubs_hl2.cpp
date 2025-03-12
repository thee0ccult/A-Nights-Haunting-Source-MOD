//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "c_weapon__stubs.h"
#include "basehlcombatweapon_shared.h"
#include "c_basehlcombatweapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

STUB_WEAPON_CLASS( cycler_weapon, WeaponCycler, C_BaseCombatWeapon );

STUB_WEAPON_CLASS( weapon_binoculars, WeaponBinoculars, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_bugbait, WeaponBugBait, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_flaregun, Flaregun, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_annabelle, WeaponAnnabelle, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_gauss, WeaponGaussGun, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_cubemap, WeaponCubemap, C_BaseCombatWeapon );
STUB_WEAPON_CLASS( weapon_alyxgun, WeaponAlyxGun, C_HLSelectFireMachineGun );
STUB_WEAPON_CLASS( weapon_citizenpackage, WeaponCitizenPackage, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_citizensuitcase, WeaponCitizenSuitcase, C_WeaponCitizenPackage );

#ifndef HL2MP
STUB_WEAPON_CLASS( weapon_ar2, WeaponAR2, C_HLMachineGun );
STUB_WEAPON_CLASS(weapon_ak47, WeaponAK47, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_galil, WeaponGalil, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_sg552, WeaponSg552, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_m4a1, WeaponM4a1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_m249, WeaponM249, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_frag, WeaponFrag, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_rpg, WeaponRPG, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_pistol, WeaponPistol, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS(weapon_p228, WeaponP228, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_glock, WeaponGlock, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_fiveseven, WeaponFiveseven, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_shotgun, WeaponShotgun, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS(weapon_m3pump, WeaponM3pump, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_scout, WeaponScout, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_baikal56, WeaponBaikal56, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_patriot, WeaponPatriot, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_smg1, WeaponSMG1, C_HLSelectFireMachineGun );
STUB_WEAPON_CLASS(weapon_ump45, WeaponUmp45, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_mp5, WeaponMp5, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS( weapon_357, Weapon357, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_crossbow, WeaponCrossbow, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_slam, Weapon_SLAM, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_crowbar, WeaponCrowbar, C_BaseHLBludgeonWeapon );
STUB_WEAPON_CLASS(weapon_shovel, WeaponShovel, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_knife, WeaponKnife, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_pipewrench, WeaponPipewrench, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_pitchfork, WeaponPitchfork, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_axe, WeaponAxe, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_bat, WeaponBat, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_cleaver, WeaponCleaver, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_hockeystick, WeaponHockeystick, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_sledgehammer, WeaponSledgehammer, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_hammer, WeaponHammer, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_blowtorch, WeaponBlowtorch, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_pickaxe, WeaponPickaxe, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_pipe, WeaponPipe, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_cicle, WeaponCicle, C_BaseHLBludgeonWeapon);

#ifdef HL2_EPISODIC
STUB_WEAPON_CLASS( weapon_hopwire, WeaponHopwire, C_BaseHLCombatWeapon );
//STUB_WEAPON_CLASS( weapon_proto1, WeaponProto1, C_BaseHLCombatWeapon );
#endif
#ifdef HL2_LOSTCOAST
STUB_WEAPON_CLASS( weapon_oldmanharpoon, WeaponOldManHarpoon, C_WeaponCitizenPackage );
#endif
#endif


