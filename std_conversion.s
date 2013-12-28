
.global
  .buffer:
    .string     "          "
  .bufferEnd:
    .i8_t      0
.end

.function  "integerToString"
  .param  %R1         %T001 ; number
  .param  %R2         %T002 ; string address
  .param  %R3         %T003 ; string length

  .local
    .baseAscii0: .const
      .i8_t    48
  .end

  MOV,    0 ,     %T005
; Find rightmost string position
  ADD,    %T003,  -%T002
; Start conversion
.convert:
; Test if string is overflowed
  LO,     %T005+, %T003
  IFNJ,   @exitError
; Convert position
  MOV,    %T001,  %T004
  QUOT,   10,     %T004
  ADD,    .baseAscii0 : .i8_t,   %T004 : .i8_t
; Store the char in the string
  MOV,    %T004 : .i8_t,  (%T002)- : .i8_t
; increment number of processed chars
  DIV,    10,     %T001
  TZJ,    @exit
  JMP,    @convert
.exitError:
  MOV,    0 ,     %T005
.exit:
  RET,    %T005
.end
