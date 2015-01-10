COMPILE += $(JAVA_INCLUDE)
LINK += $(JAVA_LINK)

CPP_MODULES = NJObject.o

ALL_MODULES = $(CPP_MODULES)

all: .depend libneu_java

.depend: $(CPP_MODULES:.o=.cpp)
	$(COMPILE) -MM $(CPP_MODULES:.o=.cpp) > .depend
	touch .depend

-include .depend

libneu_java: $(CPP_MODULES) $(META_MODULES)

ifeq ($(PLATFORM), Darwin)
	$(LINK) -single_module -dynamiclib -o $(NEU_LIB)/libneu_java.$(VERSION).dylib $(ALL_MODULES) -L$(NEU_HOME)/lib -lneu_core -lneu -install_name $(NEU_HOME)/lib/libneu_java.$(VERSION).dylib
	(cd $(NEU_LIB); ln -f -s libneu_java.$(VERSION).dylib libneu_java.dylib)
	(cd $(NEU_LIB); ln -f -s libneu_java.$(VERSION).dylib libneu_java.$(MAJOR).dylib)
	(cd $(NEU_LIB); ln -f -s libneu_java.$(VERSION).dylib libneu_java.$(MAJOR).$(MINOR).dylib)
	rm -f $(NEU_LIB)/libneu_java_static.a
	ar rcs $(NEU_LIB)/libneu_java_static.a $(ALL_MODULES)
else
	$(LINK) -shared -Wl,-soname,libneu_java.so -o $(NEU_LIB)/libneu_java.so.$(VERSION) $(ALL_MODULES) -L$(NEU_HOME)/lib -lneu_core -ljvm
	(cd $(NEU_LIB); ln -f -s libneu_java.so.$(VERSION) libneu_java.so)
	(cd $(NEU_LIB); ln -f -s libneu_java.so.$(VERSION) libneu_java.so.$(MAJOR))
	(cd $(NEU_LIB); ln -f -s libneu_java.so.$(VERSION) libneu_java.so.$(MAJOR).$(MINOR))
	rm -f $(NEU_LIB)/libneu_java_static.a
	ar rcs $(NEU_LIB)/libneu_java_static.a $(ALL_MODULES)
endif

$(CPP_MODULES): $(@.o=.cpp)
	$(COMPILE) -c $< -o $@

clean:
	rm -f .depend
	rm -f *_meta.h
	rm -f $(CPP_MODULES)

spotless: clean
	rm -f $(NEU_LIB)/libneu_java.*
	rm -f $(NEU_LIB)/libneu_java_static.a	
