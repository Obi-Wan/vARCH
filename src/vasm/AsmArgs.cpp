/*
 * AsmArgs.cpp
 *
 *  Created on: 20/mar/2011
 *      Author: ben
 */

#include "AsmArgs.h"

#include <iostream>
#include <cstdlib>

using namespace std;

void
AsmArgs::parse() throw (WrongArgumentException)
{
  for(int32_t argNum = 1; argNum < argc; argNum++) {
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

    } else if (!tempArg.substr(0, 2).compare("-O")) {
      /* Tells to append an include dir */
      CHECK_THROW( tempArg.size() == 3,
          WrongArgumentException(
              "Optimization Level should be between 0 and 3") );

      optimLevel = atoi(tempArg.substr(2, tempArg.size()-2).c_str());

      CHECK_THROW( optimLevel >= 0 && optimLevel <= 3,
          WrongArgumentException(
              "Optimization Level should be between 0 and 3") );

    } else if (!tempArg.compare("-reg-auto-alloc")) {
      regAutoAlloc = true;

    } else if (!tempArg.compare("-d")) {
      /* Tells which file will be the output */
      CHECK_THROW( debugSymbolsName.empty(),
          WrongArgumentException("Double definition of Debug Symbols file") );
      CHECK_THROW( (++argNum < argc),
          WrongArgumentException(
              "You didn't specify the Debug Symbols file name") );
      debugSymbolsName.assign(argv[argNum]);

    } else if (!tempArg.compare("-h")) {
      throw WrongArgumentException("");
    } else {
      throw WrongArgumentException(string("Unknown option: ") + argv[argNum]);
    }
  }

  InfoPrintf(("Input  filename: %s\n", inputName.c_str()));
  InfoPrintf(("Output filename: %s\n", outputName.c_str()));
#ifdef INFO
  string includes;
  for(uint32_t inclNum = 0; inclNum < includeDirs.size(); inclNum++) {
    includes += " " + includeDirs[inclNum];
  }
#endif
  InfoPrintf(("DebugSymbols filename: %s\n", debugSymbolsName.c_str()));
  InfoPrintf(("Include dirs: %s\n", includes.c_str()));
  InfoPrintf(("Optimization Level: %u\n", optimLevel));
  InfoPrintf(("Register auto allocation: %s\n\n",
                regAutoAlloc ? "Enabled" : "Disabled"));

  CHECK_THROW( !inputName.empty(),
          WrongArgumentException("you didn't specify the input file") );
  CHECK_THROW( !outputName.empty(),
          WrongArgumentException("you didn't specify the output file") );
}

void
AsmArgs::printHelp() const throw()
{
  cout << "Syntax is: 'new-asm -c <input_file> -o <output_file>'" << endl;
  cout << "To specify additional paths for searching the include files:" << endl
        << "  -I<include_path> as many times as you want" << endl;
  cout << "To generate a file containing debug symbols:" << endl
        << "  -d <output_file> (if extension is .xml, an XML file will be "
            "generated)" << endl;
  cout << "To let the assembler auto allocate registers:" << endl
        << "  add flag -reg-auto-alloc" << endl;
  cout << "To enable some sort of optimizations:" << endl
        << "  -O<optimization_level> with <optimization_level> as a number "
        << "between 0 and 3" << endl;
}
