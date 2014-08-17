include $(NEU_HOME)/Makefile.defs

TARGET = database1
OBJECTS = main.o

LIBS = -L$(NEU_LIB) -lneu_core

all: .depend $(TARGET)

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

