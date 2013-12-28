
; subroutine that prints a string, and counts the printed chars
.function   "print"
  .param    %R1         %T001 ; address of the string to print

  .local
    .printCmd: .const
      .i32_t  131072
    .printCmd1: .const
      .i32_t  131073
    .endChar: .const
      .i16_t   0
  .end

  MOV,    0 ,    %T002
.test:
  EQ,     (%T001) : .i8_t,    .endChar : .i16_t
  IFJ,    @exit
  PUT,    (%T001)+ : .i8_t,   .printCmd1
  INCR,   %T002
  JMP,    @test
.exit:
  RET,    %T002
.end

.function   "get_end_char"
  .local
    .endChar: .const
      .i8_t   0
  .end

  MOV,    .endChar : .i8_t,   %T001
  RET,    %T001
.end

