
.global
  .buffer:
    .string     "          "
  .bufferEnd:
    .int        $13
.end

.function  "integerToString"
  .param  %R1 ; number
  .param  %R2 ; string address
  .param  %R3 ; string length

  .local
    .baseAscii0:
      .const .int     $48
  .end

  MOV     %R1         %T001
  MOV     %R2         %T002
  MOV     %R3         %T003
  MOV     $0          %T005
; Find rightmost string position
  ADD     %T003       -%T002
; Start conversion
.convert:
; Test is string is overflowed
  LO      %T005+      %T003
  IFNJ    @exitError
; Convert position
  MOV     %T001       %T004
  QUOT    $10         %T004
  ADD     .baseAscii0 %T004
; Store the char in the string
  MOV     %T004       (T002)-
; increment number of processed chars
  DIV     $10         %T001
  TZJ     @exit
  JMP     @convert
.exitError:
  MOV     $0          %T005
  RET     %T005
.exit:
  RET     %T005
.end
