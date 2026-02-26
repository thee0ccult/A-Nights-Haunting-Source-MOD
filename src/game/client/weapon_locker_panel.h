#pragma once

#include <vgui/VGUI.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/RichText.h>
#include <utlvector.h>
#include <utlstring.h>

class CWeaponLockerPanel : public vgui::Frame
{
    DECLARE_CLASS_SIMPLE(CWeaponLockerPanel, vgui::Frame);

public:
    CWeaponLockerPanel();
    ~CWeaponLockerPanel();

    void Open(int entIndex);
    void ClearData();

    void AddCategory(const char* name);
    void AddWeapon(int category, const char* weapon);
    void LoadFromScript(const char* scriptFile);
    void OnKeyCodePressed(vgui::KeyCode code) override;

private:

    void ShowMainMenu();     // ADD THIS
    void ShowCategory(int index); // ADD THIS

    CUtlVector<CUtlString*> m_CategoryNames;
    CUtlVector< CUtlVector<CUtlString*>* > m_CategoryWeapons;

    vgui::RichText* m_pText;

    int  m_iEntityIndex;
    int  m_iCurrentCategory;
    bool m_bInCategory;
    int  m_iCurrentPage;
};