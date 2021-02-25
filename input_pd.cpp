#include "stdafx.h"

#include "PdBase.hpp"
#include "PdObject.hpp"
#include "strhelp.h"
#include <fstream>
#include <regex>
#include <iomanip>

using namespace std;
using namespace pd;

enum {
	pd_nch = 2,		// # of channels
	pd_bips = 16,	// bits per sample
	pd_hz = 44100,	// sample rate
	pd_blks = 64,	// libpd.blockSize()
	pd_byps = pd_bips / 8, // bytes per sample
	pd_width = pd_byps * pd_nch, // total sample width
	pd_read = pd_width*2 * pd_blks*2, // deltaread
};

class PdFoo : public pd::PdBase
{ public: Patch play; };

static PdFoo lpd;
static PdObject obj;

static bool wrote = false;
static bool iInited = false;

class input_pd : public input_stubs {
public:
	void open(service_ptr_t<file> p_filehint, const char * p_path,
	 t_input_open_reason p_reason, abort_callback & p_abort) {
		m_file = p_filehint;
		if (p_reason < 2)
			input_open_file_helper(m_file, p_path, p_reason, p_abort);
		else input_open_file_helper(m_file, p_path, input_open_info_read, p_abort);

		string all = p_path;
		string name = all.substr(all.find_last_of("\\") + 1);
		string path = all.substr(7, all.find_last_of("\\") - 7);
		patch = Patch(name, path);
		pd_info();

		if (p_reason == input_open_decode && wrote)
		{	patch = lpd.play;
			wrote = false;   }
		else if (p_reason == input_open_info_write)
		{	m_file = p_filehint;
			input_open_file_helper(m_file, p_path, p_reason, p_abort);   }
	}

	void get_info(file_info & p_info, abort_callback & p_abort) {
		t_filesize size = m_file->get_size(p_abort);
		if (size != filesize_invalid) p_info = info;
		p_info.info_set_int("samplerate", pd_hz);
		p_info.info_set_int("channels", pd_nch);
		p_info.info_set_int("bitspersample", pd_bips);
		p_info.info_set("encoding", "lossless");
		p_info.info_set_bitrate((pd_bips * pd_nch * pd_hz + 500) / 1000);
	}

	void pd_info() {
		text.clear();
		string line;
		ifstream in(patch.path() + "\\" + patch.filename());
		while (getline(in, line)) text.push_back(line);
		if (!pd_update())
		{	smatch match;
			regex canvas = regex("^#N canvas \\d+ \\d+ \\d+ \\d+ \\d+;$");
			if (regex_search(text[0], match, canvas))
			{	regex connect("^#X connect \\d+ \\d+ \\d+ \\d+;$");
				for (int i = text.size()-1; i>=0; --i)
				{	if (regex_search(text[i], match, connect)) continue;
					text.insert(text.begin()+i+1, "#X restore 20 20 pd meta;");
					text.insert(text.begin()+i+1, "#N canvas 0 0 450 300 meta 0;");
					break;   }   }
			pd_update();   }
	}

	bool pd_update() {
		bool update = false;
		smatch match;
		regex canvas("^#N canvas \\d+ \\d+ \\d+ \\d+ (meta|info) \\d+;$");
		for (int i=0; i < text.size(); ++i)
		{	if (regex_search(text[i], match, canvas))
			{	start = i+1;
				regex meta("^#X text \\d+ \\d+ ((?: *[\\w])+) *: *(.+);$");
				regex restore("^#X restore \\d+ \\d+ pd "+match[1].str()+";$");
				for (; i < text.size(); ++i)
				{	if (regex_search(text[i], match, meta))
					{	if (match[1].str() == "length")
						{	vector<string> clock = split(match[2].str(), ':');
							if (clock.size() > 1)
							{	int mins = stoi(clock[0]);
								double secs = stod(clock[1]);
								info.set_length(60*mins + secs);   }
							else info.set_length(stod(match[2].str()));   }
						else
						{	string value = match[2].str();
							value = ReplaceAll(value, " \\,", ",");
							value = ReplaceAll(value, " \\;", ";");
							info.meta_set(match[1].str().c_str(),
										  match[2].str().c_str());   }   }
					else if (regex_search(text[i], match, restore))
					{	end=i; break;   }   }
				update = true;
				break;   }   }
		return update;
	}

