#include "stdafx.h"
#include "strhelp.h"
#include <fstream>
#include <vector>

using namespace std;

// ed621f5d - ab14 - 4c63 - b34b - 10b3e1ea080f
static const GUID guid_mygroup =
{ 0xed621f5d, 0xab14, 0x4c63, { 0xb3, 0x4b, 0x10, 0xb3, 0xe1, 0xea, 0x08, 0x0f } };


// Switch to contextmenu_group_factory to embed your commands in the root menu
// but separated from other commands.

//static contextmenu_group_factory g_mygroup(guid_mygroup, contextmenu_groups::root, 0);
static contextmenu_group_popup_factory g_mygroup(guid_mygroup,
 contextmenu_groups::root, "Pd Player", 0);

static void RunTestCommand(metadb_handle_list_cref data);
void RunCalculatePeak(metadb_handle_list_cref data); //decode.cpp

// Simple context menu item class.
class myitem : public contextmenu_item_simple {
public:
	enum {
		cmd_test1 = 0,
		cmd_total
	};

	GUID get_parent() {
		return guid_mygroup;
	}

	unsigned get_num_items() {
		return cmd_total;
	}

	void get_item_name(unsigned p_index,pfc::string_base & p_out) {
		switch(p_index)
		{	case cmd_test1: p_out = "Load mixer"; break;
			default: uBugCheck();   }
	}

	void context_command(unsigned p_index,metadb_handle_list_cref p_data,const GUID& p_caller) {
		switch(p_index)
		{	case cmd_test1:
				RunTestCommand(p_data);
				break;
			default: uBugCheck();   }
	}

	GUID get_item_guid(unsigned p_index) {
		// cc81302d - 82f9 - 464c - 8920 - dcb6b6b3b68b
		static const GUID guid_test1 =
		{ 0xcc81302d, 0x82f9, 0x464c, { 0x89, 0x20, 0xdc, 0xb6, 0xb6, 0xb3, 0xb6, 0x8b } };
		
		switch(p_index)
		{	case cmd_test1: return guid_test1;
			default: uBugCheck();   }
	}

	bool get_item_description(unsigned p_index,pfc::string_base & p_out) {
		switch(p_index)
		{	case cmd_test1:
				p_out = "Reads a patch's mixer to be loaded on refresh";
				return true;
			
			// should never happen unless somebody called us with invalid parameters - bail
			default: uBugCheck();   }
	}
};

static contextmenu_item_factory_t<myitem> g_myitem_factory;

extern vector<string> mixt;

static void RunTestCommand(metadb_handle_list_cref data) {
	pfc::string_formatter message;
	if (data.get_count() > 0)
	{	message << data[0];
		string path = message.c_str();
		path = ReplaceAll(path, "\"", "");
		string ext = path.substr(path.find_last_of(".") + 1);
		if (ext == "pd")
		{	mixt.clear();
			string line;
			ifstream in(path);
			while (getline(in, line)) mixt.push_back(line);   }   }
}
