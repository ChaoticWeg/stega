#include <string>

#include "boost/algorithm/string.hpp"
#include "CmdParser.h"

std::string CmdParser::getCarrierFile() const { return *carrierFile; }
std::string CmdParser::getSecretFile() const { return *secretFile; }
Mode CmdParser::getMode() const { return mode; }

CmdParser::CmdParser(int argc, char** argv)
{
	_argc = argc;
	_argv = argv;

	parse();
}

CmdParser::~CmdParser()
{
	delete carrierFile;
	delete secretFile;
}

void CmdParser::parse()
{
	// default mode is invalid
	mode = INVALID;

	// we need at least one argument, not counting
	// the program's call name
	if (_argc == 1)
		return;

	std::string modeStr(_argv[1]);
	boost::algorithm::to_lower(modeStr);

	// if we have two or more arguments (again, not
	// counting the program's call name), assume that
	// we are either encoding or decoding
	if (_argc >= 3)
	{

		// encode
		if (modeStr.compare("encode") == 0)
		{
			if (_argc > 3)
			{
				mode = ENCODE;
				carrierFile = new std::string(_argv[2]);
				secretFile = new std::string(_argv[3]);
			}
		}

		// decode
		else if (modeStr.compare("decode") == 0)
		{
			mode = DECODE;
			carrierFile = new std::string(_argv[2]);
			secretFile = NULL;
		}
	}

	// handling for "--help" (single argument)
	else if (modeStr.compare("--help") == 0)
		mode = HELP;
}