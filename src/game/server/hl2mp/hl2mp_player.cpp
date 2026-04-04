//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for HL2.
//
//=============================================================================//

#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "hl2mp_player.h"
#include "globalstate.h"
#include "game.h"
#include "gamerules.h"
#include "hl2mp_player_shared.h"
#include "predicted_viewmodel.h"
#include "in_buttons.h"
#include "hl2mp_gamerules.h"
#include "KeyValues.h"
#include "team.h"
#include "weapon_hl2mpbase.h"
#include "grenade_satchel.h"
#include "eventqueue.h"
#include "gamestats.h"
#include "te_effect_dispatch.h"
#include "particle_parse.h"
#include "utlvector.h"
#include "baseentity.h"
#include "npc_manhack.h"

#include "engine/IEngineSound.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"

#include "ilagcompensationmanager.h"

int g_iLastCitizenModel = 0;
int g_iLastCombineModel = 0;

ConVar sv_weapon_slot_limit(
	"sv_weapon_slot_limit",
	"0",
	FCVAR_REPLICATED | FCVAR_NOTIFY,
	"Limit players to one weapon per slot (Bioshock-style)."
);

CBaseEntity	 *g_pLastCombineSpawn = NULL;
CBaseEntity	 *g_pLastRebelSpawn = NULL;
CBaseEntity* g_pLastZombieSpawn = NULL; //zombie

extern CBaseEntity				*g_pLastSpawn;

#define HL2MP_COMMAND_MAX_RATE 0.3

void DropPrimedFragGrenade( CHL2MP_Player *pPlayer, CBaseCombatWeapon *pGrenade );

LINK_ENTITY_TO_CLASS( player, CHL2MP_Player );

LINK_ENTITY_TO_CLASS( info_player_combine, CPointEntity );
LINK_ENTITY_TO_CLASS( info_player_rebel, CPointEntity );
LINK_ENTITY_TO_CLASS(info_player_zombie, CPointEntity); //zombie

IMPLEMENT_SERVERCLASS_ST(CHL2MP_Player, DT_HL2MP_Player)
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 0), 11, SPROP_CHANGES_OFTEN ),
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 11, SPROP_CHANGES_OFTEN ),
	SendPropEHandle( SENDINFO( m_hRagdoll ) ),
	SendPropInt( SENDINFO( m_iSpawnInterpCounter), 4 ),
	SendPropInt( SENDINFO( m_iPlayerSoundType), 3 ),
	
	SendPropBool(SENDINFO(m_bHealthVisionActive)),// health vision zombie

	SendPropExclude( "DT_BaseAnimating", "m_flPoseParameter" ),
	SendPropExclude( "DT_BaseFlex", "m_viewtarget" ),

//	SendPropExclude( "DT_ServerAnimationData" , "m_flCycle" ),	
//	SendPropExclude( "DT_AnimTimeMustBeFirst" , "m_flAnimTime" ),
	
END_SEND_TABLE()

BEGIN_DATADESC( CHL2MP_Player )
END_DATADESC()

const char *g_ppszRandomCitizenModels[] = 
{
	"models2/humans/drn0/drn0.mdl",
	"models2/humans/gordon/gordon.mdl",
	"models2/humans/group03/male_01.mdl",
	"models2/humans/group03/male_02.mdl",
	"models2/humans/group03/female_01.mdl",
	"models2/humans/group03/male_03.mdl",
	"models2/humans/group03/female_02.mdl",
	"models2/humans/group03/male_04.mdl",
	"models2/humans/group03/female_03.mdl",
	"models2/humans/group03/male_05.mdl",
	"models2/humans/group03/female_04.mdl",
	"models2/humans/group03/male_06.mdl",
	"models2/humans/group03/female_06.mdl",
	"models2/humans/group03/male_07.mdl",
	"models2/humans/group03/female_07.mdl",
	"models2/humans/group03/male_08.mdl",
	"models2/humans/group03/male_09.mdl",
};

const char *g_ppszRandomCombineModels[] =
{
	"models2/combine_soldier.mdl",
	"models2/combine_soldier_prisonguard.mdl",
	"models2/combine_super_soldier.mdl",
	"models2/police.mdl",
	"models2/gman/gman.mdl",

};


#define MAX_COMBINE_MODELS 5
#define MODEL_CHANGE_INTERVAL 5.0f
#define TEAM_CHANGE_INTERVAL 5.0f

#define HL2MPPLAYER_PHYSDAMAGE_SCALE 4.0f

#pragma warning( disable : 4355 )

CHL2MP_Player::CHL2MP_Player() : m_PlayerAnimState( this )
{
	m_angEyeAngles.Init();

	m_iLastWeaponFireUsercmd = 0;

	m_flNextModelChangeTime = 0.0f;
	m_flNextTeamChangeTime = 0.0f;

	m_flNextLeapTime = 0.0f; //zombie leap
	m_bZombieLeapActive = false;// zombie leap
	m_flNextCrowSound = 0.0f;
	m_bFlyMode = false;
	m_flNextZombieManhackTime = 0.0f; //crow throw

	m_hFlyAnchor = NULL;

	m_bHealthVisionActive = false; //health vision

	m_iSpawnInterpCounter = 0;

    m_bEnterObserver = false;
	m_bReady = false;

	BaseClass::ChangeTeam( 0 );
	
//	UseClientSideAnimation();
}

CHL2MP_Player::~CHL2MP_Player( void )
{

}

void CHL2MP_Player::UpdateOnRemove(void)
{

	m_flNextZombieManhackTime = 0.0f;

	variant_t emptyVariant;

	StopFlyParticle();

	for (int i = 0; i < m_hFlyParticles.Count(); ++i)
	{
		CBaseEntity* pEnt = m_hFlyParticles[i].Get();
		if (!pEnt)
			continue;

		pEnt->AcceptInput("Kill", this, this, emptyVariant, 0);
		UTIL_Remove(pEnt);
	}

	m_hFlyParticles.RemoveAll();

	if (m_hFlyAnchor)
	{
		CBaseEntity* pAnchor = m_hFlyAnchor.Get();
		if (pAnchor)
		{
			pAnchor->AcceptInput("KillHierarchy", this, this, emptyVariant, 0);
			pAnchor->AcceptInput("Kill", this, this, emptyVariant, 0);
			UTIL_Remove(pAnchor);
		}

		m_hFlyAnchor = NULL;
	}

	if (m_hRagdoll)
	{
		UTIL_RemoveImmediate(m_hRagdoll);
		m_hRagdoll = NULL;
	}

	BaseClass::UpdateOnRemove();
}

void CHL2MP_Player::Precache(void)
{
	BaseClass::Precache();
	// disabled flies particles
	// PrecacheParticleSystem("flies_large");
	UTIL_PrecacheOther("npc_manhack"); //crow throw
	PrecacheScriptSound("NPC_MetroPolice.DeployManhack");

	PrecacheModel("sprites/glow01.vmt");

	// Zombie model
	PrecacheModel("models2/humans/group03/male_07.mdl");

	// Crow leap model
	PrecacheModel("models/crow.mdl");

	// Zombie sounds
	PrecacheScriptSound("NPC_FastZombie.LeapAttack");

	PrecacheScriptSound("NPC_Crow.Alert");

	// Precache Citizen models
	int nHeads = ARRAYSIZE(g_ppszRandomCitizenModels);
	int i;

	for (i = 0; i < nHeads; ++i)
		PrecacheModel(g_ppszRandomCitizenModels[i]);

	// Precache Combine Models
	nHeads = ARRAYSIZE(g_ppszRandomCombineModels);

	for (i = 0; i < nHeads; ++i)
		PrecacheModel(g_ppszRandomCombineModels[i]);

	PrecacheFootStepSounds();

	PrecacheScriptSound("NPC_MetroPolice.Die");
	PrecacheScriptSound("NPC_CombineS.Die");
	PrecacheScriptSound("NPC_Citizen.die");
}

void CHL2MP_Player::GiveAllItems( void )
{
	EquipSuit();

	CBasePlayer::GiveAmmo( 255,	"Pistol");
	CBasePlayer::GiveAmmo( 255,	"AR2" );
	CBasePlayer::GiveAmmo( 5,	"AR2AltFire" );
	CBasePlayer::GiveAmmo( 255,	"SMG1");
	CBasePlayer::GiveAmmo( 1,	"smg1_grenade");
	CBasePlayer::GiveAmmo( 255,	"Buckshot");
	CBasePlayer::GiveAmmo( 32,	"357" );
	CBasePlayer::GiveAmmo( 3,	"rpg_round");

	CBasePlayer::GiveAmmo( 1,	"grenade" );
	CBasePlayer::GiveAmmo( 2,	"slam" );

	GiveNamedItem( "weapon_crowbar" );
	GiveNamedItem( "weapon_stunstick" );
	GiveNamedItem("weapon_shovel");
	GiveNamedItem("weapon_knife");
	GiveNamedItem("weapon_pipewrench");
	GiveNamedItem("weapon_pitchfork");
	GiveNamedItem("weapon_axe");
	GiveNamedItem("weapon_bat");
	GiveNamedItem("weapon_cleaver");
	GiveNamedItem("weapon_cigarette");
	GiveNamedItem("weapon_hockeystick");
	GiveNamedItem("weapon_sledgehammer");
	GiveNamedItem("weapon_hammer");
	GiveNamedItem("weapon_blowtorch");
	GiveNamedItem("weapon_pickaxe");
	GiveNamedItem("weapon_pipe");
	GiveNamedItem("weapon_cicle");
	GiveNamedItem( "weapon_pistol" );
	GiveNamedItem( "weapon_357" );
	GiveNamedItem("weapon_p228");
	GiveNamedItem("weapon_pistolsilenced");
	GiveNamedItem("weapon_glock");
	GiveNamedItem("weapon_fiveseven");
	GiveNamedItem("weapon_dualies");

	GiveNamedItem( "weapon_smg1" );
	GiveNamedItem("weapon_ump45");
	GiveNamedItem("weapon_mp5");
	GiveNamedItem( "weapon_ar2" );
	GiveNamedItem("weapon_ak47");
	GiveNamedItem("weapon_galil");
	GiveNamedItem("weapon_sg552");
	GiveNamedItem("weapon_m4a1");
	GiveNamedItem("weapon_m249");
	GiveNamedItem("weapon_scout");
	GiveNamedItem("weapon_baikal56");
	GiveNamedItem("weapon_patriot");

	GiveNamedItem("weapon_m3pump");
	GiveNamedItem( "weapon_shotgun" );
	GiveNamedItem( "weapon_frag" );
	
	GiveNamedItem( "weapon_crossbow" );
	
	GiveNamedItem( "weapon_rpg" );

	GiveNamedItem( "weapon_slam" );

	GiveNamedItem( "weapon_physcannon" );
	
}

