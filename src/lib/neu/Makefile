COMPILE += -I$(NEU_HOME)/lib/src -I$(LLVM_DIR)/include -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS

LINK += -L$(NEU_LIB)

CPP_MODULES = NMObject.o NMGenerator.o NPQueue.o NNet.o NNModule.o

META_MODULES = NConcept.o NCOntology.o NLib.o

SUB_MODULES = nm/parse.tab.o nm/NMParser.o nm/parse.l.o npl/parse.tab.o npl/NPLParser.o npl/parse.l.o npl/NPLModule.o

ALL_MODULES = $(CPP_MODULES) $(SUB_MODULES) $(META_MODULES)

all: .depend libneu

.depend: $(CPP_MODULES:.o=.cpp)
	$(COMPILE) -MM $(CPP_MODULES:.o=.cpp) $(META_MODULES:.o=.cpp) -DMETA_GUARD > .depend

-include .depend

neu_m:
	(cd nm; $(MAKE))

neu_pl:
	(cd npl; $(MAKE))

LLVM_LINK = -L$(LLVM_DIR)/lib -lLLVMX86CodeGen -lLLVMSelectionDAG -lLLVMX86Desc -lLLVMX86Info -lLLVMX86AsmPrinter -lLLVMX86Utils -lLLVMJIT -lLLVMCodeGen -lLLVMScalarOpts -lLLVMTransformUtils -lLLVMAnalysis -lLLVMTarget -lLLVMObject -lLLVMMCParser -lLLVMBitReader -lLLVMExecutionEngine -lLLVMMC -lLLVMCore -lLLVMSupport -lcurses -ldl -lz

libneu: $(CPP_MODULES) $(META_MODULES) neu_m neu_pl

ifeq ($(PLATFORM), Darwin)
	$(LINK) -single_module -dynamiclib -o $(NEU_LIB)/libneu.$(VERSION).dylib $(ALL_MODULES) -L$(NEU_HOME)/lib -lneu_core $(LLVM_LINK) -install_name $(NEU_HOME)/lib/libneu.$(VERSION).dylib
	(cd $(NEU_LIB); ln -f -s libneu.$(VERSION).dylib libneu.dylib)
	(cd $(NEU_LIB); ln -f -s libneu.$(VERSION).dylib libneu.$(MAJOR).dylib)
	(cd $(NEU_LIB); ln -f -s libneu.$(VERSION).dylib libneu.$(MAJOR).$(MINOR).dylib)
	rm -f $(NEU_LIB)/libneu_static.a
	ar rcs $(NEU_LIB)/libneu_static.a $(ALL_MODULES)
else
	$(LINK) -shared -Wl,-soname,libneu.so -o $(NEU_LIB)/libneu.so.$(VERSION) $(ALL_MODULES) -L$(NEU_HOME)/lib -lneu_core $(LLVM_LINK)
	(cd $(NEU_LIB); ln -f -s libneu.so.$(VERSION) libneu.so)
	(cd $(NEU_LIB); ln -f -s libneu.so.$(VERSION) libneu.so.$(MAJOR))
	(cd $(NEU_LIB); ln -f -s libneu.so.$(VERSION) libneu.so.$(MAJOR).$(MINOR))
	rm -f $(NEU_LIB)/libneu_static.a
	ar rcs $(NEU_LIB)/libneu_static.a $(ALL_MODULES)
endif

$(CPP_MODULES): $(@.o=.cpp)
	$(COMPILE) -c $< -o $@

NConcept.o: $(@.o=.cpp)
	neu-meta -class NConcept NConcept.cpp
	$(COMPILE) -c $< -o $@

NCOntology.o: $(@.o=.cpp)
	neu-meta -class Method -no-factory NCOntology.cpp
	neu-meta -class NCOntology -no-factory NCOntology.cpp
	neu-meta -class NCCodeGen -no-factory NCOntology.cpp
	$(COMPILE) -c $< -o $@

NLib.o: $(@.o=.cpp)
	neu-meta -class NLib NLib.cpp
	$(COMPILE) -c $< -o $@

clean:
	rm -f .depend
	rm -f *_meta.h
	rm -f $(META_MODULES)
	rm -f $(CPP_MODULES)
	rm -f $(C_MODULES)
	(cd nm; $(MAKE) clean)
	(cd npl; $(MAKE) clean)

spotless: clean
	rm -f $(NEU_LIB)/libneu.*
	rm -f $(NEU_LIB)/libneu_static.a	
	(cd nm; $(MAKE) spotless)
	(cd npl; $(MAKE) spotless)
