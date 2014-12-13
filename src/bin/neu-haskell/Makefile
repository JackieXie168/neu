TARGET = $(NEU_HOME)/bin/neu-haskell
MODULES = main.o

LIBS = -L$(NEU_LIB) -lneu -lneu_core -ledit

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
