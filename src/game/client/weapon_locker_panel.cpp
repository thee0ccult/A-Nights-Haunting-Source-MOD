#include "cbase.h"
#include "weapon_locker_panel.h"
#include "iclientmode.h"
#include <vgui/ISurface.h>
#include <vgui_controls/RichText.h>
#include <tier1/strtools.h>
#include "filesystem.h"
#include "KeyValues.h"
using namespace vgui;

CWeaponLockerPanel::CWeaponLockerPanel() :
    Frame(nullptr, "WeaponLockerPanel")
{
    SetSize(300, 240);
    SetPos(20, 400);
    SetTitle("", false);
    SetMoveable(false);
    SetSizeable(false);
    SetCloseButtonVisible(false);
    SetVisible(false);

    m_pText = new RichText(this, "LockerText");
    m_pText->SetBounds(10, 10, 280, 220);
    m_pText->SetVerticalScrollbar(false);

    m_iEntityIndex = -1;
    m_iCurrentCategory = -1;
    m_iCurrentPage = 0;
    m_bInCategory = false;
}

CWeaponLockerPanel::~CWeaponLockerPanel()
{
    ClearData();
}

void CWeaponLockerPanel::ClearData()
{
    for (int i = 0; i < m_CategoryNames.Count(); ++i)
        delete m_CategoryNames[i];

    for (int i = 0; i < m_CategoryWeapons.Count(); ++i)
    {
        CUtlVector<CUtlString*>* list = m_CategoryWeapons[i];

        if (list)
        {
            for (int j = 0; j < list->Count(); ++j)
                delete (*list)[j];

            delete list;
        }
    }

    m_CategoryNames.RemoveAll();
    m_CategoryWeapons.RemoveAll();
}

void CWeaponLockerPanel::Open(int entIndex)
{
    m_iEntityIndex = entIndex;
    m_iCurrentCategory = -1;
    m_iCurrentPage = 0;
    m_bInCategory = false;

    SetVisible(true);
    MakePopup();
    MoveToFront();

    ShowMainMenu();
}

void CWeaponLockerPanel::AddCategory(const char* name)
{
    CUtlString* catName = new CUtlString(name);
    m_CategoryNames.AddToTail(catName);

    CUtlVector<CUtlString*>* weaponList = new CUtlVector<CUtlString*>();
    m_CategoryWeapons.AddToTail(weaponList);
}

void CWeaponLockerPanel::AddWeapon(int category, const char* weapon)
{
    if (category < 0 || category >= m_CategoryWeapons.Count())
        return;

    CUtlString* weaponStr = new CUtlString(weapon);
    m_CategoryWeapons[category]->AddToTail(weaponStr);
}

void CWeaponLockerPanel::ShowMainMenu()
{
    m_pText->SetText("");
    m_pText->GotoTextStart();

    const int pageSize = 8;
    int totalItems = m_CategoryNames.Count();

    // ---- Safety clamp ----
    if (totalItems <= 0)
    {
        m_pText->InsertString("No categories available\n");
        m_pText->InsertString("0. Exit\n");
        return;
    }

    if (m_iCurrentPage < 0)
        m_iCurrentPage = 0;

    int maxPages = (totalItems - 1) / pageSize;

    if (m_iCurrentPage > maxPages)
        m_iCurrentPage = 0;
    // ----------------------

    int start = m_iCurrentPage * pageSize;
    int end = MIN(start + pageSize, totalItems);

    char buffer[256];
    int button = 1;

    for (int i = start; i < end; ++i)
    {
        Q_snprintf(buffer, sizeof(buffer),
            "%d. %s\n",
            button,
            m_CategoryNames[i]->String());

        m_pText->InsertString(buffer);
        button++;
    }

    if (m_iCurrentPage < maxPages)
        m_pText->InsertString("9. Next\n");

    m_pText->InsertString("0. Exit\n");
}