void CHL2MP_Player::GiveDefaultItems()
{
	// ZOMBIE LOADOUT
	if (GetTeamNumber() == TEAM_ZOMBIE)
	{
		// DO NOT remove items here
		// Engine already cleaned inventory on spawn

		CBaseCombatWeapon* pKnife = (CBaseCombatWeapon*)GiveNamedItem("weapon_knife");
		if (pKnife)
		{
			Weapon_Equip(pKnife);
			Weapon_Switch(pKnife);
		}

		CBaseCombatWeapon* pPhys = (CBaseCombatWeapon*)GiveNamedItem("weapon_physcannon");
		if (pPhys)
		{
			Weapon_Equip(pPhys);
		}

		return;
	}

	// NORMAL PLAYERS
	CBaseCombatWeapon* pPhys = (CBaseCombatWeapon*)GiveNamedItem("weapon_physcannon");

	if (pPhys)
	{
		Weapon_Equip(pPhys);
		Weapon_Switch(pPhys);
	}
}



void CHL2MP_Player::PickDefaultSpawnTeam( void )
{
	if ( GetTeamNumber() == 0 )
	{
		if ( HL2MPRules()->IsTeamplay() == false )
		{
			if ( GetModelPtr() == NULL )
			{
				const char *szModelName = NULL;
				szModelName = engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_playermodel" );

				if ( ValidatePlayerModel( szModelName ) == false )
				{
					char szReturnString[512];

					Q_snprintf( szReturnString, sizeof (szReturnString ), "cl_playermodel models2/combine_soldier.mdl\n" );
					engine->ClientCommand ( edict(), szReturnString );
				}

				ChangeTeam( TEAM_UNASSIGNED );
			}
		}
		else
		{
			CTeam *pCombine = g_Teams[TEAM_COMBINE];
			CTeam *pRebels = g_Teams[TEAM_REBELS];

			if ( pCombine == NULL || pRebels == NULL )
			{
				ChangeTeam( random->RandomInt( TEAM_COMBINE, TEAM_REBELS ) );
			}
			else
			{
				if ( pCombine->GetNumPlayers() > pRebels->GetNumPlayers() )
				{
					ChangeTeam( TEAM_REBELS );
				}
				else if ( pCombine->GetNumPlayers() < pRebels->GetNumPlayers() )
				{
					ChangeTeam( TEAM_COMBINE );
				}
				else
				{
					ChangeTeam( random->RandomInt( TEAM_COMBINE, TEAM_REBELS ) );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets HL2 specific defaults.
//-----------------------------------------------------------------------------
void CHL2MP_Player::Spawn(void)
{
	m_flNextModelChangeTime = 0.0f;
	m_flNextTeamChangeTime = 0.0f;
	m_flNextZombieManhackTime = gpGlobals->curtime + 1.0f; //crow throw


	PickDefaultSpawnTeam();

	BaseClass::Spawn();

	if (!IsObserver())
	{
		pl.deadflag = false;
		RemoveSolidFlags(FSOLID_NOT_SOLID);
		RemoveEffects(EF_NODRAW);

		GiveDefaultItems();
	}

	SetNumAnimOverlays(3);
	ResetAnimation();

	m_nRenderFX = kRenderNormal;
	m_Local.m_iHideHUD = 0;

	AddFlag(FL_ONGROUND);

	m_impactEnergyScale = HL2MPPLAYER_PHYSDAMAGE_SCALE;

	if (HL2MPRules()->IsIntermission())
		AddFlag(FL_FROZEN);
	else
		RemoveFlag(FL_FROZEN);

	m_iSpawnInterpCounter = (m_iSpawnInterpCounter + 1) % 8;

	m_Local.m_bDucked = false;

	SetPlayerUnderwater(false);
	m_bHealthVisionActive = false; //health vision
	m_bReady = false;
	m_bFlyMode = false;
	StopFlyParticle();
	StopParticleEffects(this);
	RemoveEffects(EF_NODRAW);
	SetRenderMode(kRenderNormal);
	SetRenderColor(255, 255, 255, 255);
	SetCloakStatus(0);
	SetCloakFactor(0.0f);
	// ZOMBIE SETUP
	if (GetTeamNumber() == TEAM_ZOMBIE)
	{
		m_bZombieLeapActive = false;
		SetModel("models2/humans/group03/male_07.mdl");
		SetupPlayerSoundsByModel("models2/humans/group03/male_07.mdl");

		SetHealth(200);
		SetMaxHealth(200);
	}
}


void CHL2MP_Player::PickupObject( CBaseEntity *pObject, bool bLimitMassAndSize )
{
	
}

bool CHL2MP_Player::ValidatePlayerModel( const char *pModel )
{
	int iModels = ARRAYSIZE( g_ppszRandomCitizenModels );
	int i;	

	for ( i = 0; i < iModels; ++i )
	{
		if ( !Q_stricmp( g_ppszRandomCitizenModels[i], pModel ) )
		{
			return true;
		}
	}

	iModels = ARRAYSIZE( g_ppszRandomCombineModels );

	for ( i = 0; i < iModels; ++i )
	{
	   	if ( !Q_stricmp( g_ppszRandomCombineModels[i], pModel ) )
		{
			return true;
		}
	}

	return false;
}

void CHL2MP_Player::SetPlayerTeamModel(void)
{
	const char* szModelName = NULL;
	szModelName = engine->GetClientConVarValue(engine->IndexOfEdict(edict()), "cl_playermodel");

	int modelIndex = modelinfo->GetModelIndex(szModelName);

	if (GetTeamNumber() == TEAM_ZOMBIE)
	{
		szModelName = "models2/humans/group03/male_07.mdl";
		m_iModelType = TEAM_ZOMBIE;
	}
	else
	{
		if (modelIndex == -1 || ValidatePlayerModel(szModelName) == false)
		{
			szModelName = "models2/Combine_Soldier.mdl";
			m_iModelType = TEAM_COMBINE;

			char szReturnString[512];

			Q_snprintf(szReturnString, sizeof(szReturnString), "cl_playermodel %s\n", szModelName);
			engine->ClientCommand(edict(), szReturnString);
		}

		if (GetTeamNumber() == TEAM_COMBINE)
		{
			if (Q_stristr(szModelName, "models2/human"))
			{
				int nHeads = ARRAYSIZE(g_ppszRandomCombineModels);

				g_iLastCombineModel = (g_iLastCombineModel + 1) % nHeads;
				szModelName = g_ppszRandomCombineModels[g_iLastCombineModel];
			}

			m_iModelType = TEAM_COMBINE;
		}
		else if (GetTeamNumber() == TEAM_REBELS)
		{
			if (!Q_stristr(szModelName, "models2/human"))
			{
				int nHeads = ARRAYSIZE(g_ppszRandomCitizenModels);

				g_iLastCitizenModel = (g_iLastCitizenModel + 1) % nHeads;
				szModelName = g_ppszRandomCitizenModels[g_iLastCitizenModel];
			}

			m_iModelType = TEAM_REBELS;
		}
	}

	SetModel(szModelName);
	SetupPlayerSoundsByModel(szModelName);

	m_flNextModelChangeTime = gpGlobals->curtime + MODEL_CHANGE_INTERVAL;
}

void CHL2MP_Player::SetPlayerModel(void)
{
	const char* szModelName = NULL;
	const char* pszCurrentModelName = modelinfo->GetModelName(GetModel());

	szModelName = engine->GetClientConVarValue(engine->IndexOfEdict(edict()), "cl_playermodel");

	if (GetTeamNumber() == TEAM_ZOMBIE)
	{
		szModelName = "models2/humans/group03/male_07.mdl";
		m_iModelType = TEAM_ZOMBIE;
	}
	else
	{
		if (ValidatePlayerModel(szModelName) == false)
		{
			char szReturnString[512];

			if (ValidatePlayerModel(pszCurrentModelName) == false)
			{
				pszCurrentModelName = "models2/Combine_Soldier.mdl";
			}

			Q_snprintf(szReturnString, sizeof(szReturnString), "cl_playermodel %s\n", pszCurrentModelName);
			engine->ClientCommand(edict(), szReturnString);

			szModelName = pszCurrentModelName;
		}

		if (GetTeamNumber() == TEAM_COMBINE)
		{
			int nHeads = ARRAYSIZE(g_ppszRandomCombineModels);

			g_iLastCombineModel = (g_iLastCombineModel + 1) % nHeads;
			szModelName = g_ppszRandomCombineModels[g_iLastCombineModel];

			m_iModelType = TEAM_COMBINE;
		}
		else if (GetTeamNumber() == TEAM_REBELS)
		{
			int nHeads = ARRAYSIZE(g_ppszRandomCitizenModels);

			g_iLastCitizenModel = (g_iLastCitizenModel + 1) % nHeads;
			szModelName = g_ppszRandomCitizenModels[g_iLastCitizenModel];

			m_iModelType = TEAM_REBELS;
		}
		else
		{
			if (Q_strlen(szModelName) == 0)
			{
				szModelName = g_ppszRandomCitizenModels[0];
			}

			if (Q_stristr(szModelName, "models2/human"))
			{
				m_iModelType = TEAM_REBELS;
			}
			else
			{
				m_iModelType = TEAM_COMBINE;
			}
		}

		int modelIndex = modelinfo->GetModelIndex(szModelName);

		if (modelIndex == -1)
		{
			szModelName = "models2/Combine_Soldier.mdl";
			m_iModelType = TEAM_COMBINE;

			char szReturnString[512];

			Q_snprintf(szReturnString, sizeof(szReturnString), "cl_playermodel %s\n", szModelName);
			engine->ClientCommand(edict(), szReturnString);
		}
	}

	SetModel(szModelName);
	SetupPlayerSoundsByModel(szModelName);

	m_flNextModelChangeTime = gpGlobals->curtime + MODEL_CHANGE_INTERVAL;
}

void CHL2MP_Player::SetupPlayerSoundsByModel( const char *pModelName )
{
	if ( Q_stristr( pModelName, "models2/human") )
	{
		m_iPlayerSoundType = (int)PLAYER_SOUNDS_CITIZEN;
	}
	else if ( Q_stristr(pModelName, "police" ) )
	{
		m_iPlayerSoundType = (int)PLAYER_SOUNDS_METROPOLICE;
	}
	else if ( Q_stristr(pModelName, "combine" ) )
	{
		m_iPlayerSoundType = (int)PLAYER_SOUNDS_COMBINESOLDIER;
	}
}

void CHL2MP_Player::ResetAnimation( void )
{
	if ( IsAlive() )
	{
		SetSequence ( -1 );
		SetActivity( ACT_INVALID );

		if (!GetAbsVelocity().x && !GetAbsVelocity().y)
			SetAnimation( PLAYER_IDLE );
		else if ((GetAbsVelocity().x || GetAbsVelocity().y) && ( GetFlags() & FL_ONGROUND ))
			SetAnimation( PLAYER_WALK );
		else if (GetWaterLevel() > 1)
			SetAnimation( PLAYER_WALK );
	}
}


bool CHL2MP_Player::Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex )
{
	bool bRet = BaseClass::Weapon_Switch( pWeapon, viewmodelindex );

	if ( bRet == true )
	{
		ResetAnimation();
	}

	return bRet;
}

void CHL2MP_Player::UpdateZombieCloak()
{
	// only zombies can cloak
	if (GetTeamNumber() != TEAM_ZOMBIE)
	{
		SetCloakStatus(0);
		SetCloakFactor(0.0f);
		return;
	}

	// m_bFlyMode = cloak requested
	if (m_bFlyMode)
	{
		// start / continue cloaking
		if (GetCloakStatus() == 0 || GetCloakStatus() == 1)
		{
			SetCloakStatus(3); // cloaking
		}
	}
	else
	{
		// start / continue uncloaking
		if (GetCloakStatus() == 2 || GetCloakStatus() == 3)
		{
			SetCloakStatus(1); // uncloaking
		}
	}

	// advance cloak factor
	if (!engine->IsPaused())
	{
		if (GetCloakStatus() == 3) // cloaking
		{
			float flNew = GetCloakFactor() + 0.02f;

			// players should not go full 1.0, keep some silhouette
			if (flNew >= 0.975f)
			{
				flNew = 0.975f;
				SetCloakStatus(2); // fully cloaked
			}

			SetCloakFactor(flNew);
		}
		else if (GetCloakStatus() == 1) // uncloaking
		{
			float flNew = GetCloakFactor() - 0.02f;

			if (flNew <= 0.0f)
			{
				flNew = 0.0f;
				SetCloakStatus(0); // fully visible
			}

			SetCloakFactor(flNew);
		}
		else if (GetCloakStatus() == 0)
		{
			SetCloakFactor(0.0f);
		}
		else if (GetCloakStatus() == 2)
		{
			// fully cloaked - do nothing
		}
	}

	// =======================================
	// HARD SHADOW DISABLE (FIX)
	// =======================================
	if (GetTeamNumber() != TEAM_ZOMBIE)
	{
		RemoveEffects(EF_NOSHADOW);
		RemoveEffects(EF_NORECEIVESHADOW);
		return;
	}

	if (GetCloakFactor() > 0.1f)
	{
		AddEffects(EF_NOSHADOW);
		AddEffects(EF_NORECEIVESHADOW);
	}
	else
	{
		RemoveEffects(EF_NOSHADOW);
		RemoveEffects(EF_NORECEIVESHADOW);
		RemoveEffects(EF_NODRAW); // safety restore
	}
}

void CHL2MP_Player::StartFlyParticle()
{

	if (GetTeamNumber() != TEAM_ZOMBIE)
		return;

	static const Vector offsets[] =
	{
		Vector(0, 0, 40),
		Vector(20, 0, 45),
		Vector(-20, 0, 45),
		Vector(0, 20, 35),
		Vector(0, -20, 35)
	};

	variant_t emptyVariant;

	// Create anchor once
	if (!m_hFlyAnchor)
	{
		CBaseEntity* pAnchor = CreateEntityByName("info_target");
		if (!pAnchor)
			return;

		DispatchSpawn(pAnchor);
		pAnchor->AddEffects(EF_NODRAW);
		pAnchor->SetParent(this);
		pAnchor->SetLocalOrigin(vec3_origin);
		pAnchor->SetLocalAngles(vec3_angle);

		m_hFlyAnchor = pAnchor;
	}
	else
	{
		CBaseEntity* pAnchor = m_hFlyAnchor.Get();
		if (pAnchor)
		{
			pAnchor->SetParent(this);
			pAnchor->SetLocalOrigin(vec3_origin);
			pAnchor->SetLocalAngles(vec3_angle);
		}
	}

	if (m_hFlyParticles.Count() == 0)
	{
		CBaseEntity* pAnchor = m_hFlyAnchor.Get();
		if (!pAnchor)
			return;

		for (int i = 0; i < ARRAYSIZE(offsets); ++i)
		{
			CBaseEntity* pEnt = CreateEntityByName("info_particle_system");
			if (!pEnt)
				continue;

			pEnt->KeyValue("effect_name", "flies_large");
			pEnt->KeyValue("start_active", "0");

			DispatchSpawn(pEnt);

			pEnt->SetParent(pAnchor);
			pEnt->SetLocalOrigin(offsets[i]);
			pEnt->SetLocalAngles(vec3_angle);
			pEnt->Activate();
			pEnt->RemoveEffects(EF_NODRAW);

			m_hFlyParticles.AddToTail(pEnt);
		}
	}

	for (int i = 0; i < m_hFlyParticles.Count(); ++i)
	{
		CBaseEntity* pEnt = m_hFlyParticles[i].Get();
		if (!pEnt)
			continue;

		pEnt->RemoveEffects(EF_NODRAW);
		pEnt->AcceptInput("Start", this, this, emptyVariant, 0);
	}
}

void CHL2MP_Player::StopFlyParticle()
{
	variant_t emptyVariant;

	for (int i = 0; i < m_hFlyParticles.Count(); ++i)
	{
		CBaseEntity* pEnt = m_hFlyParticles[i].Get();
		if (!pEnt)
			continue;

		pEnt->AcceptInput("Stop", this, this, emptyVariant, 0);
		StopParticleEffects(pEnt);
		pEnt->AddEffects(EF_NODRAW);   // hide entity too
	}

	// keep anchor alive, but stop it following if you want
	if (m_hFlyAnchor)
	{
		CBaseEntity* pAnchor = m_hFlyAnchor.Get();
		if (pAnchor)
		{
			pAnchor->SetParent(this);
			pAnchor->SetLocalOrigin(vec3_origin);
			pAnchor->SetLocalAngles(vec3_angle);
		}
	}

	StopParticleEffects(this);
}

void CHL2MP_Player::PreThink(void)
{
	QAngle vOldAngles = GetLocalAngles();

	QAngle vTempAngles = EyeAngles();
	if (vTempAngles[PITCH] > 180.0f)
	{
		vTempAngles[PITCH] -= 360.0f;
	}
	SetLocalAngles(vTempAngles);

	BaseClass::PreThink();
	State_PreThink();
	UpdateZombieCloak();

	// =========================================================
	// ZOMBIE CROW LEAP STATE
	// =========================================================
	if (GetTeamNumber() == TEAM_ZOMBIE)
	{
		// End leap when grounded and restore zombie model
		if (m_bZombieLeapActive && (GetFlags() & FL_ONGROUND))
		{
			m_bZombieLeapActive = false;
			StopFlyParticle();
			m_bFlyMode = false;

			if (Q_stricmp(STRING(GetModelName()), "models/crow.mdl") == 0)
			{
				SetModel("models2/humans/group03/male_07.mdl");
				SetupPlayerSoundsByModel("models2/humans/group03/male_07.mdl");
				ResetAnimation();
			}

			// =========================================================
			// FULL VIEWMODEL RESTORE (CRITICAL FIX)
			// =========================================================
			CBaseViewModel* vm = GetViewModel(0);
			CBaseCombatWeapon* pWep = GetActiveWeapon();

			if (vm)
			{
				vm->RemoveEffects(EF_NODRAW);
			}

			if (pWep)
			{
				pWep->RemoveEffects(EF_NODRAW);

				// FORCE VIEWMODEL RE-DEPLOY
				pWep->Deploy();
			}
		}

		// Start leap with ATTACK2
		if ((m_nButtons & IN_ATTACK2) && gpGlobals->curtime >= m_flNextLeapTime && !m_bZombieLeapActive)
		{
			if (m_bFlyMode)
			{
				m_bFlyMode = false;
				StopFlyParticle();

				SetRenderMode(kRenderNormal);
				SetRenderColor(255, 255, 255, 255);

				CBaseViewModel* vm = GetViewModel(0);
				if (vm) vm->RemoveEffects(EF_NODRAW);

				CBaseCombatWeapon* pWepRestore = GetActiveWeapon();
				if (pWepRestore) pWepRestore->RemoveEffects(EF_NODRAW);
			}
			Vector forward;
			AngleVectors(EyeAngles(), &forward);

			Vector velocity = forward * 380.0f + Vector(0, 0, 220.0f);
			SetAbsVelocity(velocity);

			m_bZombieLeapActive = true;
			// Hide weapon + viewmodel
			CBaseViewModel* vm = GetViewModel(0);
			if (vm)
			{
				vm->AddEffects(EF_NODRAW);
			}

			CBaseCombatWeapon* pWep = GetActiveWeapon();
			if (pWep)
			{
				pWep->AddEffects(EF_NODRAW);
			}
			m_flNextLeapTime = gpGlobals->curtime + 3.0f;

			// swap to crow model
			if (Q_stricmp(STRING(GetModelName()), "models/crow.mdl") != 0)
			{
				SetModel("models/crow.mdl");
				ResetSequence(LookupSequence("Fly01"));
			}

			int iFlySeq = LookupSequence("Fly01");
			if (iFlySeq < 0)
				iFlySeq = LookupSequence("Idle01");

			if (iFlySeq >= 0)
			{
				ResetSequence(iFlySeq);
				SetCycle(0.0f);
				m_flPlaybackRate = 1.0f;
			}

			EmitSound("NPC_FastZombie.LeapAttack");
		}

		// While in crow leap, tapping jump gives lift
		if (m_bZombieLeapActive && (m_afButtonPressed & IN_JUMP))
		{
			Vector vel = GetAbsVelocity();

			// small flap upward boost
			vel.z += 120.0f;

			// optional cap so it doesn't become infinite rocket flight
			if (vel.z > 260.0f)
			{
				vel.z = 260.0f;
			}

			SetAbsVelocity(vel);
		}

		// Optional gentle glide feel while crow is active
		if (m_bZombieLeapActive)
		{
			Vector vel = GetAbsVelocity();

			// soften falling speed a bit so repeated jumps feel like flapping
			if (vel.z < -200.0f)
			{
				vel.z = -200.0f;
				SetAbsVelocity(vel);
			}
			// Crow screech loop
			if (gpGlobals->curtime >= m_flNextCrowSound)
			{
				EmitSound("NPC_Crow.Alert");
				m_flNextCrowSound = gpGlobals->curtime + 1.5f; // interval
			}
		}
	}

	// Toggle fly particle mode
	// =========================================================
	// NON-ZOMBIE HARD BLOCK
	// =========================================================
	if (GetTeamNumber() != TEAM_ZOMBIE)
	{
		if (m_bFlyMode || m_bZombieLeapActive)
		{
			m_bFlyMode = false;
			m_bZombieLeapActive = false;
			StopFlyParticle();

			CBaseViewModel* vm = GetViewModel(0);
			if (vm) vm->RemoveEffects(EF_NODRAW);

			CBaseCombatWeapon* wep = GetActiveWeapon();
			if (wep) wep->RemoveEffects(EF_NODRAW);

			if (Q_stricmp(STRING(GetModelName()), "models/crow.mdl") == 0)
			{
				SetPlayerTeamModel();
				ResetAnimation();
			}

			SetRenderMode(kRenderNormal);
			SetRenderColor(255, 255, 255, 255);
		}
	}
	else
	{
		// Toggle fly particle mode (ZOMBIES ONLY) disabled for now
		if ((m_afButtonPressed & IN_FLY) && !m_bZombieLeapActive)
		{
			m_bFlyMode = !m_bFlyMode;

			if (m_bFlyMode)
			{
				// Cloak ON (KEEP THIS)
				CBaseViewModel* vm = GetViewModel(0);
				if (vm) vm->AddEffects(EF_NODRAW);

				CBaseCombatWeapon* wep = GetActiveWeapon();
				if (wep) wep->AddEffects(EF_NODRAW);

				// REMOVE PARTICLES
				// StopFlyParticle();
				// StartFlyParticle();
			}
			else
			{
				m_bFlyMode = false;

				// REMOVE PARTICLES
				// StopFlyParticle();

				// Restore weapon/viewmodel
				CBaseViewModel* vm = GetViewModel(0);
				if (vm) vm->RemoveEffects(EF_NODRAW);

				CBaseCombatWeapon* wep = GetActiveWeapon();
				if (wep) wep->RemoveEffects(EF_NODRAW);
			}
		}
	}

	// ============================
	// HEALTH VISION (ZOMBIE ONLY)
	// ============================
	if (GetTeamNumber() == TEAM_ZOMBIE)
	{
		if (m_afButtonPressed & IN_HEALTHVISION)
		{
			m_bHealthVisionActive = !m_bHealthVisionActive;

			EmitSound(m_bHealthVisionActive ? "HL2Player.FlashlightOn" : "HL2Player.FlashlightOff");
		}
	}
	else
	{
		m_bHealthVisionActive = false;
	}

	// hard safety: if fly mode is off, no fly particles are allowed to exist
	if (GetTeamNumber() != TEAM_ZOMBIE || !m_bFlyMode)
	{
		StopFlyParticle();
	}

	m_vecTotalBulletForce = vec3_origin;
	SetLocalAngles(vOldAngles);
}

void CHL2MP_Player::PostThink(void)
{
	BaseClass::PostThink();

	if (GetFlags() & FL_DUCKING)
	{
		SetCollisionBounds(VEC_CROUCH_TRACE_MIN, VEC_CROUCH_TRACE_MAX);
	}

	m_PlayerAnimState.Update();
	m_angEyeAngles = EyeAngles();

	QAngle angles = GetLocalAngles();
	angles[PITCH] = 0;
	SetLocalAngles(angles);
}

void CHL2MP_Player::PlayerDeathThink()
{
	if( !IsObserver() )
	{
		BaseClass::PlayerDeathThink();
	}
}

void CHL2MP_Player::FireBullets ( const FireBulletsInfo_t &info )
{
	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( this, this->GetCurrentCommand() );

	FireBulletsInfo_t modinfo = info;

	CWeaponHL2MPBase *pWeapon = dynamic_cast<CWeaponHL2MPBase *>( GetActiveWeapon() );

	if ( pWeapon )
	{
		modinfo.m_iPlayerDamage = modinfo.m_flDamage = pWeapon->GetHL2MPWpnData().m_iPlayerDamage;
	}

	NoteWeaponFired();

	BaseClass::FireBullets( modinfo );

	// Move other players back to history positions based on local player's lag
	lagcompensation->FinishLagCompensation( this );
}

void CHL2MP_Player::NoteWeaponFired( void )
{
	Assert( m_pCurrentCommand );
	if( m_pCurrentCommand )
	{
		m_iLastWeaponFireUsercmd = m_pCurrentCommand->command_number;
	}
}

extern ConVar sv_maxunlag;

bool CHL2MP_Player::WantsLagCompensationOnEntity(const CBaseEntity *pEntity, const CUserCmd *pCmd, const CBitVec<MAX_EDICTS> *pEntityTransmitBits) const
{
	// No need to lag compensate at all if we're not attacking in this command and
	// we haven't attacked recently.
	if ( !( pCmd->buttons & IN_ATTACK ) && (pCmd->command_number - m_iLastWeaponFireUsercmd > 5) )
		return false;

	// If this entity hasn't been transmitted to us and acked, then don't bother lag compensating it.
	if (pEntityTransmitBits && !pEntityTransmitBits->Get(pEntity->entindex())) 
		return false;

	const Vector &vMyOrigin = GetAbsOrigin();
	const Vector &vHisOrigin = pEntity->GetAbsOrigin();

	// get max distance player could have moved within max lag compensation time, 
	// multiply by 1.5 to to avoid "dead zones"  (sqrt(2) would be the exact value)
	float maxspeed;
	CBasePlayer *pPlayer = ToBasePlayer((CBaseEntity*)pEntity);
	if (pPlayer)
		maxspeed = pPlayer->MaxSpeed();
	else
		maxspeed = 600;
	float maxDistance = 1.5 * maxspeed * sv_maxunlag.GetFloat();


	// If the player is within this distance, lag compensate them in case they're running past us.
	if ( vHisOrigin.DistTo( vMyOrigin ) < maxDistance )
		return true;

	// If their origin is not within a 45 degree cone in front of us, no need to lag compensate.
	Vector vForward;
	AngleVectors( pCmd->viewangles, &vForward );
	
	Vector vDiff = vHisOrigin - vMyOrigin;
	VectorNormalize( vDiff );

	float flCosAngle = 0.707107f;	// 45 degree angle
	if ( vForward.Dot( vDiff ) < flCosAngle )
		return false;

	return true;
}

Activity CHL2MP_Player::TranslateTeamActivity( Activity ActToTranslate )
{
	if ( m_iModelType == TEAM_COMBINE )
		 return ActToTranslate;
	
	if ( ActToTranslate == ACT_RUN )
		 return ACT_RUN_AIM_AGITATED;

	if ( ActToTranslate == ACT_IDLE )
		 return ACT_IDLE_AIM_AGITATED;

	if ( ActToTranslate == ACT_WALK )
		 return ACT_WALK_AIM_AGITATED;

	return ActToTranslate;
}

extern ConVar hl2_normspeed;

//-----------------------------------------------------------------------------
// Purpose: Set the activity based on an event or current state.
// Restores original HL2MP animation handling for Combine/Rebel/DM players.
// TEAM_ZOMBIE only keeps a very small crow-airborne override.
//-----------------------------------------------------------------------------
void CHL2MP_Player::SetAnimation(PLAYER_ANIM playerAnim)
{
	// =========================================================
	// ZOMBIE CROW OVERRIDE ONLY
	// Keep the special crow fly animation while the zombie is
	// actively in crow leap and actually using the crow model.
	// Everything else falls through to the original HL2MP logic.
	// =========================================================
	if (GetTeamNumber() == TEAM_ZOMBIE &&
		m_bZombieLeapActive &&
		Q_stricmp(STRING(GetModelName()), "models/crow.mdl") == 0)
	{
		if (!(GetFlags() & FL_ONGROUND))
		{
			int iAir = LookupSequence("Fly01");
			if (iAir < 0)
				iAir = LookupSequence("Idle01");

			if (iAir >= 0)
			{
				if (GetSequence() != iAir)
				{
					ResetSequence(iAir);
					SetCycle(0);
				}

				m_flPlaybackRate = 1.0f;
				return;
			}
		}
	}

	// =========================================================
	// ORIGINAL HL2MP PLAYER ANIMATION HANDLING
	// =========================================================
	int animDesired;
	float speed;

	speed = GetAbsVelocity().Length2D();

	if (GetFlags() & (FL_FROZEN | FL_ATCONTROLS))
	{
		speed = 0;
		playerAnim = PLAYER_IDLE;
	}

	Activity idealActivity = ACT_HL2MP_RUN;

	if (playerAnim == PLAYER_JUMP)
	{
		idealActivity = ACT_HL2MP_JUMP;
	}
	else if (playerAnim == PLAYER_DIE)
	{
		if (m_lifeState == LIFE_ALIVE)
		{
			return;
		}
	}
	else if (playerAnim == PLAYER_ATTACK1)
	{
		if (GetActivity() == ACT_HOVER ||
			GetActivity() == ACT_SWIM ||
			GetActivity() == ACT_HOP ||
			GetActivity() == ACT_LEAP ||
			GetActivity() == ACT_DIESIMPLE)
		{
			idealActivity = GetActivity();
		}
		else
		{
			idealActivity = ACT_HL2MP_GESTURE_RANGE_ATTACK;
		}

		// =========================================================
		// CRITICAL FIX: ensure weapon drives viewmodel anim
		// =========================================================
		CBaseCombatWeapon* pWeapon = GetActiveWeapon();
		if (pWeapon)
		{
			pWeapon->SendWeaponAnim(ACT_VM_PRIMARYATTACK);
		}
	}
	else if (playerAnim == PLAYER_RELOAD)
	{
		idealActivity = ACT_HL2MP_GESTURE_RELOAD;
	}
	else if (playerAnim == PLAYER_IDLE || playerAnim == PLAYER_WALK)
	{
		if (!(GetFlags() & FL_ONGROUND) && GetActivity() == ACT_HL2MP_JUMP)
		{
			idealActivity = GetActivity();
		}
		else
		{
			if (GetFlags() & FL_DUCKING)
			{
				if (speed > 0)
				{
					idealActivity = ACT_HL2MP_WALK_CROUCH;
				}
				else
				{
					idealActivity = ACT_HL2MP_IDLE_CROUCH;
				}
			}
			else
			{
				if (speed > 0)
				{
					idealActivity = ACT_HL2MP_RUN;
				}
				else
				{
					idealActivity = ACT_HL2MP_IDLE;
				}
			}
		}

		idealActivity = TranslateTeamActivity(idealActivity);
	}

	if (idealActivity == ACT_HL2MP_GESTURE_RANGE_ATTACK)
	{
		Activity act = Weapon_TranslateActivity(idealActivity);

		RestartGesture(act);
		Weapon_SetActivity(Weapon_TranslateActivity(ACT_RANGE_ATTACK1), 0);

		return;
	}
	else if (idealActivity == ACT_HL2MP_GESTURE_RELOAD)
	{
		RestartGesture(Weapon_TranslateActivity(idealActivity));
		return;
	}
	else
	{
		SetActivity(idealActivity);

		animDesired = SelectWeightedSequence(Weapon_TranslateActivity(idealActivity));

		if (animDesired == -1)
		{
			animDesired = SelectWeightedSequence(idealActivity);

			if (animDesired == -1)
			{
				animDesired = 0;
			}
		}

		if (GetSequence() == animDesired)
			return;

		m_flPlaybackRate = 1.0f;
		ResetSequence(animDesired);
		SetCycle(0);
		return;
	}
}

extern int	gEvilImpulse101;
//-----------------------------------------------------------------------------
// Purpose: Player reacts to bumping a weapon. 
// Input  : pWeapon - the weapon that the player bumped into.
// Output : Returns true if player picked up the weapon
//-----------------------------------------------------------------------------
bool CHL2MP_Player::BumpWeapon( CBaseCombatWeapon *pWeapon )
{
	CBaseCombatCharacter *pOwner = pWeapon->GetOwner();

	// Can I have this weapon type?
	if ( !IsAllowedToPickupWeapons() )
		return false;
	// ZOMBIE WEAPON RESTRICTION
	if (GetTeamNumber() == TEAM_ZOMBIE)
	{
		const char* pszClass = pWeapon->GetClassname();

		if (Q_stricmp(pszClass, "weapon_knife") != 0 &&
			Q_stricmp(pszClass, "weapon_physcannon") != 0)
		{
			return false; // block everything else
		}
	}
	// Require +use only for weapons physically lying in the world
	if (m_lifeState == LIFE_ALIVE)
	{
		// Only block weapons that are in the world and have physics
		if (!pWeapon->GetOwner() &&
			pWeapon->VPhysicsGetObject() != NULL &&
			!(m_afButtonPressed & IN_USE))
		{
			return false;
		}
	}




	if ( pOwner || !Weapon_CanUse( pWeapon ) || !g_pGameRules->CanHavePlayerItem( this, pWeapon ) )
	{
		if ( gEvilImpulse101 )
		{
			UTIL_Remove( pWeapon );
		}
		return false;
	}

	// Don't let the player fetch weapons through walls (use MASK_SOLID so that you can't pickup through windows)
	if( !pWeapon->FVisible( this, MASK_SOLID ) && !(GetFlags() & FL_NOTARGET) )
	{
		return false;
	}

	bool bOwnsWeaponAlready = !!Weapon_OwnsThisType( pWeapon->GetClassname(), pWeapon->GetSubType());

	if ( bOwnsWeaponAlready == true ) 
	{
		//If we have room for the ammo, then "take" the weapon too.
		 if ( Weapon_EquipAmmoOnly( pWeapon ) )
		 {
			 pWeapon->CheckRespawn();

			 UTIL_Remove( pWeapon );
			 return true;
		 }
		 else
		 {
			 return false;
		 }
	}

	pWeapon->CheckRespawn();

	if (sv_weapon_slot_limit.GetBool())
	{
		int slot = pWeapon->GetSlot();

		// Only restrict real weapon slots (ignore invalid)
		if (slot >= 0 && slot < MAX_WEAPON_SLOTS)
		{
			CBaseCombatWeapon* pExisting = Weapon_GetSlot(slot);

			if (pExisting && pExisting != pWeapon)
			{
				Weapon_DropSlot(slot);
			}
		}
	}

	Weapon_Equip(pWeapon);
	Weapon_Switch(pWeapon); // ensures safe swap

	return true;

}

static CHL2MP_Player* GetCurrentZombiePlayer()
{
	for (int i = 1; i <= gpGlobals->maxClients; ++i)
	{
		CHL2MP_Player* pPlayer = ToHL2MPPlayer(UTIL_PlayerByIndex(i));
		if (!pPlayer)
			continue;

		if (pPlayer->GetTeamNumber() == TEAM_ZOMBIE)
			return pPlayer;
	}

	return NULL;
}

void CHL2MP_Player::ChangeTeam( int iTeam )
{
/*	if ( GetNextTeamChangeTime() >= gpGlobals->curtime )
	{
		char szReturnString[128];
		Q_snprintf( szReturnString, sizeof( szReturnString ), "Please wait %d more seconds before trying to switch teams again.\n", (int)(GetNextTeamChangeTime() - gpGlobals->curtime) );

		ClientPrint( this, HUD_PRINTTALK, szReturnString );
		return;
	}*/

	bool bKill = false;

	// always clear zombie-only abilities before any team change
	m_bFlyMode = false;
	m_bZombieLeapActive = false;
	m_flNextZombieManhackTime = 0.0f; //crow throw
	m_bHealthVisionActive = false; //health vision
	// FULL CLEANUP (NOT JUST STOP)
	variant_t emptyVariant;

	// kill particles
	for (int i = 0; i < m_hFlyParticles.Count(); ++i)
	{
		CBaseEntity* pEnt = m_hFlyParticles[i].Get();
		if (!pEnt)
			continue;

		pEnt->AcceptInput("Kill", this, this, emptyVariant, 0);
		UTIL_Remove(pEnt);
	}

	m_hFlyParticles.RemoveAll();

	// kill anchor
	if (m_hFlyAnchor)
	{
		CBaseEntity* pAnchor = m_hFlyAnchor.Get();
		if (pAnchor)
		{
			pAnchor->AcceptInput("KillHierarchy", this, this, emptyVariant, 0);
			pAnchor->AcceptInput("Kill", this, this, emptyVariant, 0);
			UTIL_Remove(pAnchor);
		}

		m_hFlyAnchor = NULL;
	}

	CBaseViewModel* vm = GetViewModel(0);
	if (vm) vm->RemoveEffects(EF_NODRAW);

	CBaseCombatWeapon* wep = GetActiveWeapon();
	if (wep) wep->RemoveEffects(EF_NODRAW);

	SetRenderMode(kRenderNormal);
	SetRenderColor(255, 255, 255, 255);

	SetCloakStatus(0);
	SetCloakFactor(0.0f);

	if ( HL2MPRules()->IsTeamplay() != true && iTeam != TEAM_SPECTATOR )
	{
		//don't let them try to join combine or rebels during deathmatch.
		iTeam = TEAM_UNASSIGNED;
	}

	if ( HL2MPRules()->IsTeamplay() == true )
	{
		if ( iTeam != GetTeamNumber() && GetTeamNumber() != TEAM_UNASSIGNED )
		{
			bKill = true;
		}
	}
	// =========================================================
	// SINGLE ZOMBIE SAFETY NET
	// =========================================================
	if (iTeam == TEAM_ZOMBIE)
	{
		for (int i = 1; i <= gpGlobals->maxClients; ++i)
		{
			CHL2MP_Player* pPlayer = ToHL2MPPlayer(UTIL_PlayerByIndex(i));

			if (!pPlayer)
				continue;

			// another zombie already exists
			if (pPlayer != this && pPlayer->GetTeamNumber() == TEAM_ZOMBIE)
			{
				ClientPrint(this, HUD_PRINTCENTER, "Zombie slot is already taken.");
				ClientPrint(this, HUD_PRINTTALK, "Only one zombie player is allowed.");
				DevMsg("[ZOMBIE CHECK] Found player %d team %d\n", i, pPlayer->GetTeamNumber());
				return; // HARD BLOCK
			}
		}
	}

	BaseClass::ChangeTeam( iTeam );

	m_flNextTeamChangeTime = gpGlobals->curtime + TEAM_CHANGE_INTERVAL;

	if ( HL2MPRules()->IsTeamplay() == true )
	{
		SetPlayerTeamModel();
	}
	else
	{
		SetPlayerModel();
	}

	if ( iTeam == TEAM_SPECTATOR )
	{
		RemoveAllItems( true );

		State_Transition( STATE_OBSERVER_MODE );
	}

	if ( bKill == true )
	{
		CommitSuicide();
	}
}

bool CHL2MP_Player::HandleCommand_JoinTeam(int team)
{
	if (!GetGlobalTeam(team) || team == 0)
	{
		Warning("HandleCommand_JoinTeam( %d ) - invalid team index.\n", team);
		return false;
	}

	// =========================================================
	// SINGLE ZOMBIE SLOT LOCK
	// Only one player on TEAM_ZOMBIE may exist at once.
	// =========================================================
	if (team == TEAM_ZOMBIE)
	{
		CHL2MP_Player* pCurrentZombie = GetCurrentZombiePlayer();

		// If another player already owns the zombie slot, deny this join.
		if (pCurrentZombie && pCurrentZombie != this)
		{
			ClientPrint(this, HUD_PRINTCENTER, "Zombie slot is already taken.");
			ClientPrint(this, HUD_PRINTTALK, "Only one zombie player is allowed at a time.");
			return false;
		}

		// Optional: if teamplay is required for zombie mode, hard-block here too.
		if (!HL2MPRules()->IsTeamplay())
		{
			ClientPrint(this, HUD_PRINTCENTER, "Zombie team requires teamplay.");
			ClientPrint(this, HUD_PRINTTALK, "Enable teamplay before joining the zombie team.");
			return false;
		}
	}

	if (team == TEAM_SPECTATOR)
	{
		if (!mp_allowspectators.GetInt())
		{
			ClientPrint(this, HUD_PRINTCENTER, "#Cannot_Be_Spectator");
			return false;
		}

		if (GetTeamNumber() != TEAM_UNASSIGNED && !IsDead())
		{
			m_fNextSuicideTime = gpGlobals->curtime;
			CommitSuicide();

			// cancel self-kill frag loss
			IncrementFragCount(1);
		}

		ChangeTeam(TEAM_SPECTATOR);
		return true;
	}
	else
	{
		StopObserverMode();
		State_Transition(STATE_ACTIVE);
	}

	ChangeTeam(team);
	return true;
}

bool CHL2MP_Player::ClientCommand( const CCommand &args )
{
	if (FStrEq(args[0], "zombie_manhack"))
	{
		ThrowZombieManhack(); // ONLY CALL THIS
		return true;
	}

	if ( FStrEq( args[0], "spectate" ) )
	{
		if ( ShouldRunRateLimitedCommand( args ) )
		{
			// instantly join spectators
			HandleCommand_JoinTeam( TEAM_SPECTATOR );	
		}
		return true;
	}
	else if ( FStrEq( args[0], "jointeam" ) ) 
	{
		if ( args.ArgC() < 2 )
		{
			Warning( "Player sent bad jointeam syntax\n" );
		}

		if ( ShouldRunRateLimitedCommand( args ) )
		{
			int iTeam = atoi( args[1] );
			HandleCommand_JoinTeam( iTeam );
		}
		return true;
	}
	else if ( FStrEq( args[0], "joingame" ) )
	{
		return true;
	}

	return BaseClass::ClientCommand( args );
}

void CHL2MP_Player::CheatImpulseCommands( int iImpulse )
{
	switch ( iImpulse )
	{
		case 101:
			{
				if( sv_cheats->GetBool() )
				{
					GiveAllItems();
				}
			}
			break;

		default:
			BaseClass::CheatImpulseCommands( iImpulse );
	}
}

bool CHL2MP_Player::ShouldRunRateLimitedCommand( const CCommand &args )
{
	int i = m_RateLimitLastCommandTimes.Find( args[0] );
	if ( i == m_RateLimitLastCommandTimes.InvalidIndex() )
	{
		m_RateLimitLastCommandTimes.Insert( args[0], gpGlobals->curtime );
		return true;
	}
	else if ( (gpGlobals->curtime - m_RateLimitLastCommandTimes[i]) < HL2MP_COMMAND_MAX_RATE )
	{
		// Too fast.
		return false;
	}
	else
	{
		m_RateLimitLastCommandTimes[i] = gpGlobals->curtime;
		return true;
	}
}

void CHL2MP_Player::CreateViewModel( int index /*=0*/ )
{
	Assert( index >= 0 && index < MAX_VIEWMODELS );

	if ( GetViewModel( index ) )
		return;

	CPredictedViewModel *vm = ( CPredictedViewModel * )CreateEntityByName( "predicted_viewmodel" );
	if ( vm )
	{
		vm->SetAbsOrigin( GetAbsOrigin() );
		vm->SetOwner( this );
		vm->SetIndex( index );
		DispatchSpawn( vm );
		vm->FollowEntity( this, false );
		m_hViewModel.Set( index, vm );
	}
}

bool CHL2MP_Player::BecomeRagdollOnClient( const Vector &force )
{
	return true;
}

// -------------------------------------------------------------------------------- //
// Ragdoll entities.
// -------------------------------------------------------------------------------- //

class CHL2MPRagdoll : public CBaseAnimatingOverlay
{
public:
	DECLARE_CLASS( CHL2MPRagdoll, CBaseAnimatingOverlay );
	DECLARE_SERVERCLASS();

	// Transmit ragdolls to everyone.
	virtual int UpdateTransmitState()
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

public:
	// In case the client has the player entity, we transmit the player index.
	// In case the client doesn't have it, we transmit the player's model index, origin, and angles
	// so they can create a ragdoll in the right place.
	CNetworkHandle( CBaseEntity, m_hPlayer );	// networked entity handle 
	CNetworkVector( m_vecRagdollVelocity );
	CNetworkVector( m_vecRagdollOrigin );
};

LINK_ENTITY_TO_CLASS( hl2mp_ragdoll, CHL2MPRagdoll );

IMPLEMENT_SERVERCLASS_ST_NOBASE( CHL2MPRagdoll, DT_HL2MPRagdoll )
	SendPropVector( SENDINFO(m_vecRagdollOrigin), -1,  SPROP_COORD ),
	SendPropEHandle( SENDINFO( m_hPlayer ) ),
	SendPropModelIndex( SENDINFO( m_nModelIndex ) ),
	SendPropInt		( SENDINFO(m_nForceBone), 8, 0 ),
	SendPropVector	( SENDINFO(m_vecForce), -1, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vecRagdollVelocity ) )
END_SEND_TABLE()


void CHL2MP_Player::CreateRagdollEntity( void )
{
	if ( m_hRagdoll )
	{
		UTIL_RemoveImmediate( m_hRagdoll );
		m_hRagdoll = NULL;
	}

	// If we already have a ragdoll, don't make another one.
	CHL2MPRagdoll *pRagdoll = dynamic_cast< CHL2MPRagdoll* >( m_hRagdoll.Get() );
	
	if ( !pRagdoll )
	{
		// create a new one
		pRagdoll = dynamic_cast< CHL2MPRagdoll* >( CreateEntityByName( "hl2mp_ragdoll" ) );
	}

	if ( pRagdoll )
	{
		pRagdoll->m_hPlayer = this;
		pRagdoll->m_vecRagdollOrigin = GetAbsOrigin();
		pRagdoll->m_vecRagdollVelocity = GetAbsVelocity();
		pRagdoll->m_nModelIndex = m_nModelIndex;
		pRagdoll->m_nForceBone = m_nForceBone;
		pRagdoll->m_vecForce = m_vecTotalBulletForce;
		pRagdoll->SetAbsOrigin( GetAbsOrigin() );
	}

	// ragdolls will be removed on round restart automatically
	m_hRagdoll = pRagdoll;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CHL2MP_Player::FlashlightIsOn( void )
{
	return IsEffectActive( EF_DIMLIGHT );
}

extern ConVar flashlight;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2MP_Player::FlashlightTurnOn( void )
{
	if( flashlight.GetInt() > 0 && IsAlive() )
	{
		AddEffects( EF_DIMLIGHT );
		EmitSound( "HL2Player.FlashlightOn" );
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2MP_Player::FlashlightTurnOff( void )
{
	RemoveEffects( EF_DIMLIGHT );
	
	if( IsAlive() )
	{
		EmitSound( "HL2Player.FlashlightOff" );
	}
}

void CHL2MP_Player::Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget, const Vector *pVelocity )
{
	//Drop a grenade if it's primed.
	if ( GetActiveWeapon() )
	{
		CBaseCombatWeapon *pGrenade = Weapon_OwnsThisType("weapon_frag");

		if ( GetActiveWeapon() == pGrenade )
		{
			if ( ( m_nButtons & IN_ATTACK ) || (m_nButtons & IN_ATTACK2) )
			{
				DropPrimedFragGrenade( this, pGrenade );
				return;
			}
		}
	}

	BaseClass::Weapon_Drop( pWeapon, pvecTarget, pVelocity );
}


void CHL2MP_Player::DetonateTripmines( void )
{
	CBaseEntity *pEntity = NULL;

	while ((pEntity = gEntList.FindEntityByClassname( pEntity, "npc_satchel" )) != NULL)
	{
		CSatchelCharge *pSatchel = dynamic_cast<CSatchelCharge *>(pEntity);
		if (pSatchel->m_bIsLive && pSatchel->GetThrower() == this )
		{
			g_EventQueue.AddEvent( pSatchel, "Explode", 0.20, this, this );
		}
	}

	// Play sound for pressing the detonator
	EmitSound( "Weapon_SLAM.SatchelDetonate" );
}

void CHL2MP_Player::DelayedRoundRestartThink()
{
	const char* currentMap = STRING(gpGlobals->mapname);

	DevMsg("[ROUND RESTART] Executing delayed restart: %s\n", currentMap);

	engine->ChangeLevel(currentMap, NULL);
}

void CHL2MP_Player::Event_Killed( const CTakeDamageInfo &info )
{
	// SAFETY: reset crow state on death
	m_bZombieLeapActive = false;
	m_bFlyMode = false;
	StopParticleEffects(this);
	RemoveEffects(EF_NODRAW);
	StopFlyParticle();
	m_bHealthVisionActive = false;
	m_flNextZombieManhackTime = 0.0f; //crow throw

	if (Q_stricmp(STRING(GetModelName()), "models/crow.mdl") == 0)
	{
		SetModel("models2/humans/group03/male_07.mdl");
		SetupPlayerSoundsByModel("models2/humans/group03/male_07.mdl");
	}

	//update damage info with our accumulated physics force
	CTakeDamageInfo subinfo = info;
	subinfo.SetDamageForce( m_vecTotalBulletForce );

	SetNumAnimOverlays( 0 );

	// Note: since we're dead, it won't draw us on the client, but we don't set EF_NODRAW
	// because we still want to transmit to the clients in our PVS.
	CreateRagdollEntity();

	DetonateTripmines();

	BaseClass::Event_Killed( subinfo );

	if ( info.GetDamageType() & DMG_DISSOLVE )
	{
		if ( m_hRagdoll )
		{
			m_hRagdoll->GetBaseAnimating()->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
		}
	}

	CBaseEntity *pAttacker = info.GetAttacker();

	if ( pAttacker )
	{
		int iScoreToAdd = 1;

		if ( pAttacker == this )
		{
			iScoreToAdd = -1;
		}

		GetGlobalTeam( pAttacker->GetTeamNumber() )->AddScore( iScoreToAdd );
	}

	// --- existing code ---
	if (pAttacker && pAttacker->IsPlayer())
	{
		CHL2MP_Player* pKiller = ToHL2MPPlayer(pAttacker);
		if (pKiller && pKiller != this) // prevent suicides
		{
			// Send only to the killer’s client, not the entire server
			engine->ClientCommand(pKiller->edict(), "player_kill_increment\n");
		}
	}

	FlashlightTurnOff();
	m_lifeState = LIFE_DEAD;

	m_bZombieLeapActive = false;
	m_bFlyMode = false;
	StopParticleEffects(this);
	RemoveEffects(EF_NODRAW);
	SetRenderMode(kRenderNormal);
	SetRenderColor(255, 255, 255, 255);
	SetCloakStatus(0);
	SetCloakFactor(0.0f);
	StopZooming();
}

int CHL2MP_Player::OnTakeDamage(const CTakeDamageInfo& inputInfo)
{
	// ensure manhack damage works
	if (inputInfo.GetAttacker() && inputInfo.GetAttacker()->Classify() == CLASS_MANHACK)
	{
		// allow full damage
		return BaseClass::OnTakeDamage(inputInfo);
	}
	// Zombies ignore fall damage
	if (GetTeamNumber() == TEAM_ZOMBIE && (inputInfo.GetDamageType() & DMG_FALL))
		return 0;

	// return here if the player is in the respawn grace period vs. slams.
	if (gpGlobals->curtime < m_flSlamProtectTime && (inputInfo.GetDamageType() == DMG_BLAST))
		return 0;

	m_vecTotalBulletForce += inputInfo.GetDamageForce();

	gamestats->Event_PlayerDamage(this, inputInfo);

	return BaseClass::OnTakeDamage(inputInfo);
}

void CHL2MP_Player::DeathSound( const CTakeDamageInfo &info )
{
	if ( m_hRagdoll && m_hRagdoll->GetBaseAnimating()->IsDissolving() )
		 return;

	char szStepSound[128];

	Q_snprintf( szStepSound, sizeof( szStepSound ), "%s.Die", GetPlayerModelSoundPrefix() );

	const char *pModelName = STRING( GetModelName() );

	CSoundParameters params;
	if ( GetParametersForSound( szStepSound, params, pModelName ) == false )
		return;

	Vector vecOrigin = GetAbsOrigin();
	
	CRecipientFilter filter;
	filter.AddRecipientsByPAS( vecOrigin );

	EmitSound_t ep;
	ep.m_nChannel = params.channel;
	ep.m_pSoundName = params.soundname;
	ep.m_flVolume = params.volume;
	ep.m_SoundLevel = params.soundlevel;
	ep.m_nFlags = 0;
	ep.m_nPitch = params.pitch;
	ep.m_pOrigin = &vecOrigin;

	EmitSound( filter, entindex(), ep );
}

CBaseEntity* CHL2MP_Player::EntSelectSpawnPoint( void )
{
	CBaseEntity *pSpot = NULL;
	CBaseEntity *pLastSpawnPoint = g_pLastSpawn;
	edict_t		*player = edict();
	const char *pSpawnpointName = "info_player_deathmatch";

	if ( HL2MPRules()->IsTeamplay() == true )
	{
		if ( GetTeamNumber() == TEAM_COMBINE )
		{
			pSpawnpointName = "info_player_combine";
			pLastSpawnPoint = g_pLastCombineSpawn;
		}
		else if ( GetTeamNumber() == TEAM_REBELS )
		{
			pSpawnpointName = "info_player_rebel";
			pLastSpawnPoint = g_pLastRebelSpawn;
		}

		else if (GetTeamNumber() == TEAM_ZOMBIE)
		{
			pSpawnpointName = "info_player_zombie";
			pLastSpawnPoint = g_pLastZombieSpawn;
		}

		if ( gEntList.FindEntityByClassname( NULL, pSpawnpointName ) == NULL )
		{
			pSpawnpointName = "info_player_deathmatch";
			pLastSpawnPoint = g_pLastSpawn;
		}
	}

	pSpot = pLastSpawnPoint;
	// Randomize the start spot
	for ( int i = random->RandomInt(1,5); i > 0; i-- )
		pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );
	if ( !pSpot )  // skip over the null point
		pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );

	CBaseEntity *pFirstSpot = pSpot;

	do 
	{
		if ( pSpot )
		{
			// check if pSpot is valid
			if ( g_pGameRules->IsSpawnPointValid( pSpot, this ) )
			{
				if ( pSpot->GetLocalOrigin() == vec3_origin )
				{
					pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );
					continue;
				}

				// if so, go to pSpot
				goto ReturnSpot;
			}
		}
		// increment pSpot
		pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );
	} while ( pSpot != pFirstSpot ); // loop if we're not back to the start

	// we haven't found a place to spawn yet,  so kill any guy at the first spawn point and spawn there
	if ( pSpot )
	{
		CBaseEntity *ent = NULL;
		for ( CEntitySphereQuery sphere( pSpot->GetAbsOrigin(), 128 ); (ent = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity() )
		{
			// if ent is a client, kill em (unless they are ourselves)
			if ( ent->IsPlayer() && !(ent->edict() == player) )
				ent->TakeDamage( CTakeDamageInfo( GetContainingEntity(INDEXENT(0)), GetContainingEntity(INDEXENT(0)), 300, DMG_GENERIC ) );
		}
		goto ReturnSpot;
	}

	if ( !pSpot  )
	{
		pSpot = gEntList.FindEntityByClassname( pSpot, "info_player_start" );

		if ( pSpot )
			goto ReturnSpot;
	}

ReturnSpot:

	if ( HL2MPRules()->IsTeamplay() == true )
	{
		if ( GetTeamNumber() == TEAM_COMBINE )
		{
			g_pLastCombineSpawn = pSpot;
		}
		else if ( GetTeamNumber() == TEAM_REBELS ) 
		{
			g_pLastRebelSpawn = pSpot;
		}
		else if (GetTeamNumber() == TEAM_ZOMBIE)
		{
			g_pLastZombieSpawn = pSpot;
		}
	}

	g_pLastSpawn = pSpot;

	m_flSlamProtectTime = gpGlobals->curtime + 0.5;

	return pSpot;
} 


CON_COMMAND( timeleft, "prints the time remaining in the match" )
{
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_GetCommandClient() );

	int iTimeRemaining = (int)HL2MPRules()->GetMapRemainingTime();
    
	if ( iTimeRemaining == 0 )
	{
		if ( pPlayer )
		{
			ClientPrint( pPlayer, HUD_PRINTTALK, "The HELLish sLAUGHTER never ends." );
		}
		else
		{
			Msg( "* No Time Limit *\n" );
		}
	}
	else
	{
		int iMinutes, iSeconds;
		iMinutes = iTimeRemaining / 60;
		iSeconds = iTimeRemaining % 60;

		char minutes[8];
		char seconds[8];

		Q_snprintf( minutes, sizeof(minutes), "%d", iMinutes );
		Q_snprintf( seconds, sizeof(seconds), "%2.2d", iSeconds );

		if ( pPlayer )
		{
			ClientPrint( pPlayer, HUD_PRINTTALK, "Time left in HELL: %s1:%s2", minutes, seconds );
		}
		else
		{
			Msg( "Time Remaining:  %s:%s\n", minutes, seconds );
		}
	}	
}


void CHL2MP_Player::Reset()
{	
	ResetDeathCount();
	ResetFragCount();
}

bool CHL2MP_Player::IsReady()
{
	return m_bReady;
}

void CHL2MP_Player::SetReady( bool bReady )
{
	m_bReady = bReady;
}

void CHL2MP_Player::CheckChatText( char *p, int bufsize )
{
	//Look for escape sequences and replace

	char *buf = new char[bufsize];
	int pos = 0;

	// Parse say text for escape sequences
	for ( char *pSrc = p; pSrc != NULL && *pSrc != 0 && pos < bufsize-1; pSrc++ )
	{
		// copy each char across
		buf[pos] = *pSrc;
		pos++;
	}

	buf[pos] = '\0';

	// copy buf back into p
	Q_strncpy( p, buf, bufsize );

	delete[] buf;	

	const char *pReadyCheck = p;

	HL2MPRules()->CheckChatForReadySignal( this, pReadyCheck );
}

void CHL2MP_Player::State_Transition( HL2MPPlayerState newState )
{
	State_Leave();
	State_Enter( newState );
}


void CHL2MP_Player::State_Enter( HL2MPPlayerState newState )
{
	m_iPlayerState = newState;
	m_pCurStateInfo = State_LookupInfo( newState );

	// Initialize the new state.
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnEnterState )
		(this->*m_pCurStateInfo->pfnEnterState)();
}


