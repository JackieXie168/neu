include Makefile.defs

export MAJOR = 1
export MINOR = 7
export RELEASE = 1

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

llvm:
	rm -rf llvm
	$(HTTP_GET) http://www.llvm.org/releases/3.5.0/llvm-3.5.0.src.tar.xz
	$(HTTP_GET) http://www.llvm.org/releases/3.5.0/cfe-3.5.0.src.tar.xz
	tar xf llvm-3.5.0.src.tar.xz
	tar xf cfe-3.5.0.src.tar.xz
	mv llvm-3.5.0.src llvm
	mv cfe-3.5.0.src llvm/tools/clang
ifeq ($(PLATFORM), Darwin)
	cd llvm && export REQUIRES_RTTI=1 && ./configure --enable-libcpp --prefix=$(NEU_HOME)/llvm/build-release && make -j $(BUILD_THREADS) && make install
else
	cd llvm && export REQUIRES_RTTI=1 && ./configure --prefix=$(NEU_HOME)/llvm/build-release && make -j $(BUILD_THREADS) && make install
endif
	rm llvm-3.5.0.src.tar.xz
	rm cfe-3.5.0.src.tar.xz

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
