AM_YFLAGS = -d
#AM_LFLAGS = --header-file=$(<:.lpp=.h)

bin_PROGRAMS = vasm_low

COMMON_ASM_SRC = \
		  IncludesTree.cpp \
		  ../FileHandler.cpp \
		  asm-function.cpp \
		  asm-program.cpp \
		  AsmArgs.cpp \
		  IR/IR_LowLevel_Statements.cpp \
		  algorithms/Labels.cpp	\
		  algorithms/TempsMap.cpp	\
		  algorithms/InterferenceGraph.cpp	\
		  backend/AssemFlowGraph.cpp \
		  backend/RegAllocator.cpp \
		  backend/Frame.cpp \
		  backend/Optimizer.cpp \
		  disassembler/Disassembler.cpp

vasm_low_SOURCES = \
			${COMMON_ASM_SRC} \
			IR_Low_parser/IR_Low_parser.ypp \
		  IR_Low_parser/IR_Low_lexer.lpp \
		  vAsmLow.cpp

