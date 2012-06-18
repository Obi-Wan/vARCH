
.global
  .buffer:
    .string     "          "
  .bufferEnd:
    .i32_t      13
.end

.function  "integerToString"
  .param  %R1         %T001 ; number
  .param  %R2         %T002 ; string address
  .param  %R3         %T003 ; string length

  .local
    .const .baseAscii0:
      .i32_t    48
  .end

  MOV:.i32_t,   0 ,     %T005
; Find rightmost string position
  ADD:.i32_t,   %T003,  -%T002
; Start conversion
.convert:
; Test is string is overflowed
  LO:.i32_t,    %T005+, %T003
  IFNJ:.i32_t,  @exitError
; Convert position
  MOV:.i32_t,   %T001,  %T004
  QUOT:.i32_t,  10,     %T004
  ADD:.i32_t,   .baseAscii0 ,   %T004
; Store the char in the string
  MOV:.i32_t,   %T004,  (%T002)-
; increment number of processed chars
  DIV:.i32_t,   10,     %T001
  TZJ:.i32_t,   @exit
  JMP:.i32_t,   @convert
.exitError:
  MOV:.i32_t,   0 ,     %T005
.exit:
  RET:.i32_t,   %T005
.end
