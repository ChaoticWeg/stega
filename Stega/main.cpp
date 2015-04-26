//**************************************************************
// File:		main.cpp
// Author:		Shawn Lutch
// Description: Executing class
//**************************************************************

// boost
#include <boost/algorithm/string/predicate.hpp>

// c++ stdlib
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <assert.h>

// project
#include "CmdParser.h"
#include "ExitCodes.h"
#include "Metadata.h"

using namespace std;

Metadata* readMetadata(ifstream &infile);
void readStringFromFile(ifstream &file, string &result);
void readPixelsFromFile(ifstream &file, vector<unsigned int> &pixels,
	unsigned int startPos, unsigned int numPixels);

int encode(string carrierFile, string secretFile);
int decode(string carrierFile);

void hide(vector<unsigned int> &pixels, string text);
void find(vector<unsigned int> &pixels, string &text);



//**************************************************************
//  COMMAND LINE USAGE
//     [0]         [1]             [2]               [3]
// Stega.exe [encode|decode] <CarrierFile.bmp> <SecretFile.txt>
//**************************************************************

int main(int argc, char** argv)
{
	int exitCode = 0;
	string SECRET_TEXT, CARRIER_NAME;
	CmdParser cla(argc, argv);

	switch (cla.getMode())
	{
		case INVALID:
			cerr << "ERROR - Invalid arguments given.\n";
		case HELP:
			cerr << "Usage:"
				<< "\n\t" << argv[0] << " encode <CarrierFile.bmp> <SecretFile.txt>"
				<< "\n\t" << argv[0] << " decode <InputFile.bmp>\n";
			return (int)EXIT_CODES::INVALID_CLA;

		case ENCODE:
			CARRIER_NAME = cla.getCarrierFile();
			SECRET_TEXT = cla.getSecretFile();
			return encode(CARRIER_NAME, SECRET_TEXT);

		case DECODE:
			CARRIER_NAME = cla.getCarrierFile();
			return decode(CARRIER_NAME);
	}

}


//*********************************************************
// void readStringFromFile(ifstream &file, string &result)
// 
// Reads a string in as text from the ifstream and assigns
// its value to the string passed in by reference as
// 'result'
//*********************************************************

void readStringFromFile(ifstream &file, string &result)
{
	stringstream ss;
	file.seekg(0L, ios::beg);

	char c;
	while (file.get(c))
		ss << c;

	result = ss.str();
}


//***********************************************************************
// void readPixelsFromFile(ifstream &file, vector<unsigned int> &pixels,
//		unsigned int startPos, unsigned int numPixels)
//
// Reads 'numPixels' pixels from the ifstream 'file',
// starting 'startPos' bytes from the beginning of the
// file, and stores each pixel in the 'pixels' vector.
//***********************************************************************

void readPixelsFromFile(ifstream &file, vector<unsigned int> &pixels,
	unsigned int startPos, unsigned int numPixels)
{
	file.seekg(startPos, ios::beg);
	for (unsigned int i = 0; i < numPixels; i++)
	{
		unsigned int pixel;
		file.read(reinterpret_cast<char*>(&pixel), sizeof(unsigned int));
		pixels.push_back(pixel);
	}
}


//********************************************************
// Metadata* readMetadata(ifstream &infile)
//
// Reads in the relevant file metadata (filesize, header
// size, pixel count, etc.) from the ifstream. Returns a
// `Metadata` struct containing all of the relevant data,
// as read from the ifstream.
//********************************************************

Metadata* readMetadata(ifstream &infile)
{
	Metadata *result = new Metadata;

	// read the filesize
	infile.seekg(2L, ios::beg);
	infile.read(reinterpret_cast<char*>(&(result->filesize)), sizeof(unsigned int));

	// read the headers size (i.e. pixel offset)
	infile.seekg(10L, ios::beg);
	infile.read(reinterpret_cast<char*>(&(result->sizeHeaders)), sizeof(unsigned int));
	result->headers = new char[result->sizeHeaders];

	// read the DIB header size
	infile.seekg(14L, ios::beg);
	infile.read(reinterpret_cast<char*>(&(result->sizeDIB)), sizeof(unsigned int));

	// calculate BMP header size
	result->sizeBMP = result->sizeHeaders - result->sizeDIB;

	// calculate pixel count
	result->pixelCount = (result->filesize - result->sizeHeaders) / sizeof(unsigned int);

	// read the header data
	infile.seekg(0L, ios::beg);
	infile.read(result->headers, result->sizeHeaders);

	return result;
}


//*******************************************************
// void hide(vector<unsigned int> &pixels, string text)
//
// Hides the string inside of the pixels by distributing
// each letter (four-byte unsigned int) across the least
// significant bits of the R, G, and B channels
//*******************************************************

// steps to be taken in the process of hiding

unsigned int* splitChannels(const unsigned int pixel);
unsigned int* separateCharacterBytes(const unsigned char c);
unsigned int* maskChannels(const unsigned int *channels);

void hide(vector<unsigned int> &pixels, const string text)
{
	cerr << "DEBUG hiding message: '" << text << "'\n";

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
		channels = splitChannels(pixel);

		// separate the three bytes of the character (i.e. 0x06f)
		letterBytes = separateCharacterBytes(c);

		// mask the bytes to prepare to 'AND' in the letter bytes
		maskedChannels = maskChannels(channels);

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

	// mask out a null terminator in the next pixel, so that we will know where to stop when finding
	unsigned int pNUL = pixels[i] & 0xFFF0F0F0;
	pixels[i] = pNUL;

	delete[] channels;
	delete[] letterBytes;
	delete[] maskedChannels;
	delete[] newBytes;
}

