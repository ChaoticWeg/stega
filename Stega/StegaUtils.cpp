//**************************************************************
// File:        StegaUtils.cpp
// Author:      Shawn Lutch
// Description: Implementation of StegaUtils.h
//**************************************************************

#include "StegaUtils.h"
#include <sstream>
#include <cctype>


//*******************************************************
// void hide(vector<unsigned int> &pixels, string text)
//
// Hides the string inside of the pixels by distributing
// each letter (four-byte unsigned int) across the least
// significant bits of the R, G, and B channels
//*******************************************************
void StegaUtils::hide(std::vector<unsigned int> &pixels, const std::string text)
{
	unsigned int i = 0;
	for (i = 0; i < text.length(); i++)
	{
		unsigned int pixel = pixels[i];		// the raw pixel (hex ARGB)
		unsigned char hexchar = text.at(i);	// the character to hide (ASCII)

		// mask out LSBs so the char can be easily piped in
		pixel &= 0xFFF0F0F0;

		// align bytes of character
		unsigned int modified_char =
			((hexchar & 0xF00) << 8)	// shift most significant bit 2 places left
			| ((hexchar & 0x0F0) << 4)	// shift second bit 1 place left
			| (hexchar & 0x00F);		// leave least significant bit where it is

		// hide the character inside the pixel
		pixel |= modified_char;

		// we have our "new" pixel
		pixels[i] = pixel;
	}	// TODO leave steps separated for readability?

	// mask out a null terminator in the following pixel to signal the end of the message
	pixels[i] &= 0xFFF0F0F0;
}

//*******************************************************
// void find(vector<unsigned int> &pixels, string &text)
//
// Searches for a string hidden inside the pixels and
// stores the result in the string passed by reference
//*******************************************************
void StegaUtils::find(std::vector<unsigned int> &pixels, std::string &text)
{
	// states
	bool foundNUL = false;

	unsigned int i = -1;	// index
	std::stringstream ss;	// easy string concatenation

	do
	{
		++i;

		// get the pixel
		unsigned int pixel = pixels[i];

		// extract the hidden char
		unsigned int hiddenChar =
			((pixel & 0x000F0000) >> 8)		// shift most significant bit two places to right
			| ((pixel & 0x00000F00) >> 4)	// shift second bit one place to right
			| (pixel & 0x00000F);			// leave least significant bit where it is

		// is it a null terminator?
		if (hiddenChar == 0x000)
		{
			// don't add the null terminator to the result.
			// just end the loop without doing anything
			foundNUL = true;
		}

		// otherwise, assume it's part of the message
		else
		{
			// add this character to the stringstream if it's printable
			if (isprint(hiddenChar))
				ss << ((char)hiddenChar);
		}

		// stop if we find a null terminator or garbage, or if we've looked at all pixels
	} while (!foundNUL && (i < (pixels.size() - 1)));

	// assign the contents of the stringstream to the output parameter
	text = ss.str();
}