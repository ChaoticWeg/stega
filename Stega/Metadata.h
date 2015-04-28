//**************************************************************
// File:        Metadata.h
// Author:      Shawn Lutch
// Description: Object to hold data about the bitmap image file
//**************************************************************

#ifndef FILE_METADATA_H
#define FILE_METADATA_H

#include <fstream>

class Metadata
{
public:
	unsigned int
		filesize,		// the size of the file, including headers and pixels, in bytes
		sizeHeaders,	// the size of the headers, in bytes
		sizeDIB,		// the size of the DIB header, in bytes
		sizeBMP,		// the size of the BMP header, in bytes
		pixelCount;		// the number of pixels in the file

	char* headers;		// raw header bytes
	// we don't concern ourselves with this data much after populating the above values,
	// other than to copy it to the new file verbatim after modifying the pixels.

	// creation/destruction
	static Metadata* create(std::ifstream &infile);
	~Metadata();

private:
	// no constructors allowed. instead only use static builder method
	Metadata(){};
};

#endif