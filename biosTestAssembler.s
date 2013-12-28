#include "std_io.s"
#include "std_conversion.s"

#define DEFINITION  "try!"
#define OTHER_DEF   MOV,     @string1 ,   %T001
#define PARAM( x, y ) MOV, x, y

.global
  ; string to write
  .string1:
    .string     "test: yeah"
    .i8_t      10           ; '\n'
    .i8_t      0
  .stringError:
    .string     "Error"
    .i8_t      0
.end

.function   "main"
.init:
  MOV,     @string1 ,   %T001
  MOV,     4 ,          %R6
; call the printing subroutine
  JSR,     @print ,     %T001
; call recursive subroutine
  MOV,     4 ,          %T002
  MOV,     %T002 ,      %T012
  MOV,     %T012 ,      %T013
  MOV,     %T013 ,      %T014
  JSR,     @recursive , %T014
; call conversion subroutine
  MOV,     @bufferEnd , %T003
  SUB,     @buffer ,    %T003
  MOV,     15234 ,      %T004
  MOV,     @buffer ,    %T005
  JSR,     @integerToString , %T004 , %T005 , %T003
; Verify result
  MOV,     %R0 ,        %T008
  EQ,      %T008 ,      0
  IFJ,     @error
  MOV,     @buffer ,    %T007
  JSR,     @print ,     %T007
  HALT
.error:
  MOV,     @stringError , %T001
  JSR,     @print ,       %T001
  HALT
.end

.function   "recursive"
  .param  %R1         %T001

  .local
    .decrement: .const
      .i32_t  2
    .numberOfCalls: .static
      .i32_t  0
    .lowerBound:
      .i32_t  0
    .stateRegister:
      .i32_t  0
    .localString:
      .string "local_string"
      .i8_t  0
  .end

  SUB,     .decrement , %T001
  INCR,    .numberOfCalls
  LO,      %T001 ,      .lowerBound
  IFJ,     @exit
  JSR,     @recursive , %T001
.exit:
  MOV,     %SR ,        .stateRegister
  RET
.end