// split the pixel into its four channels (A, R, G, B)
unsigned int* splitChannels(const unsigned int pixel)
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
unsigned int* separateCharacterBytes(const unsigned char c)
{
	unsigned int *letterBytes = new unsigned int[3];

	letterBytes[0] = ((int)c << 8) & 0x00FF0000;
	letterBytes[1] = ((int)c << 4) & 0x0000FF00;
	letterBytes[2] = ((int)c) & 0x0000000F;

	return letterBytes;
}

// mask the pixel's channel bytes to allow for the
// character's bits to be inserted
unsigned int* maskChannels(const unsigned int *channels)
{
	unsigned int *maskedChannels = new unsigned int[4];

	maskedChannels[0] = channels[0];
	maskedChannels[1] = channels[1] & 0xFFF0FFFF;
	maskedChannels[2] = channels[2] & 0xFFFFF0FF;
	maskedChannels[3] = channels[3] & 0xFFFFFFF0;

	return maskedChannels;
}


//*******************************************************
// void find(vector<unsigned int> &pixels, string &text)
//
// Searches for a string hidden inside the pixels
//*******************************************************

void findCharacterInPixel(const unsigned int pixel, char &result)
{
	// ignore alpha channel (channels[0]). it's irrelevant
	unsigned int *channels = splitChannels(pixel);

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

void find(vector<unsigned int> &pixels, string &text)
{
	bool foundNUL = false;
	unsigned int i = -1;
	stringstream ss;

	do
	{
		i++;

		// get the pixel
		unsigned int pixel = pixels[i];

		// find the hidden char
		char hiddenChar;
		findCharacterInPixel(pixel, hiddenChar);

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
	} while (!foundNUL); // go until we hit a null terminator

	text = ss.str();
}


//****************************************************
// int encode(string carrierFile, string secretFile)
//
// Hides the text inside the image.
// This is what is called by the main function.
//****************************************************

int encode(string carrierFile, string secretFile)
{
	if (!boost::algorithm::ends_with(carrierFile, ".bmp"))
	{
		cerr << "ERROR - the input file must be a bitmap image.\n";
		return (int)EXIT_CODES::INVALID_INPUT_FILE_TYPE;
	}

	if (!boost::algorithm::ends_with(secretFile, ".txt"))
	{
		cerr << "ERROR - the secret file must be a text file.\n";
		return (int)EXIT_CODES::INVALID_INPUT_FILE_TYPE;
	}

	// file streams
	ifstream infile(carrierFile, ios::in | ios::binary);
	ifstream sfile(secretFile, ios::in);

	if (infile.fail() || sfile.fail())
	{
		cerr << "ERROR - unable to open one or both input files."
			"\nEnsure that both files exist and try again.\n";
		return (int)EXIT_CODES::ENC_FILE_FAIL;
	}

	// read in the secret text
	string SECRET_TEXT;
	readStringFromFile(sfile, SECRET_TEXT);

	// file metadata and pixels
	cout << "Reading data from file\n";
	Metadata *mdata = readMetadata(infile);
	vector<unsigned int> pixels;

	// is the image big enough to hold the file plus a null terminator?
	if ((SECRET_TEXT.length() + 1) > mdata->pixelCount)
	{
		// no, it is not. print error message and exit with non-zero exit code
		cout << "ERROR: the image (" << mdata->pixelCount << " pixels) is not large enough to hold the text ("
			<< SECRET_TEXT.length() << " bytes)\nPlease select an image file that is large enough." << endl;
		return (int)EXIT_CODES::ENC_IMAGE_TOO_SMALL;
	}

	// read in pixels
	cout << "Reading pixels from file\n";
	readPixelsFromFile(infile, pixels, mdata->sizeHeaders, mdata->pixelCount);

	// we're done with the infile. close it
	infile.close();

	// hide the text inside the image
	cout << "Hiding message '" << SECRET_TEXT << "' inside " << carrierFile << "\n";
	hide(pixels, SECRET_TEXT);

	// write the file
	ofstream outfile("out.bmp", ios::out | ios::binary);
	cout << "Writing the new pixels to 'out.bmp'\n";
	outfile.seekp(0L, ios::beg);
	outfile.write(mdata->headers, mdata->sizeHeaders);
	for (unsigned int i = 0; i < mdata->pixelCount; i++)
		outfile.write(reinterpret_cast<char*>(&(pixels.at(i))), sizeof(unsigned int));
	outfile.close();

	// clean up
	delete[] mdata->headers;
	delete mdata;

	return (int)EXIT_CODES::OK;
}


//***********************************************************
// int decode(string carrierFile)
//
// Finds the hidden message inside an image file.
// (If the message is garbled nonsense, there's probably no
// message hidden inside it.)
//
// This is what is called by the main function.
//***********************************************************

int decode(string carrierFile)
{
	if (!boost::algorithm::ends_with(carrierFile, ".bmp"))
	{
		cerr << "ERROR - the input file must be a bitmap image.\n";
		return (int)EXIT_CODES::INVALID_INPUT_FILE_TYPE;
	}

	string result;
	ifstream infile(carrierFile, ios::in | ios::binary);

	if (infile.fail())
	{
		cerr << "ERROR - unable to open the input file."
			"\nEnsure that the file exists and try again.\n";
		return (int)EXIT_CODES::DEC_FILE_FAIL;
	}

	// file metadata and pixels
	Metadata *mdata = readMetadata(infile);
	vector<unsigned int> pixels;

	// read all the pixels
	readPixelsFromFile(infile, pixels, mdata->sizeHeaders, mdata->pixelCount);

	// we're done with the infile. close it
	infile.close();

	// find the message
	find(pixels, result);

	cout << "There was a message hidden in the file!\n";
	cout << "\"" << result << "\"" << endl;

	return (int)EXIT_CODES::OK;
}