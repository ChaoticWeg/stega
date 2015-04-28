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
#include "StegaUtils.h"

using namespace std;

void readStringFromFile(ifstream &file, string &result);
void readPixelsFromFile(ifstream &file, vector<unsigned int> &pixels,
	unsigned int startPos, unsigned int numPixels);

int encode(string carrierFile, string secretFile);
int decode(string carrierFile);



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
	cerr << "Reading data from file\n";
	Metadata *mdata = Metadata::create(infile);
	vector<unsigned int> pixels;

	// is the image big enough to hold the file plus a null terminator?
	if ((SECRET_TEXT.length() + 1) > mdata->pixelCount)
	{
		// no, it is not. print error message and exit with non-zero exit code
		cerr << "ERROR: the image (" << mdata->pixelCount << " pixels) is not large enough to hold the text ("
			<< SECRET_TEXT.length() << " bytes)\nPlease select an image file that is large enough." << endl;
		return (int)EXIT_CODES::ENC_IMAGE_TOO_SMALL;
	}

	// read in pixels
	cerr << "Reading pixels from file\n";
	readPixelsFromFile(infile, pixels, mdata->sizeHeaders, mdata->pixelCount);

	// we're done with the infile. close it
	infile.close();

	// hide the text inside the image
	cerr << "Hiding message '" << SECRET_TEXT << "' inside " << carrierFile << "\n";
	StegaUtils::hide(pixels, SECRET_TEXT);

	// write the file
	ofstream outfile("out.bmp", ios::out | ios::binary);
	cerr << "Writing the new pixels to 'out.bmp'\n";
	outfile.seekp(0L, ios::beg);
	outfile.write(mdata->headers, mdata->sizeHeaders);
	for (unsigned int i = 0; i < mdata->pixelCount; i++)
		outfile.write(reinterpret_cast<char*>(&(pixels.at(i))), sizeof(unsigned int));
	outfile.close();

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
	Metadata *mdata = Metadata::create(infile);
	vector<unsigned int> pixels;

	// read all the pixels
	readPixelsFromFile(infile, pixels, mdata->sizeHeaders, mdata->pixelCount);

	// we're done with the infile. close it
	infile.close();

	// find the message
	StegaUtils::find(pixels, result);

	// if there wasn't a null terminator 
	if (result.length() == pixels.size() || result.length() == 0)
	{
		cerr << "!!! There was no message hidden in the file\n";
	}
	else
	{
		cerr << "Printing message to stdout" << endl;
		cout << result << endl;
	}

	delete mdata;
	return (int)EXIT_CODES::OK;
}