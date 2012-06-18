#include "std_io.s"
#include "std_conversion.s"

.global
  ; string to write
  .string1:
    .string     "test: yeah"
    .i32_t      10           ; '\n'
    .i32_t      13
  .stringError:
    .string     "Error"
    .i32_t      13
.end

.function   "main"
.init:
  MOV:.i32_t,     @string1 ,   %T001
  MOV:.i32_t,     4 ,          %R6
; call the printing subroutine
  JSR:.i32_t,     @print ,     %T001
; call recursive subroutine
  MOV:.i32_t,     4 ,          %T002
  MOV:.i32_t,     %T002 ,      %T012
  MOV:.i32_t,     %T012 ,      %T013
  MOV:.i32_t,     %T013 ,      %T014
  JSR:.i32_t,     @recursive , %T014
; call conversion subroutine
  MOV:.i32_t,     @bufferEnd , %T003
  SUB:.i32_t,     @buffer ,    %T003
  MOV:.i32_t,     15234 ,      %T004
  MOV:.i32_t,     @buffer ,    %T005
  JSR:.i32_t,     @integerToString , %T004 , %T005 , %T003
; Verify result
  MOV:.i32_t,     %R1 ,        %T008
  EQ:.i32_t,      %T008 ,      0
  IFJ:.i32_t,     @error
  MOV:.i32_t,     @buffer ,    %T007
  JSR:.i32_t,     @print ,     %T007
  HALT:.i32_t
.error:
  MOV:.i32_t,     @stringError , %T001
  JSR:.i32_t,     @print ,       %T001
  HALT:.i32_t
.end

.function   "recursive"
  .param  %R1         %T001

  .local
    .const .decrement:
      .i32_t  2
    .shared .numberOfCalls:
      .i32_t  0
    .lowerBound:
      .i32_t  0
    .stateRegister:
      .i32_t  0
    .localString:
      .string "local_string"
      .i32_t  13
  .end

  SUB:.i32_t,     .decrement , %T001
  INCR:.i32_t,    .numberOfCalls
  LO:.i32_t,      %T001 ,      .lowerBound
  IFJ:.i32_t,     @exit
  JSR:.i32_t,     @recursive , %T001
.exit:
  MOV:.i32_t,     %SR ,        .stateRegister
  RET:.i32_t
.end

