#ifndef STEGA_UTILS_H
#define STEGA_UTILS_H

#include <vector>
#include <string>

class StegaUtils
{
public:
	
	// encoding
	static unsigned int* splitChannels(const unsigned int pixel);
	static unsigned int* separateCharacterBytes(const unsigned char c);
	static unsigned int* maskChannels(const unsigned int *channels);
	static void hide(std::vector<unsigned int> &pixels, const std::string text);

	// decoding
	static void findCharacterInPixel(const unsigned int pixel, char &result);
	static void find(std::vector<unsigned int> &pixels, std::string &text);
};

#endif