void CHL2MP_Player::State_Leave()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnLeaveState )
	{
		(this->*m_pCurStateInfo->pfnLeaveState)();
	}
}


void CHL2MP_Player::State_PreThink()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnPreThink )
	{
		(this->*m_pCurStateInfo->pfnPreThink)();
	}
}


CHL2MPPlayerStateInfo *CHL2MP_Player::State_LookupInfo( HL2MPPlayerState state )
{
	// This table MUST match the 
	static CHL2MPPlayerStateInfo playerStateInfos[] =
	{
		{ STATE_ACTIVE,			"STATE_ACTIVE",			&CHL2MP_Player::State_Enter_ACTIVE, NULL, &CHL2MP_Player::State_PreThink_ACTIVE },
		{ STATE_OBSERVER_MODE,	"STATE_OBSERVER_MODE",	&CHL2MP_Player::State_Enter_OBSERVER_MODE,	NULL, &CHL2MP_Player::State_PreThink_OBSERVER_MODE }
	};

	for ( int i=0; i < ARRAYSIZE( playerStateInfos ); i++ )
	{
		if ( playerStateInfos[i].m_iPlayerState == state )
			return &playerStateInfos[i];
	}

	return NULL;
}

bool CHL2MP_Player::StartObserverMode(int mode)
{
	//we only want to go into observer mode if the player asked to, not on a death timeout
	if ( m_bEnterObserver == true )
	{
		VPhysicsDestroyObject();
		return BaseClass::StartObserverMode( mode );
	}
	return false;
}

