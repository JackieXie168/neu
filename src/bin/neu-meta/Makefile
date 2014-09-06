TARGET = $(NEU_HOME)/bin/neu-meta
MODULES = main.o

COMPILE += -I$(LLVM_DIR)/include -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS

LIBS = -L$(LLVM_DIR)/lib -lclangFrontendTool -lclangFrontend -lclangDriver -lclangSerialization -lclangCodeGen -lclangParse -lclangSema -lclangStaticAnalyzerFrontend -lclangStaticAnalyzerCheckers -lclangStaticAnalyzerCore -lclangARCMigrate -lclangRewriteFrontend -lclangRewrite -lclangAnalysis -lclangEdit -lclangAST -lclangBasic -lclangLex -lclangTooling -lLLVMOption -lLLVMLinker -lLLVMIRReader -lLLVMipo -lLLVMVectorize -lLLVMInstrumentation -lLLVMBitWriter -lLLVMBitReader -lLLVMAsmParser -lLLVMX86Disassembler -lLLVMX86AsmParser -lLLVMX86CodeGen -lLLVMSelectionDAG -lLLVMAsmPrinter -lLLVMMCParser -lLLVMCodeGen -lLLVMObjCARCOpts -lLLVMScalarOpts -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis -lLLVMX86Desc -lLLVMX86Info -lLLVMTarget -lLLVMX86AsmPrinter -lLLVMMC -lLLVMObject -lLLVMX86Utils -lLLVMCore -lLLVMSupport -lz -lpthread -ldl -lm -lcurses -L$(NEU_LIB) -lneu_core

all: .depend $(TARGET)

.depend: $(MODULES:.o=.cpp)
	$(COMPILE) -MM $(MODULES:.o=.cpp) > .depend

-include .depend

$(MODULES): $(@.o=.cpp)
	$(COMPILE) -c $< -o $@

$(TARGET): $(MODULES)
	$(LINK) -o $(TARGET) $(MODULES) $(LIBS)

clean:
	rm -f $(MODULES)
	rm -f .depend

spotless: clean
	rm -f $(TARGET)
