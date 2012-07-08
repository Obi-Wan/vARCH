
; subroutine that prints a string, and counts the printed chars
.function   "print"
  .param    %R1         %T001 ; address of the string to print

  .local
    .const .printCmd:
      .i32_t   131072
    .const .printCmd1:
      .i32_t   131073
    .const .endChar:
      .i8_t   13
  .end

  MOV,    0 ,    %T002
.test:
  EQ,     (%T001) : .i8_t,    .endChar : .i8_t
  IFJ,    @exit
  PUT,    (%T001)+ : .i8_t,   .printCmd1
  INCR,   %T002
  JMP,    @test
.exit:
  RET,    %T002
.end

.function   "get_end_char"
  .local
    .const .endChar:
      .i8_t   13
  .end

  MOV,    .endChar : .i8_t,   %T001
  RET,    %T001
.end

