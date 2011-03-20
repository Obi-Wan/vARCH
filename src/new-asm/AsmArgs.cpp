/*
 * AsmArgs.cpp
 *
 *  Created on: 20/mar/2011
 *      Author: ben
 */

#include "AsmArgs.h"

#include <iostream>

using namespace std;

void
AsmArgs::parse() throw (WrongArgumentException)
{
  for(uint32_t argNum = 1; argNum < argc; argNum++) {
    const string tempArg(argv[argNum]);

    if (!tempArg.compare("-c")) {
      /* Tells which file to compile */
      CHECK_THROW( inputName.empty(),
          WrongArgumentException("Double definition of input file") );
      CHECK_THROW( (++argNum < argc),
          WrongArgumentException("You didn't specify the input name") );
      inputName.assign(argv[argNum]);

    } else if (!tempArg.compare("-o")) {
      /* Tells which file will be the output */
      CHECK_THROW( outputName.empty(),
          WrongArgumentException("Double definition of output file") );
      CHECK_THROW( (++argNum < argc),
          WrongArgumentException("You didn't specify the output name") );
      outputName.assign(argv[argNum]);

    } else if (!tempArg.substr(0, 2).compare("-I")) {
      /* Tells to append an include dir */
      includeDirs.push_back(tempArg.substr(2, tempArg.size()-2));

    } else if (!tempArg.compare("-h")) {
      throw WrongArgumentException("");
    } else {
      throw WrongArgumentException(string("Unknown option: ") + argv[argNum]);
    }
  }

  DebugPrintf(("Input  filename: %s\n", inputName.c_str()));
  DebugPrintf(("Output filename: %s\n", outputName.c_str()));
#ifdef DEBUG
  string includes;
  for(uint32_t inclNum = 0; inclNum < includeDirs.size(); inclNum++) {
    includes += " " + includeDirs[inclNum];
  }
  DebugPrintf(("Include dirs: %s\n\n", includes.c_str()));
#endif

  CHECK_THROW( !inputName.empty(),
          WrongArgumentException("you didn't specify the input file") );
  CHECK_THROW( !outputName.empty(),
          WrongArgumentException("you didn't specify the output file") );
}

void
AsmArgs::printHelp() const throw()
{
  cout << "Syntax is: 'new-asm -c <input_file> -o <output_file>'" << endl;
}