	void retag(const file_info & p_info, abort_callback & p_abort) {
		info = p_info;
		vector<string> coms;
		int x = 175, y = 60;
		for (int i=0; i < info.meta_get_count(); y+=20, ++i)
		{	string field = info.meta_enum_name(i);
			string value = "";
			transform(field.begin(), field.end(), field.begin(), ::tolower);
			for (int j=0; j<info.meta_enum_value_count(i); ++j)
			{	if (j>0) value += "; ";
				value += info.meta_enum_value(i, j);   }
			value = ReplaceAll(value, "\\", "/");
			value = ReplaceAll(value, ",", " \\,");
			value = ReplaceAll(value, ";", " \\;");
			ostringstream s;
			s << "#X text " << x << " " << y << " " << field << " : " << value << ";";
			coms.push_back(s.str());   }
		ostringstream s;
		s.precision(5);
		int mins = info.get_length() / 60;
		double secs = info.get_length() - (60*mins);
		s << "#X text " << x << " " << y << " length : " << mins << ":"
			<< setfill('0') << setw((secs == (int)secs) ? 2 : 6) << secs << ";";
		coms.push_back(s.str());

		text.erase(text.begin() + start, text.begin() + end);
		text.insert(text.begin()+start, coms.begin(), coms.end());

		for (string newtext : text)
			m_file->write_string_raw((newtext+"\n").c_str(), p_abort);
		m_file->set_eof(p_abort);
		wrote = true;
	}

	t_filestats get_file_stats(abort_callback & p_abort) {
		auto stats = m_file->get_stats(p_abort);
		return stats;
	}

	void decode_initialize(unsigned p_flags, abort_callback & p_abort) {
		//equivalent to seek to zero, except it also works on nonseekable streams
		m_file->reopen(p_abort);
		
		if (!lpd.isInited())
		{	lpd.init(0, pd_nch, pd_hz, true);
			string fappdata = getenv("APPDATA");
			fappdata = ReplaceAll(fappdata, "\\", "/");
			fappdata += "/foobar2000/user-components/foo_pd/extra";
			lpd.addToSearchPath(fappdata);
			lpd.setReceiver(&obj);
			lpd.subscribe("stop");
			lpd.computeAudio(true);   }
		else if (!iInited) 
		{	lpd.setReceiver(&obj);
			lpd.subscribe("stop");   }
		iInited = true;
	}

	bool decode_run(audio_chunk & p_chunk, abort_callback & p_abort) {
		m_buffer.set_size(pd_read*pd_width);

		if (!patch.isValid())
		{	lpd.closePatch(lpd.play);
			patch = lpd.openPatch(patch);
			lpd.play = patch;
			lpd << Float("vol", 1);
			lpd << Bang("play");
			obj.stop = false;   }
		
		lpd.receiveMessages();
		if (obj.stop)
		{	lpd.closePatch(lpd.play);
			return false;   }

		lpd.processShort(pd_bips, 0, m_buffer.get_ptr());
		p_chunk.set_data_fixedpoint
		(	m_buffer.get_ptr(), pd_read*pd_width, pd_hz, pd_nch, pd_bips,
			audio_chunk::g_guess_channel_config(pd_nch)   );

		//processed successfully, no EOF
		return true;
	}

	void decode_seek(double p_seconds, abort_callback & p_abort) {
		m_file->ensure_seekable(); // throw exceptions if nonseekable.
		lpd << Float("pos", p_seconds);
	}

	bool decode_can_seek() {
		return m_file->can_seek();
	}
	
	bool decode_get_dynamic_info(file_info & p_out, double & p_timestamp_delta) {
		return false; // deals with dynamic information such as VBR bitrates
	}
	
	bool decode_get_dynamic_info_track(file_info & p_out, double & p_timestamp_delta) {
		return false; // deals with dynamic information such as track changes in live streams
	}

	void decode_on_idle(abort_callback & p_abort) {
		m_file->on_idle(p_abort);
	}

	static bool g_is_our_content_type(const char * p_content_type) {
		return false; // match against supported mime types here
	}

	static bool g_is_our_path(const char * p_path, const char * p_extension) {
		return stricmp_utf8(p_extension, "pd") == 0;
	}

	static const char * g_get_name() {
		return "Pd Player";
	}

	static const GUID g_get_guid() {
		// 4e0ba29e - fe4f - 42ba - 89fc - 89db2ad7d355
		static const GUID guid_pd_decode =
		{ 0x4e0ba29e, 0xfe4f, 0x42ba,{ 0x89, 0xfc, 0x89, 0xdb, 0x2a, 0xd7, 0xd3, 0x55 } };
		return guid_pd_decode;
	}
public:
	service_ptr_t<file> m_file;
	pfc::array_t<short> m_buffer;
	Patch patch;
	file_info_impl info;
	vector<string> text;
	int start=0, end=0;
};

static input_singletrack_factory_t<input_pd> g_input_pd_factory;

DECLARE_FILE_TYPE("Pure Data", "*.PD");
