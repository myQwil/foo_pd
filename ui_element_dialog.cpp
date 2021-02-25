#include "stdafx.h"

#include "resource.h"
#include "PdBase.hpp"
#include "strhelp.h"
#include <fstream>
#include <regex>

using namespace std;
using namespace pd;

vector<string> mixt;
extern cfg_bool cfg_refresh;

namespace {
// Anonymous namespace : standard practice in fb2k components
// Nothing outside should have any reason to see these symbols,
// and we don't want funny results if another cpp has similarly named classes.
// service_factory at the bottom takes care of publishing our class.

// 5741202f - dcb1 - 4249 - 9723 - 977906bfc7aa
static const GUID guid_myelem =
{ 0x5741202f ,0xdcb1 ,0x4249 ,{ 0x97 ,0x23 ,0x97 ,0x79 ,0x6 ,0xbf ,0xc7 ,0xaa } };

static PdBase lpd;

static struct {
	int id;
	int lbl;
	int lblOut;
	string dest;
	BOOL isGradual;
	BOOL isReverse;
	float min ,max;
} hsl[] = {
	 { IDC_SLIDER1 ,IDC_LBL_SLIDER1 ,IDC_LBL_SL1OUT ,"" ,0 ,0 ,0 ,1000 }
	,{ IDC_SLIDER2 ,IDC_LBL_SLIDER2 ,IDC_LBL_SL2OUT ,"" ,0 ,0 ,0 ,1000 }
	,{ IDC_SLIDER3 ,IDC_LBL_SLIDER3 ,IDC_LBL_SL3OUT ,"" ,0 ,0 ,0 ,1000 }
	,{ IDC_SLIDER4 ,IDC_LBL_SLIDER4 ,IDC_LBL_SL4OUT ,"" ,0 ,0 ,0 ,1000 }
	,{ IDC_SLIDER5 ,IDC_LBL_SLIDER5 ,IDC_LBL_SL5OUT ,"" ,0 ,0 ,0 ,1000 }
	,{ IDC_SLIDER6 ,IDC_LBL_SLIDER6 ,IDC_LBL_SL6OUT ,"" ,0 ,0 ,0 ,1000 }
	,{ IDC_SLIDER7 ,IDC_LBL_SLIDER7 ,IDC_LBL_SL7OUT ,"" ,0 ,0 ,0 ,1000 }
};

static struct {
	int id;
	string dest;
	BOOL state;
} tgl[] = {
	 { IDC_TOGGLE1 ,"" ,false }
	,{ IDC_TOGGLE2 ,"" ,false }
	,{ IDC_TOGGLE3 ,"" ,false }
};

static struct {
	int id;
	string dest;
} bng[] = {
	 { IDC_BTN_BANG1 ,"" }
	,{ IDC_BTN_BANG2 ,"" }
	,{ IDC_BTN_BANG3 ,"" }
};

static struct {
	int id;
	int ed;
	string dest;
} msg[] = {
	 { IDC_BTN_MSG1 ,IDC_EDT_MSG1    ,"" }
	,{ IDC_BTN_MSG2 ,IDC_EDT_MSG2    ,"" }
	,{ IDC_BTN_ANY  ,IDC_EDT_ANY_MSG ,"" }
};

bool is_number(const string &s) {
	return !s.empty() && s.find_first_not_of("0123456789.-") == std::string::npos;
}

template <typename T>
string to_stringp(const T a_value ,const int n = 5) {
	ostringstream out;
	out.precision(n);
	out << a_value;
	return out.str();
}

class CDialogUIElem : public CDialogImpl<CDialogUIElem>
,public ui_element_instance ,private play_callback_impl_base {
public:
	CDialogUIElem( ui_element_config::ptr cfg ,ui_element_instance_callback::ptr cb )
	: m_callback(cb) ,m_flags( parseConfig(cfg) ) {}

	enum { IDD = IDD_UI_ELEMENT };

	BEGIN_MSG_MAP_EX( CDialogUIElem )
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_CODE_HANDLER_EX(BN_CLICKED ,OnButtonClicked)
		MSG_WM_HSCROLL(OnHScroll)
	END_MSG_MAP()

	void initialize_window(HWND parent) {
		WIN32_OP(Create(parent) != NULL);
	}

	HWND get_wnd() {
		return m_hWnd;
	}

	void set_configuration(ui_element_config::ptr config) {
		m_flags = parseConfig(config);
		if (m_hWnd != NULL)
			configToUI();
	}

	ui_element_config::ptr get_configuration() {
		return makeConfig(m_flags);
	}

	static GUID g_get_guid() {
		return guid_myelem;
	}

	static void g_get_name(pfc::string_base &out) {
		out = "Pd Mixer";
	}

	static ui_element_config::ptr g_get_default_configuration() {
		return makeConfig( );
	}

	static const char *g_get_description() {
		return "Interact with patch settings";
	}

	static GUID g_get_subclass() {
		return ui_element_subclass_utility;
	}

private:
	void on_playback_new_track(metadb_handle_ptr p_track) {
		string path = p_track->get_path();
		path.erase(0 ,7);
		mixt.clear();
		string line;
		ifstream in(path);
		while (getline(in ,line))
			mixt.push_back(line);
		if (!cfg_refresh) pd_mix();
	}

	static uint32_t parseConfig( ui_element_config::ptr cfg ) {
		try
		{	::ui_element_config_parser in ( cfg );
			uint32_t flags; in >> flags;
			return flags;   }
		catch (exception_io_data)
		{	return 0;   }
	}

	static ui_element_config::ptr makeConfig(uint32_t flags = 0) {
		ui_element_config_builder out;
		out << flags;
		return out.finish( g_get_guid() );
	}

	void OnButtonClicked(UINT uNotifyCode ,int nID ,CWindow wndCtl) {
		int i;
		for (i=0; i < PFC_TABSIZE(tgl); ++i)
			if (tgl[i].id == nID)
			{	tgl[i].state = !tgl[i].state;
				lpd << Float(tgl[i].dest ,tgl[i].state);
				return;   }
		for (i=0; i < PFC_TABSIZE(bng); ++i)
			if (bng[i].id == nID)
			{	lpd << Bang(bng[i].dest);
				return;   }
		for (i=0; i < PFC_TABSIZE(msg); ++i)
			if (msg[i].id == nID)
			{	if (nID == IDC_BTN_ANY)
					msg[i].dest = uGetDlgItemText(*this ,IDC_EDT_ANY_DEST).c_str();
				string result = uGetDlgItemText(*this ,msg[i].ed).c_str();
				result = ReplaceAll(result ,", " ,",");
				vector<string> cmds = split(result ,',');
				for (string cmd : cmds)
				{	vector<string> list = split(cmd ,' ');
					List send;
					for (int j=0; j < list.size(); ++j)
					{	if (is_number(list[j]))
							send.addFloat(stof(list[j]));
						else if (j>0) send.addSymbol(list[j]);   }
					if (list.size() > 0)
					{	if (is_number(list[0]))
							lpd.sendList(msg[i].dest ,send);
						else lpd.sendMessage(msg[i].dest ,list[0] ,send);   }   }
				return;   }
		if (nID == IDC_BTN_REFRESH) pd_mix();
	}

	void OnHScroll(UINT nSBCode ,UINT nPos ,CScrollBar pScrollBar) {
		for (int i=0; i < PFC_TABSIZE(hsl); ++i)
			if (pScrollBar.m_hWnd == m_slider[i].m_hWnd)
			{	float val = m_slider[i].GetPos();
				float min=hsl[i].min ,max=hsl[i].max;
				if      (hsl[i].isGradual)
					val = val * ((max - min) / 1000.f) + min;
				else if (hsl[i].isReverse)
					val = (min - max) - val + max;
				uSetDlgItemText(*this ,hsl[i].lblOut ,to_stringp(val).c_str());
				lpd << Float(hsl[i].dest ,val);   }
	}

	void pd_mix() {
		int s=0 ,t=0 ,b=0 ,m=0;
		isAny = false;
		smatch match;
		regex canvas("^#N canvas \\d+ \\d+ \\d+ \\d+ (mix|fb2k) \\d+;$");
		for (int i=0; i < PFC_TABSIZE(hsl); ++i)
			m_slider[i].SetPos(0);
// begin mixt for-loop
for (int i=0; i < mixt.size(); ++i)
{	if (regex_search(mixt[i] ,match ,canvas))
	{	start = i+1;
		regex slider("^#X obj \\d+ \\d+ (?:h|v)sl \\d+ \\d+ ([\\d\\.-]+) ([\\d\\.-]+)"
		   " \\d+ \\d+ ([\\w\\d\\.-]+) ([\\w\\d\\.-]+) ((?:(?:\\\\ )*[\\w\\d\\.-])+)");
		regex obj("#X obj \\d+ \\d+ (bng|tgl) \\d+ (?:\\d+ \\d+ )?\\d+ ([\\w\\d\\.-]+)"
		   " [\\w\\d\\.-]+ ((?:(?:\\\\ )*['\\w\\d\\.:-])+)");
		regex restore("^#X restore \\d+ \\d+ pd "+match[1].str()+";$");
		for (; i < mixt.size(); ++i)
		{	if (regex_search(mixt[i] ,match ,slider) && s < PFC_TABSIZE(hsl))
			{	float min=stof(match[1]) ,max=stof(match[2]);
				hsl[s].min = min ,hsl[s].max = max;
				hsl[s].isReverse = (min>max);
				hsl[s].isGradual = (match[4] == "gradual");
				if (!hsl[s].isGradual)
				{	if (hsl[s].isReverse)
					     m_slider[s].SetRange(max ,min);
					else m_slider[s].SetRange(min ,max);   }
				else m_slider[s].SetRange(0 ,1000);
				hsl[s].dest = match[3];
				string label = ReplaceAll(match[5] ,"\\ " ," ");
				uSetDlgItemText(*this ,hsl[s].lbl    ,label.c_str());
				uSetDlgItemText(*this ,hsl[s].lblOut ,to_stringp(min).c_str());
				++s;   }

			else if (regex_search(mixt[i] ,match ,obj))
			{	if (match[1] == "bng")
				{	if (match[3] == "empty" && b < PFC_TABSIZE(bng))
					{	bng[b].dest = match[2];
						uSetDlgItemText(*this ,bng[b].id ,bng[b].dest.c_str());
						++b;   }
					else if (m < PFC_TABSIZE(msg))
					{	string edit = ReplaceAll(match[3] ,"\\ " ," ");
						edit = ReplaceAll(edit ,"'" ,",");
						vector<string> any = split(edit ,':');
						if (any.size() > 1)
						{	uSetDlgItemText(*this ,IDC_EDT_ANY_DEST ,any[0].c_str());
							uSetDlgItemText(*this ,IDC_EDT_ANY_MSG  ,any[1].c_str());
							isAny = true;   }
						else
						{	msg[m].dest = match[2];
							if (msg[m].id == IDC_BTN_ANY)
							{	uSetDlgItemText(*this ,IDC_EDT_ANY_DEST ,msg[m].dest.c_str());
								isAny = true;   }
							else uSetDlgItemText(*this ,msg[m].id ,msg[m].dest.c_str());
							uSetDlgItemText(*this ,msg[m].ed ,edit.c_str());
							++m;   }   }   }

				else if (t < PFC_TABSIZE(tgl))
				{	tgl[t].dest = match[2];
					string label = ReplaceAll(match[3] ,"\\ " ," ");
					uSetDlgItemText(*this ,tgl[t].id ,label.c_str());
					++t;   }   }

			else if (regex_search(mixt[i] ,match ,restore))
			{	end=i;
				Trim(s ,t ,b ,m);
				break;   }   }
		break;   }   }
// end mixt for-loop
		Trim(s ,t ,b ,m);
	}

	void Trim(int s ,int t ,int b ,int m) {

		for (; s < PFC_TABSIZE(hsl); ++s)
		{	uSetDlgItemText(*this ,hsl[s].lbl    ,"--");
			uSetDlgItemText(*this ,hsl[s].lblOut ,"-");   }
		for (; t < PFC_TABSIZE(tgl); ++t)
			uSetDlgItemText(*this ,tgl[t].id ,"--");
		for (; b < PFC_TABSIZE(bng); ++b)
			uSetDlgItemText(*this ,bng[b].id ,"--");
		for (; m < PFC_TABSIZE(msg)-1; ++m)
		{	uSetDlgItemText(*this ,msg[m].id ,"--");
			uSetDlgItemText(*this ,msg[m].ed ,"");   }
		if (!isAny)
		{	uSetDlgItemText(*this ,IDC_EDT_ANY_DEST ,"");
			uSetDlgItemText(*this ,IDC_EDT_ANY_MSG  ,"");   }
	}

	void configToUI() {
		for (int i=0; i < PFC_TABSIZE(hsl); ++i)
		{	m_slider[i] = GetDlgItem(hsl[i].id);
			m_slider[i].SetRange(0 ,1000);   }
	}

	BOOL OnInitDialog(CWindow ,LPARAM) {
		if (!lpd.isInited())
		{	lpd.init(0 ,2 ,44100 ,true);
			string fappdata = getenv("APPDATA");
			fappdata = ReplaceAll(fappdata ,"\\" ,"/");
			fappdata += "/foobar2000/user-components/foo_pd/extra";
			lpd.addToSearchPath(fappdata);
			lpd.computeAudio(true);   }

		configToUI();
		return FALSE;
	}

	const ui_element_instance_callback::ptr m_callback;
	uint32_t m_flags;
	CTrackBarCtrl m_slider[PFC_TABSIZE(hsl)];
	int start=0 ,end=0;
	BOOL isAny;
};

static service_factory_single_t< ui_element_impl< CDialogUIElem > > g_CDialogUIElem_factory;
} // end namespace
