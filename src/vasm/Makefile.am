AM_YFLAGS = -d
#AM_LFLAGS = --header-file=$(<:.lpp=.h)

bin_PROGRAMS = vasm_low
#vasm_mid

COMMON_ASM_SRC = \
		  IncludesTree.cpp \
		  ErrorReporter.cpp \
		  ../FileHandler.cpp \
		  AsmArgs.cpp \
		  IR/IR_LowLevel_Arguments.cpp \
		  IR/IR_LowLevel_Statements.cpp \
		  algorithms/Labels.cpp	\
		  algorithms/TempsMap.cpp	\
		  algorithms/InterferenceGraph.cpp	\
		  asm-function.cpp \
		  asm-program.cpp \
		  backend/AssemFlowGraph.cpp \
		  backend/RegAllocator.cpp \
		  backend/Frame.cpp \
		  backend/Optimizer.cpp \
		  backend/AsmChecker.cpp \
		  disassembler/Disassembler.cpp \
		  preprocessor/AsmPreprocessor.cpp

vasm_low_SOURCES = \
		  ${COMMON_ASM_SRC} \
		  IR_Low_parser/IR_Low_parser.ypp \
		  IR_Low_parser/IR_Low_lexer.lpp \
		  AST/AST_Low/AST_Low_Tree.cpp \
		  AST/AST_Low/AST_Low_Function.cpp \
		  AST/AST_Low/AST_Low_Stmt.cpp \
		  vAsmLow.cpp

#vasm_mid_SOURCES = \
#		  ${COMMON_ASM_SRC} \
#		  IR_Mid_parser/IR_Mid_parser.ypp \
#		  IR_Mid_parser/IR_Mid_lexer.lpp \
#		  AST/AST_Mid/AST_Mid_Tree.cpp \
#		  vAsmMid.cpp

