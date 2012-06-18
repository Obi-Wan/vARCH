
; subroutine that prints a string, and counts the printed chars
.function   "print"
  .param    %R1         %T001 ; address of the string to print

  .local
    .const .printCmd:
      .i32_t   131072
    .const .printCmd1:
      .i32_t   131073
    .const .endChar:
      .i32_t   13
  .end

  MOV:.i32_t,  0 ,    %T002
.test:
  EQ:.i32_t,   (%T001),  .endChar
  IFJ:.i32_t,  @exit
  PUT:.i32_t,  (%T001)+, .printCmd1
  INCR:.i32_t, %T002
  JMP:.i32_t,  @test
.exit:
  RET:.i32_t,  %T002
.end

.function   "get_end_char"
  .local
    .const .endChar:
      .i32_t   13
  .end

  MOV:.i32_t,  .endChar,  %T001
  RET:.i32_t,  %T001
.end

