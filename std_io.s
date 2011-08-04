
; subroutine that prints a string, and counts the printed chars
.function   "print"
  .param    %R1 ; address of the string to print

  .local
    .printCmd:
      .const .int     $131072
    .printCmd1:
      .const .int     $131073
    .endChar:
      .const .int     $13
  .end

  MOV     %R1         %T001
  MOV     $0          %T002
.test:
  EQ      (T001)      .endChar
  IFJ     @exit
  PUT     (T001)+     .printCmd1
  INCR    %T002
  JMP     @test
.exit:
  RET     %T002
.end

.function   "get_end_char"
  .local
    .endChar:
      .const .int     $13
  .end

  MOV     .endChar    %T001
  RET     %T001
.end

