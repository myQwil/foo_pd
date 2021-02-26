#include "stdafx.h"
#include "resource.h"
#include <helpers/atl-misc.h>

// 133cd930 - f42c - 4700 - 820a - 38613622666d
static const GUID guid_cfg_refresh =
{ 0x133cd930 ,0xf42c ,0x4700 ,{ 0x82 ,0x0a ,0x38 ,0x61 ,0x36 ,0x22 ,0x66 ,0x6d } };

cfg_bool cfg_refresh(guid_cfg_refresh ,0);

class CMyPreferences
: public CDialogImpl<CMyPreferences> ,public preferences_page_instance {
public:
	CMyPreferences(preferences_page_callback::ptr callback) : m_callback(callback) {}

	//dialog resource ID
	enum { IDD = IDD_MYPREFERENCES };

	t_uint32 get_state();
	void apply();
	void reset();
	CCheckBox cb;

	//WTL message map
	BEGIN_MSG_MAP(CMyPreferences)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_CODE_HANDLER_EX(BN_CLICKED ,OnButtonClicked)
	END_MSG_MAP()

private:
	BOOL OnInitDialog(CWindow ,LPARAM);
	void OnButtonClicked(UINT uNotifyCode ,int nID ,CWindow wndCtl);
	bool HasChanged();
	void OnChanged();
	const preferences_page_callback::ptr m_callback;
};

BOOL CMyPreferences::OnInitDialog(CWindow ,LPARAM) {
	cb = CCheckBox(GetDlgItem(IDC_CHK_REFRESH));
	cb.ToggleCheck(cfg_refresh);
	return FALSE;
}

void CMyPreferences::OnButtonClicked(UINT uNotifyCode ,int nID ,CWindow wndCtl) {
	OnChanged();
}

t_uint32 CMyPreferences::get_state() {
	t_uint32 state = preferences_state::resettable;
	if (HasChanged()) state |= preferences_state::changed;
	return state;
}

void CMyPreferences::reset() {
	cb.ToggleCheck(false);
	OnChanged();
}

void CMyPreferences::apply() {
	cfg_refresh = cb.IsChecked();
	OnChanged();
}

bool CMyPreferences::HasChanged() {
	return cfg_refresh != cb.IsChecked();
}

void CMyPreferences::OnChanged() {
	m_callback->on_state_changed();
}

class preferences_page_myimpl : public preferences_page_impl<CMyPreferences> {
public:
	const char *get_name() {
		return "Pd Player";
	}

	GUID get_guid() {
		// 8431d9bd - bdc2 - 4986 - 9a14 - cf8b813a8ec8
		static const GUID guid = 
		{ 0x8431d9bd ,0xbdc2 ,0x4986 ,{ 0x9a ,0x14 ,0xcf ,0x8b ,0x81 ,0x3a ,0x8e ,0xc8 } };
		return guid;
	}

	GUID get_parent_guid() {
		return guid_tools;
	}
};

static preferences_page_factory_t<preferences_page_myimpl> g_preferences_page_myimpl_factory;
