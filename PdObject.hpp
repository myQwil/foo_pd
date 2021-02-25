/*
 * Copyright (c) 2012 Dan Wilcox <danomatika@gmail.com>
 *
 * BSD Simplified License.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * See https://github.com/libpd/libpd for documentation
 *
 * This file was adapted from the ofxPd openFrameworks addon example:
 * https://github.com/danomatika/ofxPd
 *
 */
#pragma once

#include "PdBase.hpp"

// custom receiver class
class PdObject : public pd::PdReceiver {
public:
	void print         (const std::string &message);
	void receiveBang   (const std::string &dest);
	void receiveFloat  (const std::string &dest ,float num);
	void receiveSymbol (const std::string &dest ,const std::string &symbol);
	void receiveList   (const std::string &dest ,const pd::List &list);
	void receiveMessage
	(const std::string &dest ,const std::string &msg ,const pd::List &list);

	bool stop = false;
};
