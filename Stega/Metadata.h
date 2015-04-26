#ifndef FILE_METADATA_H
#define FILE_METADATA_H

struct Metadata
{
public:
	unsigned int
		filesize,		// the size of the file, including headers and pixels, in bytes
		sizeHeaders,	// the size of the headers, in bytes
		sizeDIB,		// the size of the DIB header, in bytes
		sizeBMP,		// the size of the BMP header, in bytes
		pixelCount;		// the number of pixels in the file

	// headers
	char* headers;		// raw header bytes
};

#endif