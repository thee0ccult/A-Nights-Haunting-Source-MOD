#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "c_basehlplayer.h" //alternative #include "c_baseplayer.h"

#include <vgui/IScheme.h>
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
// memdbgon must be the last include file in a .cpp file!
#include "tier0/memdbgon.h"

/**
 * Simple HUD element for displaying a sniper scope on screen
 */
class CHudScope : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudScope, vgui::Panel);

public:
	CHudScope(const char* pElementName);

	void Init();
	void MsgFunc_ShowScope(bf_read& msg);

protected:
	virtual void ApplySchemeSettings(vgui::IScheme* scheme);
	virtual void Paint(void);

private:
	bool			m_bShow;
	//CHudTexture* m_pScope;// hacked out for multisight
	CHudTexture* m_pScopell;
	CHudTexture* m_pScopelr;
	CHudTexture* m_pScopeul;
	CHudTexture* m_pScopeur;
};

DECLARE_HUDELEMENT(CHudScope);
DECLARE_HUD_MESSAGE(CHudScope, ShowScope);

using namespace vgui;

/**
 * Constructor - generic HUD element initialization stuff. Make sure our 2 member variables
 * are instantiated.
 */
CHudScope::CHudScope(const char* pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudScope")
{
	vgui::Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	m_bShow = false;
	//m_pScope = 0;//hacked for multiscope
	m_pScopell = 0;
	m_pScopelr = 0;
	m_pScopeul = 0;
	m_pScopeur = 0;

	// Scope will not show when the player is dead
	SetHiddenBits(HIDEHUD_PLAYERDEAD);

	// fix for users with diffrent screen ratio (Lodle)
	int screenWide, screenTall;
	GetHudSize(screenWide, screenTall);
	SetBounds(0, 0, screenWide, screenTall);

}

/**
 * Hook up our HUD message, and make sure we are not showing the scope
 */
void CHudScope::Init()
{
	HOOK_HUD_MESSAGE(CHudScope, ShowScope);

	m_bShow = false;
}

/**
 * Load  in the scope material here
 */
void CHudScope::ApplySchemeSettings(vgui::IScheme* scheme)
{
	BaseClass::ApplySchemeSettings(scheme);

	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);

	if (!m_pScopell)
	{
		m_pScopell = gHUD.GetIcon("scopell");
	}
	if (!m_pScopelr)
	{
		m_pScopelr = gHUD.GetIcon("scopelr");
	}
	if (!m_pScopeul)
	{
		m_pScopeul = gHUD.GetIcon("scopeul");
	}
	if (!m_pScopeur)
	{
		m_pScopeur = gHUD.GetIcon("scopeur");
	}
}

/**
 * Simple - if we want to show the scope, draw it. Otherwise don't.
 */
void CHudScope::Paint(void)
{
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
	{
		return;
	}

	if (m_bShow)
	{
		int w = GetWide(); // Width of player's screen resolution.
		int h = GetTall(); // Height of player's screen resolution.

		// Our scope's width must be 4/3 the height of the screen as that is how Valve designed the textures.
		// This means that one quadrant will have a width exactly half of that, 2/3 the height of the screen.
		int x = 2 * h / 3;
		int y = h / 2; // Our scope blocks will each be half the height of the screen.

		int margin = (w - 4 * h / 3) / 2; // This is the width of the black bars on the sides of the screen.

		surface()->DrawSetColor(Color(0, 0, 0, 255));
		surface()->DrawFilledRect(0, 0, margin, h); //Fill in the left side

		surface()->DrawSetColor(Color(0, 0, 0, 255));
		surface()->DrawFilledRect(margin + 2 * x, 0, w, h); //Fill in the right side, it's offset horizontally by the width of one margin plus two scope block widths.

		// Source engine displays UI elements with an inverted-y axis.
		// This means that the point (0,0) is located in the upper left corner of the screen and is why our lower scope blocks must be offset by y = h / 2.
		m_pScopell->DrawSelf(margin, y, x, y, Color(255, 255, 255, 255));
		m_pScopeul->DrawSelf(margin, 0, x, y, Color(255, 255, 255, 255));
		m_pScopelr->DrawSelf(margin + x, y, x, y, Color(255, 255, 255, 255));
		m_pScopeur->DrawSelf(margin + x, 0, x, y, Color(255, 255, 255, 255));

		pPlayer->m_Local.m_iHideHUD |= HIDEHUD_CROSSHAIR; // This hides the crosshair while the scope is active.
	}
	else if ((pPlayer->m_Local.m_iHideHUD & HIDEHUD_CROSSHAIR) != 0)
	{
		pPlayer->m_Local.m_iHideHUD &= ~HIDEHUD_CROSSHAIR;
	}
}


/**
 * Callback for our message - set the show variable to whatever
 * boolean value is received in the message
 */
void CHudScope::MsgFunc_ShowScope(bf_read& msg)
{
	m_bShow = msg.ReadByte();
}