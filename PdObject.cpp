#include "stdafx.h"
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
#include "PdObject.hpp"

#include <iostream>

using namespace std;
using namespace pd;

//--------------------------------------------------------------
void PdObject::print(const std::string &message) {
	console::print(message.c_str());
}

//--------------------------------------------------------------		
void PdObject::receiveBang(const std::string &dest) {
	if (dest == "stop") stop = true;
}

void PdObject::receiveFloat(const std::string &dest ,float num) {
	console::printf("CPP: float %s: %g" ,dest ,num);
}

void PdObject::receiveSymbol(const std::string &dest ,const std::string &symbol) {
	console::printf("CPP: symbol %s: %s" ,dest ,symbol);
}

void PdObject::receiveList(const std::string &dest ,const List &list) {
	console::printf("CPP: list %s: %s%s" ,dest ,list.toString() ,list.types());
}

void PdObject::receiveMessage
(const std::string &dest ,const std::string &msg ,const List &list) {
	console::printf("CPP: message %s: %s %s%s" ,dest ,msg ,list.toString() ,list.types());
}
