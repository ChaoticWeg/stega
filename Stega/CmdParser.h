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