void CHL2MP_Player::StopObserverMode()
{
	m_bEnterObserver = false;
	BaseClass::StopObserverMode();
}

void CHL2MP_Player::State_Enter_OBSERVER_MODE()
{
	int observerMode = m_iObserverLastMode;
	if ( IsNetClient() )
	{
		const char *pIdealMode = engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_spec_mode" );
		if ( pIdealMode )
		{
			observerMode = atoi( pIdealMode );
			if ( observerMode <= OBS_MODE_FIXED || observerMode > OBS_MODE_ROAMING )
			{
				observerMode = m_iObserverLastMode;
			}
		}
	}
	m_bEnterObserver = true;
	StartObserverMode( observerMode );
}

void CHL2MP_Player::State_PreThink_OBSERVER_MODE()
{
	// Make sure nobody has changed any of our state.
	//	Assert( GetMoveType() == MOVETYPE_FLY );
	Assert( m_takedamage == DAMAGE_NO );
	Assert( IsSolidFlagSet( FSOLID_NOT_SOLID ) );
	//	Assert( IsEffectActive( EF_NODRAW ) );

	// Must be dead.
	Assert( m_lifeState == LIFE_DEAD );
	Assert( pl.deadflag );
}


void CHL2MP_Player::State_Enter_ACTIVE()
{
	SetMoveType(MOVETYPE_WALK);
	m_Local.m_iHideHUD = 0;

}


