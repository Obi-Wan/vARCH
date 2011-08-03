/*
 * Optimizer.h
 *
 *  Created on: 02/ago/2011
 *      Author: ben
 */

#ifndef OPTIMIZER_H_
#define OPTIMIZER_H_

#include "../asm-function.h"

class Optimizer {
public:
//  Optimizer();

  void removeUselessMoves(asm_function & func);
};

#endif /* OPTIMIZER_H_ */
