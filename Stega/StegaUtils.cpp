#include "StegaUtils.h"
#include <iostream>
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
	// arrays of unsigned ints that will be used with each pixel
	unsigned int
		*channels = NULL,
		*letterBytes = NULL,
		*maskedChannels = NULL,
		*newBytes = NULL;

	unsigned int i = 0;
	for (i = 0; i < text.length(); i++)
	{
		unsigned int pixel = pixels.at(i);
		unsigned char c = text.at(i);

		// split pixel into channels (r, g, b, a)
		channels = StegaUtils::splitChannels(pixel);

		// separate the three bytes of the character (i.e. 0x06f)
		letterBytes = StegaUtils::separateCharacterBytes(c);

		// mask the bytes to prepare to pipe in the letter bytes
		maskedChannels = StegaUtils::maskChannels(channels);

		// hide the separated character inside the RGB channels
		newBytes = new unsigned int[4];
		newBytes[0] = maskedChannels[0];
		newBytes[1] = maskedChannels[1] | letterBytes[0];
		newBytes[2] = maskedChannels[2] | letterBytes[1];
		newBytes[3] = maskedChannels[3] | letterBytes[2];

		// bring all channels back together
		unsigned int NEW_BYTE = newBytes[0] | newBytes[1] | newBytes[2] | newBytes[3];

		// we have our "new" pixel
		pixels[i] = NEW_BYTE;
	}

	i = text.length();

	// mask out a null terminator in the following pixel, so that we will know where to stop when finding
	// TODO URGENT FIND A BETTER WAY TO SIGNAL THE END OF THE MESSAGE
	pixels[i] &= 0xFFF0F0F0;

	delete[] channels;
	delete[] letterBytes;
	delete[] maskedChannels;
	delete[] newBytes;
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

		// find the hidden char
		char hiddenChar;
		StegaUtils::findCharacterInPixel(pixel, hiddenChar);

		// is it a null terminator?
		if (((int)hiddenChar) == 0x000)
		{
			// don't add the null terminator to the result.
			// just end the loop without doing anything
			foundNUL = true;
		}
		else
		{
			// add this character to the stringstream
			ss << hiddenChar;
		}
	} while (!foundNUL && i < (pixels.size() - 1));

	text = ss.str();
}



// find the character hidden in the pixel, return by reference
void StegaUtils::findCharacterInPixel(const unsigned int pixel, char &result)
{
	// ignore alpha channel (channels[0]). it's irrelevant
	unsigned int *channels = StegaUtils::splitChannels(pixel);

	// mask the channels to get each LSB
	unsigned int
		maskedR = channels[1] & 0x000F0000,
		maskedG = channels[2] & 0x00000F00,
		maskedB = channels[3] & 0x0000000F;

	// shift the LSBs to form the character
	maskedR = maskedR >> 8;
	maskedG = maskedG >> 4;

	unsigned int finalInt = maskedR | maskedG | maskedB;
	result = (char)finalInt;

	delete[] channels;
}

// split the pixel into its four channels (A, R, G, B)
unsigned int* StegaUtils::splitChannels(const unsigned int pixel)
{
	unsigned int *bytes = new unsigned int[4];

	bytes[0] = pixel & 0xFF000000;
	bytes[1] = pixel & 0x00FF0000;
	bytes[2] = pixel & 0x0000FF00;
	bytes[3] = pixel & 0x000000FF;

	return bytes;
}

// separate the character into its three bytes and shift them
// into the correct place to be inserted into the pixel
unsigned int* StegaUtils::separateCharacterBytes(const unsigned char c)
{
	unsigned int *letterBytes = new unsigned int[3];

	letterBytes[0] = ((int)c << 8) & 0x00FF0000;
	letterBytes[1] = ((int)c << 4) & 0x0000FF00;
	letterBytes[2] = ((int)c) & 0x0000000F;

	return letterBytes;
}

// mask the pixel's channel bytes to allow for the
// character's bits to be inserted
unsigned int* StegaUtils::maskChannels(const unsigned int *channels)
{
	unsigned int *maskedChannels = new unsigned int[4];

	maskedChannels[0] = channels[0];
	maskedChannels[1] = channels[1] & 0xFFF0FFFF;
	maskedChannels[2] = channels[2] & 0xFFFFF0FF;
	maskedChannels[3] = channels[3] & 0xFFFFFFF0;

	return maskedChannels;
}