/*
 * AsmChecker.h
 *
 *  Created on: 09/feb/2013
 *      Author: ben
 */

#ifndef ASMCHECKER_H_
#define ASMCHECKER_H_

#include "../asm-program.h"

class AsmChecker {
public:
  static void checkArgs(const asm_instruction_statement & stmt);
  static void checkArgs(const asm_function_call & stmt);
  static void checkArgs(const asm_return_statement & stmt);

  static void checkCallParameters(const asm_function_call & stmt,
      const deque<asm_function *> & functions);

  static void checkInstructions(const asm_program & prog,
      const bool & usingTemps);

  static bool ensureTempsUsage(const asm_function & func, const bool & used);
  static void ensureTempsUsage(const asm_program & prog, const bool & used);
};

#endif /* ASMCHECKER_H_ */
