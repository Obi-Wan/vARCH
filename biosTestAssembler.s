#include "std_io.s"
#include "std_conversion.s"

.global
  ; string to write
  .string1:
    .string     "test: yeah"
    .int        $10           ; '\n'
    .int        $13
  .stringError:
    .string     "Error"
    .int        $13
.end

.function   "main"
.init:
  MOV     @string1    %T001
  MOV     $4          %R6
; call the printing subroutine
  JSR     @print      %T001
; call recursive subroutine
  MOV     $4          %T002
  JSR     @recursive  %T002
; call conversion subroutine
  MOV     @bufferEnd  %T003
  SUB     @buffer     %T003
  MOV     $15234      %T004
  MOV     @buffer     %T005
  JSR     @integerToString  %T004  %T005  %T003
; Verify result
  MOV     %R1         %T008
  EQ      %T008       $0
  IFJ     @error
  MOV     @buffer     %T007
  JSR     @print      %T007
  HALT
.error:
  MOV     @stringError  %T001
  JSR     @print        %T001
  HALT
.end

.function   "recursive"
  .param  %R1         %T001

  .local
    .decrement:
      .const .int     $2
    .numberOfCalls:
      .shared .int    $0
    .lowerBound:
      .int            $0
    .stateRegister:
      .int            $0
  .end

  SUB     .decrement  %T001
  INCR    .numberOfCalls
  LO      %T001       .lowerBound
  IFJ     @exit
  JSR     @recursive  %T001
.exit:
  MOV     %SR         .stateRegister
  RET
.end

