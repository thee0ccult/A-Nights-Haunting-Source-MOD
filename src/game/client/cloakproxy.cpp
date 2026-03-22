#include "cbase.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterialproxy.h"
#include "c_baseentity.h"
#include "c_baseplayer.h"
#include "c_basecombatcharacter.h"
#include "c_basecombatweapon.h"
#include "baseviewmodel_shared.h"

class C_CloakProxy : public IMaterialProxy
{
public:
	C_CloakProxy() : m_pCloakFactorVar(nullptr) {}
	virtual ~C_CloakProxy() {}

	virtual bool Init(IMaterial* pMaterial, KeyValues* pKeyValues)
	{
		bool found = false;

		m_pCloakFactorVar = pMaterial->FindVar("$cloakfactor", &found, false);
		if (!found || !m_pCloakFactorVar)
			return false;

		return true;
	}

	virtual void OnBind(void* pBindArg)
	{
		if (!pBindArg || !m_pCloakFactorVar)
			return;

		IClientRenderable* pRenderable = reinterpret_cast<IClientRenderable*>(pBindArg);
		if (!pRenderable)
			return;

		IClientUnknown* pUnknown = pRenderable->GetIClientUnknown();
		if (!pUnknown)
			return;

		C_BaseEntity* pEntity = pUnknown->GetBaseEntity();
		if (!pEntity)
			return;

		float cloak = 0.0f;

		// Viewmodel
		if (C_BaseViewModel* vm = dynamic_cast<C_BaseViewModel*>(pEntity))
		{
			if (C_BasePlayer* owner = ToBasePlayer(vm->GetOwner()))
				cloak = owner->GetCloakFactor();
		}
		// Weapon
		else if (C_BaseCombatWeapon* wep = dynamic_cast<C_BaseCombatWeapon*>(pEntity))
		{
			if (C_BaseCombatCharacter* owner = ToBaseCombatCharacter(wep->GetOwner()))
				cloak = owner->GetCloakFactor();
		}
		// Player / NPC
		else if (C_BaseCombatCharacter* bcc = dynamic_cast<C_BaseCombatCharacter*>(pEntity))
		{
			cloak = bcc->GetCloakFactor();
		}

		m_pCloakFactorVar->SetFloatValue(cloak);
	}

	virtual IMaterial* GetMaterial()
	{
		return nullptr;
	}

	virtual void Release()
	{
		delete this;
	}

private:
	IMaterialVar* m_pCloakFactorVar;
};

EXPOSE_INTERFACE(C_CloakProxy, IMaterialProxy, "Invisibility" IMATERIAL_PROXY_INTERFACE_VERSION);