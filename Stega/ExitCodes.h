#ifndef EXIT_CODES_H
#define EXIT_CODES_H

enum EXIT_CODES {
	OK,										// exit code zero. we're ok
	INVALID_CLA, INVALID_INPUT_FILE_TYPE,	// invalid arguments given at runtime
	ENC_FILE_FAIL, ENC_IMAGE_TOO_SMALL,		// ENCODE: input file doesn't exist or is too small
	DEC_FILE_FAIL							// DECODE: input file doesn't exist
};

#endif