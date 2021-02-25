#pragma once
#include "PdObject.hpp"
#include "samplerate.h"
#include "strhelp.h"

enum {
	 pd_nch   = 2     // # of channels
	,pd_bips  = 16    // bits per sample
	,pd_blks  = 64    // libpd.blockSize()
	,pd_byps  = pd_bips / 8 // bytes per sample
	,pd_width = pd_byps * pd_nch // total sample width
	,pd_read  = pd_width * 2 * pd_blks * 2 // deltaread
};

static PdObject obj;

class PdFoo : public pd::PdBase {
public:
	pd::Patch play; // currently playing track
	int srate;      // sample rate

	void init() {
		if (isInited()) return;

		srate = getSampleRate();
		if (srate < 0)
			srate = 44100;
		PdBase::init(0 ,pd_nch ,srate ,true);
		if (core_api::is_portable_mode_enabled)
			addToSearchPath("profile/user-components/foo_pd/extra");
		else
		{	std::string path = getenv("APPDATA");
			path = ReplaceAll(path ,"\\" ,"/");
			path += "/foobar2000/user-components/foo_pd/extra";
			addToSearchPath(path);   }
		setReceiver(&obj);
		subscribe("stop");
		computeAudio(true);
	}
};
