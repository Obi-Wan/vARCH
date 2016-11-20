/*
 * AsmArgs.cpp
 *
 *  Created on: 20/mar/2011
 *      Author: ben
 */

#include "AsmArgs.h"

#include <cstdio>
#include <iostream>

void
AsmArgs::parse()
{
  for(int32_t argNum = 1; argNum < argc; argNum++)
  {
    const std::string tempArg(argv[argNum]);

    if (!tempArg.compare("-c"))
    {
      onlyCompile = true;
    }
    else if (!tempArg.compare("-v") || !tempArg.compare("--verbose"))
    {
      verbose = true;
    }
    else if (!tempArg.compare("-o"))
    {
      /* Tells which file will be the output */
      CHECK_THROW( outputName.empty(),
          WrongArgumentException("Double definition of output file") );
      CHECK_THROW( (++argNum < argc),
          WrongArgumentException("You didn't specify the output name") );
      outputName.assign(argv[argNum]);
    }
    else if (!tempArg.substr(0, 2).compare("-I"))
    {
      /* Tells to append an include dir */
      includeDirs.push_back(tempArg.substr(2, tempArg.size()-2));
    }
    else if (!tempArg.substr(0, 2).compare("-O"))
    {
      /* Tells to append an include dir */
      CHECK_THROW( tempArg.size() == 3,
          WrongArgumentException(
              "Optimization Level should be between 0 and 3") );

      optimLevel = atoi(tempArg.substr(2, tempArg.size()-2).c_str());

      CHECK_THROW( optimLevel >= 0 && optimLevel <= 3,
          WrongArgumentException(
              "Optimization Level should be between 0 and 3") );
    }
    else if (!tempArg.compare("--reg-auto-alloc"))
    {
      regAutoAlloc = true;
    }
    else if (!tempArg.compare("--reg-coalesce"))
    {
      regCoalesce = true;
    }
    else if (!tempArg.compare("-d"))
    {
      /* Tells which file will be the output */
      CHECK_THROW( debugSymbolsName.empty(),
          WrongArgumentException("Double definition of Debug Symbols file") );
      CHECK_THROW( (++argNum < argc),
          WrongArgumentException(
              "You didn't specify the Debug Symbols file name") );
      debugSymbolsName.assign(argv[argNum]);
    }
    else if (!tempArg.compare("--only-validate"))
    {
      onlyValidate = true;
    }
    else if (!tempArg.compare("--disassemble-result"))
    {
      disassembleResult = true;
    }
    else if (!tempArg.compare("-fomit-frame-pointer"))
    {
      omitFramePointer = true;
    }
    else if (!tempArg.compare("-h"))
    {
      throw WrongArgumentException("");
    }
    else
    {
      inputFiles.push_back(argv[argNum]);
    }
  }

  if (this->verbose)
  {
    std::printf("Input filenames (Source): \n");
    for (const std::string & filename : this->getSrcInputNames())
    {
      std::printf("  - %s\n", filename.c_str());
    }
    std::printf("Input filenames (Object): \n");
    for (const std::string & filename : this->getObjInputNames())
    {
      std::printf("  - %s\n", filename.c_str());
    }
    std::printf("Output filename: %s\n", outputName.c_str());
    std::string includes;
    for(uint32_t inclNum = 0; inclNum < includeDirs.size(); inclNum++)
    {
      includes += " " + includeDirs[inclNum];
    }
    std::printf("DebugSymbols filename: %s\n", debugSymbolsName.c_str());
    std::printf("Included directories: %s\n", includes.c_str());
    std::printf("Optimization Level: %u\n", optimLevel);
    std::printf("Register auto allocation: %s\n",
                  regAutoAlloc ? "Enabled" : "Disabled");
    std::printf("Register Coalescing: %s\n",
                  regCoalesce ? "Enabled" : "Disabled");
    std::printf("Disassembling result: %s\n",
                  disassembleResult ? "Enabled" : "Disabled");
    std::printf("Only Validating: %s\n",
                  onlyValidate ? "Enabled" : "Disabled");
    std::printf("Only Compiling: %s\n",
                  onlyCompile ? "Enabled" : "Disabled");
    std::printf("Omit frame pointer: %s\n\n",
                  omitFramePointer ? "Enabled" : "Disabled");
  }

  CHECK_THROW( !inputFiles.empty(),
          WrongArgumentException("You didn't specify any input file") );
  CHECK_THROW( !outputName.empty(),
          WrongArgumentException("You didn't specify the output file") );
}

void
AsmArgs::printHelp() const throw()
{
  std::cout << "Syntax is: 'vasm <input_files> -o <output_file> [options]'" << std::endl;
  std::cout << " --only-validate : only parse, without emitting code" << std::endl;
  std::cout << " -v : Verbose mode, giving extra information" << std::endl;
  std::cout << " -c : Just compiles, and output a static object file" << std::endl;
  std::cout << " -I<include_path> : adds an include path" << std::endl;
  std::cout << " -d <output_file> : generates debug symbols file "
        << "(if extension is .xml, an XML file will be generated)" << std::endl;
  std::cout << " --reg-auto-alloc : Lets the assembler auto allocate registers" << std::endl;
  std::cout << " --reg-coalesce : (if --reg-auto-alloc) coalesces registers" << std::endl;
  std::cout << " --disassemble-result : outputs the disassembled code" << std::endl;
  std::cout << " -O<optimization_level> : between 0 and 3, enables different "
        << "optimization levels" << std::endl;
  std::cout << " -fomit-frame-pointer : omits frame pointer (not working, yet)" << std::endl;
}
