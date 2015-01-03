include Makefile.defs

export MAJOR = 1
export MINOR = 7
export RELEASE = 0

export VERSION = $(MAJOR).$(MINOR).$(RELEASE)

ifdef JAVA_INCLUDE
  all: neu neu-test neu-haskell libneu_java neu-java
else
  all: neu neu-test neu-haskell
endif

neu: libneu
	(cd src/bin/neu; $(MAKE))

neu-test: libneu_core
	(cd src/bin/neu-test; $(MAKE))	

neu-meta: libneu_core
	(cd src/bin/neu-meta; $(MAKE))

neu-haskell: libneu
	(cd src/bin/neu-haskell; $(MAKE))

neu-java: libneu_core
	(cd src/bin/neu-java; $(MAKE))

libneu_core:
	(cd src/lib/core; $(MAKE))

libneu: libneu_core neu-meta
	(cd src/lib/neu; $(MAKE))

libneu_java: libneu_core
	(cd src/lib/java; $(MAKE))

concepts: libneu
	(cd app/concepts/src/lib; $(MAKE))

spotless:
	(cd src/bin/neu; $(MAKE) spotless)
	(cd src/bin/neu-test; $(MAKE) spotless)
	(cd src/bin/neu-meta; $(MAKE) spotless)
	(cd src/bin/neu-haskell; $(MAKE) spotless)
	(cd src/lib/core; $(MAKE) spotless)
	(cd src/lib/neu; $(MAKE) spotless)
	(cd app/concepts/src/lib; $(MAKE) spotless)
ifdef JAVA_INCLUDE
	(cd src/bin/neu-java; $(MAKE) spotless)
	(cd src/lib/java; $(MAKE) spotless)
endif

clean:
	(cd src/bin/neu; $(MAKE) clean)
	(cd src/bin/neu-test; $(MAKE) clean)
	(cd src/bin/neu-meta; $(MAKE) clean)
	(cd src/bin/neu-haskell; $(MAKE) clean)
	(cd src/lib/core; $(MAKE) clean)
	(cd src/lib/neu; $(MAKE) clean)
	(cd app/concepts/src/lib; $(MAKE) clean)
ifdef JAVA_INCLUDE
	(cd src/bin/neu-java; $(MAKE) clean)
	(cd src/lib/java; $(MAKE) clean)
endif

release:
	$(MAKE) clean
	$(MAKE) -j $(BUILD_THREADS) NEU_RELEASE=1
