//**************************************************************
// File:        StegaUtils.h
// Author:      Shawn Lutch
// Description: Static member functions for hiding characters
//              inside of pixels
//**************************************************************

#ifndef STEGA_UTILS_H
#define STEGA_UTILS_H

#include <vector>
#include <string>

class StegaUtils final
{
public:
	
	// encoding
	static void hide(std::vector<unsigned int> &pixels, const std::string text);

	// decoding
	static void find(std::vector<unsigned int> &pixels, std::string &text);
};

#endif