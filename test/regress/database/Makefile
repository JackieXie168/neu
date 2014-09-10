include $(NEU_HOME)/Makefile.defs

TARGET = database
OBJECTS = main.o

LIBS = -L$(NEU_LIB) -lneu_core

all: .depend $(TARGET)
	./database >test.out 2>&1
	./database >>test.out 2>&1

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
	rm -rf testdb

spotless: clean
	rm -f $(TARGET)