void CWeaponLockerPanel::ShowCategory(int index)
{
    m_iCurrentPage = MAX(0, m_iCurrentPage);

    m_pText->SetText("");
    m_pText->GotoTextStart();

    const int pageSize = 8;
    CUtlVector<CUtlString*>* list = m_CategoryWeapons[index];

    int maxPages = (list->Count() - 1) / pageSize;

    if (m_iCurrentPage > maxPages)
        m_iCurrentPage = 0;

    int start = m_iCurrentPage * pageSize;
    int end = MIN(start + pageSize, list->Count());

    char buffer[256];
    int button = 1;

    for (int i = start; i < end; ++i)
    {
        Q_snprintf(buffer, sizeof(buffer),
            "%d. %s\n",
            button,
            (*list)[i]->String());

        m_pText->InsertString(buffer);
        button++;
    }

    if (end < list->Count())
        m_pText->InsertString("9. Next\n");

    m_pText->InsertString("0. Back\n");
}

void CWeaponLockerPanel::OnKeyCodePressed(KeyCode code)
{
    if (code < KEY_0 || code > KEY_9)
        return;

    int num = (code == KEY_0) ? 0 : (code - KEY_1 + 1);
    const int pageSize = 8;

    // ===== MAIN MENU =====
    if (!m_bInCategory)
    {
        if (num == 0)
        {
            SetVisible(false);
            return;
        }

        if (num == 9)
        {
            m_iCurrentPage++;
            ShowMainMenu();
            return;
        }

        int index = m_iCurrentPage * pageSize + (num - 1);

        if (index >= 0 && index < m_CategoryNames.Count())
        {
            m_bInCategory = true;
            m_iCurrentCategory = index;
            m_iCurrentPage = 0;
            ShowCategory(m_iCurrentCategory);
        }

        return;
    }

    // ===== CATEGORY MENU =====
    if (m_bInCategory)
    {
        if (num == 0)
        {
            m_bInCategory = false;
            m_iCurrentPage = 0;
            ShowMainMenu();
            return;
        }

        if (num == 9)
        {
            m_iCurrentPage++;
            ShowCategory(m_iCurrentCategory);
            return;
        }

        CUtlVector<CUtlString*>* list = m_CategoryWeapons[m_iCurrentCategory];
        int index = m_iCurrentPage * pageSize + (num - 1);

        if (index >= 0 && index < list->Count())
        {
            const char* weapon = (*list)[index]->String();

            char cmd[256];
            Q_snprintf(cmd, sizeof(cmd),
                "anh_select_weapon %d %s",
                m_iEntityIndex,
                weapon);

            engine->ClientCmd(cmd);

            SetVisible(false);
        }
    }
}
void CWeaponLockerPanel::LoadFromScript(const char* scriptFile)
{
    ClearData();

    m_iCurrentPage = 0;
    m_iCurrentCategory = -1;
    m_bInCategory = false;

    KeyValues* kv = new KeyValues("WeaponLocker");

    if (!kv->LoadFromFile(filesystem, scriptFile, "GAME"))
    {
        Warning("WeaponLockerPanel: Failed to load %s\n", scriptFile);
        kv->deleteThis();
        return;
    }

    for (KeyValues* cat = kv->GetFirstSubKey(); cat; cat = cat->GetNextKey())
    {
        CUtlString* catName = new CUtlString(cat->GetName());
        m_CategoryNames.AddToTail(catName);

        CUtlVector<CUtlString*>* weaponList = new CUtlVector<CUtlString*>();

        for (KeyValues* weapon = cat->GetFirstSubKey(); weapon; weapon = weapon->GetNextKey())
        {
            const char* weaponName = weapon->GetName();

            if (weaponName && Q_strlen(weaponName) > 0)
            {
                weaponList->AddToTail(new CUtlString(weaponName));
            }
        }

        m_CategoryWeapons.AddToTail(weaponList);
    }

    kv->deleteThis();
}