void CHL2MP_Player::State_PreThink_ACTIVE()
{
	//we don't really need to do anything here. 
	//This state_prethink structure came over from CS:S and was doing an assert check that fails the way hl2dm handles death
}

float CHL2MP_Player::MaxSpeed() const
{
	if (GetTeamNumber() == TEAM_ZOMBIE)
	{
		// ALT = SPRINT (fastest)
		if (m_nButtons & IN_WALK)
			return 560.0f;

		// SHIFT = WALK (slow)
		if (m_nButtons & IN_SPEED)
			return 35.0f;

		// DEFAULT RUN
		return 260.0f;
	}

	return BaseClass::MaxSpeed();
}

//-----------------------------------------------------------------------------
// Purpose: Classify zombie team players separately
//-----------------------------------------------------------------------------
Class_T CHL2MP_Player::Classify(void)
{
	if (GetTeamNumber() == TEAM_ZOMBIE)
		return CLASS_ZOMBIE_PLAYER;

	return BaseClass::Classify();
}


//-----------------------------------------------------------------------------
// Purpose: Spawn and throw a manhack forward
//-----------------------------------------------------------------------------
void CHL2MP_Player::ThrowZombieManhack()
{
	int liveCount = 0;
	CBaseEntity* pScan = NULL;

	while ((pScan = gEntList.FindEntityByClassname(pScan, "npc_manhack")) != NULL)
	{
		if (pScan->IsMarkedForDeletion())
			continue;

		if (!pScan->IsAlive())
			continue;

		CNPC_Manhack* pOwnedManhack = assert_cast<CNPC_Manhack*>(pScan);
		if (!pOwnedManhack)
			continue;

		if (pOwnedManhack->m_hZombieOwner == this)
		{
			liveCount++;
		}
	}

	if (liveCount >= 3)
	{
		ClientPrint(this, HUD_PRINTCENTER, "Max Manhacks Reached (3)");
		EmitSound("Weapon_SMG1.Empty");
		return;
	}

	if (!BaseClass::IsAlive())
		return;

	if (GetTeamNumber() != TEAM_ZOMBIE)
		return;

	if (gpGlobals->curtime < m_flNextZombieManhackTime)
		return;

	if (m_bZombieLeapActive)
		return;

	Vector forward;
	AngleVectors(QAngle(0, EyeAngles().y, 0), &forward);

	Vector vecSrc = EyePosition() + forward * 40.0f + Vector(0, 0, -6.0f);
	QAngle angSpawn(0, EyeAngles().y, 0);

	trace_t tr;
	UTIL_TraceHull(
		EyePosition(),
		vecSrc,
		Vector(-16, -16, -16),
		Vector(16, 16, 16),
		MASK_SOLID,
		this,
		COLLISION_GROUP_NONE,
		&tr
	);

	vecSrc = tr.endpos;

	CBaseEntity* pEnt = CreateEntityByName("npc_manhack");
	if (!pEnt)
		return;

	pEnt->SetAbsOrigin(vecSrc);
	pEnt->SetAbsAngles(angSpawn);

	DispatchSpawn(pEnt);
	pEnt->Activate();

	CNPC_Manhack* pManhack = assert_cast<CNPC_Manhack*>(pEnt);
	if (pManhack)
	{
		pManhack->SetHealth(30);
		pManhack->SetMaxHealth(30);
		pManhack->StartEngine(false);
		pManhack->Wake();

		pManhack->m_hZombieOwner = this;

		pManhack->SetSolid(SOLID_BBOX);
		pManhack->SetCollisionGroup(COLLISION_GROUP_NPC);
		pManhack->SetHullSizeNormal();
		pManhack->SetBloodColor(BLOOD_COLOR_MECH);
		pManhack->m_takedamage = DAMAGE_YES;
		pManhack->RemoveSolidFlags(FSOLID_NOT_SOLID);
	}

	Vector vecVel = forward * 250.0f + Vector(0, 0, 80.0f);
	pEnt->SetAbsVelocity(vecVel);

	EmitSound("NPC_MetroPolice.DeployManhack");

	m_flNextZombieManhackTime = gpGlobals->curtime + 8.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHL2MP_Player::CanHearAndReadChatFrom( CBasePlayer *pPlayer )
{
	// can always hear the console unless we're ignoring all chat
	if ( !pPlayer )
		return false;

	return true;
}
