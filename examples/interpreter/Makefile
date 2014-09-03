include $(NEU_HOME)/Makefile.defs

TARGET = interpreter
OBJECTS = main.o

LIBS = -L$(NEU_LIB) -lneu_core -ledit

COMPILE += -I$(HOME)/neu-local/include

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

