//***********************************************************
// File:		FileHeaders.h
// Author:		Tom McQueary
// Description: BMP and DIB headers for bitmap image files
//***********************************************************

#ifndef FILE_HEADERS_H
#define FILE_HEADERS_H


//***********************************************************
// BMPHeader
// Stores information about the BMP file
//***********************************************************

struct BMPHeader
{
	char fileSignature[2];
	unsigned int bmpFileSize;
	unsigned short int unused_1;
	unsigned short int unused_2;
	unsigned int pixelOffset;
};


//***********************************************************
// DIBHeader
// Stores information about the image contained in the file
//***********************************************************

struct DIBHeader
{
	unsigned int numBytesDIBHeader;
	unsigned int widthBitmapInPixels;	// 500
	unsigned int heightBitmapInPixels;	// 500
	unsigned short numColorPlanes;
	unsigned short numBitsPerPixel;
	unsigned int typeCompression;
	unsigned int sizeRawDataInPixelArray;
	unsigned int horizPhysicalResolution;
	unsigned int vertPhysicalResolution;
	unsigned int numColorsInPalette;
	unsigned int importantColors;
	unsigned int redChannelBitMask;
	unsigned int greenChannelBitMask;
	unsigned int blueChannelBitMask;
	unsigned int alphaChannelBitMask;
	unsigned int typeOfColorSpace;
	unsigned int colorSpaceEndPoints[9];
	unsigned int redGamma;
	unsigned int greenGamma;
	unsigned int blueGamma;
};

#endif