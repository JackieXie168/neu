include $(NEU_HOME)/Makefile.defs

TARGET = test
OBJECTS = main.o

LIBS = -L$(NEU_HOME)/lib -lneu_core -lneu

all: .depend $(TARGET)
	./test >test.out 2>&1

.depend: $(OBJECTS:.o=.cpp) $(OBJECTS:.o=.h)
	$(COMPILE) -MM $(OBJECTS:.o=.cpp) > .depend

-include .depend

%.o: %.cpp %.h
	$(COMPILE) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(LINK) -o $(TARGET) $(OBJECTS) $(LIBS)

clean:
	rm -f $(OBJECTS)
	rm -f .depend

spotless: clean
	rm -f $(TARGET)

