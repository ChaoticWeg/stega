//**************************************************************
// File:        StegaUtils.cpp
// Author:      Shawn Lutch
// Description: Implementation of StegaUtils.h
//**************************************************************

#include "StegaUtils.h"
#include <sstream>

// the big ones

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
		unsigned int pixel = pixels[i];
		unsigned char hexchar = text.at(i);

		// prepare pixel
		pixel &= 0xFFF0F0F0;

		// prepare character
		unsigned int modified_char =
			// mask out and shift bits into place
			((hexchar & 0xF00) << 8) | ((hexchar & 0x0F0) << 4) | (hexchar & 0x00F);

		// hide the separated character inside the RGB channels
		pixel |= modified_char;

		// we have our "new" pixel
		pixels[i] = pixel;
	}

	i = text.length();

	// mask out a null terminator in the following pixel, so that we will know where to stop when finding
	// TODO URGENT FIND A BETTER WAY TO SIGNAL THE END OF THE MESSAGE
	pixels[i] &= 0xFFF0F0F0;
}

//*******************************************************
// void find(vector<unsigned int> &pixels, string &text)
//
// Searches for a string hidden inside the pixels
//*******************************************************
void StegaUtils::find(std::vector<unsigned int> &pixels, std::string &text)
{
	bool foundNUL = false;
	unsigned int i = -1;
	std::stringstream ss;

	do
	{
		i++;

		// get the pixel
		unsigned int pixel = pixels[i];

		// extract the hidden char
		unsigned int hiddenChar =
			((pixel & 0x000F0000) >> 8)		// isolate first bit and shift two places to right
			| ((pixel & 0x00000F00) >> 4)	// isolate second bit and shift one place to right
			| (pixel & 0x00000F);			// isolate third bit

		// is it a null terminator?
		if (hiddenChar == 0x000)
		{
			// don't add the null terminator to the result.
			// just end the loop without doing anything
			foundNUL = true;
		}
		else
		{
			// add this character to the stringstream
			ss << ((char)hiddenChar);
		}
	} while (!foundNUL && i < (pixels.size() - 1));

	text = ss.str();
}