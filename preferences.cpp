#include "stdafx.h"
#include "resource.h"
#include "guid.h"

cfg_bool cfg_refresh(guid_cfg_refresh, 0);

class CMyPreferences : public CDialogImpl<CMyPreferences>, public preferences_page_instance {
public:
	// Constructor - invoked by preferences_page_impl helpers
	// don't do Create() in here, preferences_page_impl does this for us
	CMyPreferences(preferences_page_callback::ptr callback) : m_callback(callback) {}

	// Note that we don't bother doing anything regarding destruction of our class.
	// The host ensures that our dialog is destroyed first, then the last reference to our
	// preferences_page_instance object is released, causing our object to be deleted.

	//dialog resource ID
	enum {IDD = IDD_MYPREFERENCES};
	// preferences_page_instance methods (not all of them - get_wnd()
	// is supplied by preferences_page_impl helpers)
	t_uint32 get_state();
	void apply();
	void reset();
	CCheckBox cb;

	//WTL message map
	BEGIN_MSG_MAP(CMyPreferences)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_CODE_HANDLER_EX(BN_CLICKED, OnButtonClicked)
	END_MSG_MAP()
private:
	BOOL OnInitDialog(CWindow, LPARAM);
	void OnButtonClicked(UINT uNotifyCode, int nID, CWindow wndCtl);
	bool HasChanged();
	void OnChanged();

	const preferences_page_callback::ptr m_callback;
};

BOOL CMyPreferences::OnInitDialog(CWindow, LPARAM) {
	cb = CCheckBox(GetDlgItem(IDC_CHK_REFRESH));
	cb.ToggleCheck(cfg_refresh);
	return FALSE;
}

void CMyPreferences::OnButtonClicked(UINT uNotifyCode, int nID, CWindow wndCtl) {
	// not much to do here
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
	OnChanged(); // our dialog content has not changed but the flags have
	// our currently shown values now match the settings so the apply button can be disabled
}

bool CMyPreferences::HasChanged() {
	return cfg_refresh != cb.IsChecked();
}
void CMyPreferences::OnChanged() {
	// tell the host that our state has changed to enable/disable the apply button appropriately.
	m_callback->on_state_changed();
}

class preferences_page_myimpl : public preferences_page_impl<CMyPreferences> {
	// preferences_page_impl<> helper deals with instantiation of our dialog;
	// inherits from preferences_page_v3.
public:
	const char * get_name() {return "Pd Player";}
	GUID get_guid() {
		// 8431d9bd - bdc2 - 4986 - 9a14 - cf8b813a8ec8
		static const GUID guid = 
		{ 0x8431d9bd, 0xbdc2, 0x4986, { 0x9a, 0x14, 0xcf, 0x8b, 0x81, 0x3a, 0x8e, 0xc8 } };
		return guid;
	}
	GUID get_parent_guid() {return guid_tools;}
};

static preferences_page_factory_t<preferences_page_myimpl> g_preferences_page_myimpl_factory;
