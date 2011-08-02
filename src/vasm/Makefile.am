AM_YFLAGS = -d
#AM_LFLAGS = --header-file=$(<:.lpp=.h)

bin_PROGRAMS = vasm

vasm_SOURCES = \
			asm-parser.ypp \
		  IncludesTree.cpp \
		  asm-lexer.lpp \
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
		  disassembler/Disassembler.cpp \
		  vNewAsm.cpp

