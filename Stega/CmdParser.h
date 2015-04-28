//**************************************************************
// File:        CmdParser.h
// Author:      Shawn Lutch
// Description: Object that parses command line arguments, given
//              the number of arguments (int argc) and the
//              array of arguments (char **argv)
//**************************************************************

#ifndef STEGA_CLA_H
#define STEGA_CLA_H

#include <string>

enum Mode { INVALID, HELP, ENCODE, DECODE };

class CmdParser
{
private:
	void parse();

	int _argc;
	char **_argv;

	Mode mode;
	std::string *carrierFile, *secretFile;

public:
	Mode getMode() const;
	std::string getCarrierFile() const;
	std::string getSecretFile() const;

	CmdParser(int argc, char** argv);
	~CmdParser();
};

#endif