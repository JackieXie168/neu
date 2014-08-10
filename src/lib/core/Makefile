C_MODULES = compress.o

CPP_MODULES = global.o nreal.o nstr.o nvar.o NError.o NThread.o NRegex.o NClass.o NObjectBase.o NObject.o NCommand.o NResourceManager.o NSys.o NProgram.o NMLGenerator.o NRandom.o NProc.o NEncoder.o NCommunicator.o NServer.o NBroker.o NDatabase.o

SUB_MODULES = nml/parse.tab.o nml/NMLParser.o nml/parse.l.o

ALL_MODULES = $(CPP_MODULES) $(C_MODULES) $(SUB_MODULES)

all: .depend libneu_core

.depend: $(CPP_MODULES:.o=.cpp)
	$(COMPILE_C) -MM $(C_MODULES:.o=.c) > .depend
	$(COMPILE) -MM $(CPP_MODULES:.o=.cpp) >> .depend

-include .depend

neu_ml:
	(cd nml; $(MAKE))

libneu_core: $(C_MODULES) $(CPP_MODULES) neu_ml

ifeq ($(PLATFORM), Darwin)
	$(LINK) -single_module -dynamiclib -o $(NEU_LIB)/libneu_core.$(VERSION).dylib $(ALL_MODULES) -L/usr/local/lib -lgmp -lmpfr -lz -install_name $(NEU_HOME)/lib/libneu_core.$(VERSION).dylib
	(cd $(NEU_LIB); ln -f -s libneu_core.$(VERSION).dylib libneu_core.dylib)
	(cd $(NEU_LIB); ln -f -s libneu_core.$(VERSION).dylib libneu_core.$(MAJOR).dylib)
	(cd $(NEU_LIB); ln -f -s libneu_core.$(VERSION).dylib libneu_core.$(MAJOR).$(MINOR).dylib)
	rm -f $(NEU_LIB)/libneu_core_static.a
	ar rcs $(NEU_LIB)/libneu_core_static.a $(ALL_MODULES)
else
	$(LINK) -shared -Wl,-soname,libneu_core.so -o $(NEU_LIB)/libneu_core.so.$(VERSION) $(ALL_MODULES) -L/usr/local/lib -lgmp -lmpfr -lz -lboost_regex
	(cd $(NEU_LIB); ln -f -s libneu_core.so.$(VERSION) libneu_core.so)
	(cd $(NEU_LIB); ln -f -s libneu_core.so.$(VERSION) libneu_core.so.$(MAJOR))
	(cd $(NEU_LIB); ln -f -s libneu_core.so.$(VERSION) libneu_core.so.$(MAJOR).$(MINOR))
	rm -f $(NEU_LIB)/libneu_core_static.a
	ar rcs $(NEU_LIB)/libneu_core_static.a $(ALL_MODULES)
endif

$(C_MODULES): $(@.o=.c)
	$(COMPILE_C) -c $< -o $@

$(CPP_MODULES): $(@.o=.cpp)
	$(COMPILE) -c $< -o $@

clean:
	rm -f .depend
	rm -f $(C_MODULES)
	rm -f $(CPP_MODULES)
	(cd nml; $(MAKE) clean)

spotless: clean
	rm -f $(NEU_LIB)/libneu_core.*
	rm -f $(NEU_LIB)/libneu_core_static.a
	(cd nml; $(MAKE) spotless)
