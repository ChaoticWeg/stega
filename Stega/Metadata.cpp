#include "Metadata.h"

// destructor
Metadata::~Metadata()
{
	delete[] headers;
	headers = NULL;
}


//********************************************************
// Metadata* Metadata::create(ifstream &infile)
//
// Reads in the relevant file metadata (filesize, header
// size, pixel count, etc.) from the ifstream. Returns a
// `Metadata` object containing all of the relevant data,
// as read from the ifstream.
//********************************************************

Metadata* Metadata::create(std::ifstream &infile)
{
	Metadata *result = new Metadata();

	// read the filesize
	infile.seekg(2L, std::ios::beg);
	infile.read(reinterpret_cast<char*>(&(result->filesize)), sizeof(unsigned int));

	// read the headers size (i.e. pixel offset)
	infile.seekg(10L, std::ios::beg);
	infile.read(reinterpret_cast<char*>(&(result->sizeHeaders)), sizeof(unsigned int));
	result->headers = new char[result->sizeHeaders];

	// read the DIB header size
	infile.seekg(14L, std::ios::beg);
	infile.read(reinterpret_cast<char*>(&(result->sizeDIB)), sizeof(unsigned int));

	// calculate BMP header size
	result->sizeBMP = result->sizeHeaders - result->sizeDIB;

	// calculate pixel count
	result->pixelCount = (result->filesize - result->sizeHeaders) / sizeof(unsigned int);

	// read the header data
	infile.seekg(0L, std::ios::beg);
	infile.read(result->headers, result->sizeHeaders);

	return result;
}