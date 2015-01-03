include $(NEU_HOME)/Makefile.defs

TARGET = test
OBJECTS = main.o

LIBS = -L$(NEU_HOME)/lib -lneu_core -lneu_java

all: .depend $(TARGET)
	./test >test.out 2>&1

.depend: $(OBJECTS:.o=.cpp) $(OBJECTS:.o=.h)
	$(COMPILE) -MM $(OBJECTS:.o=.cpp) > .depend

-include .depend

%.o: %.cpp %.h
	$(COMPILE) -c $< -o $@

Test.o: Test.h
	neu-java -class Test Test.h
	$(COMPILE) -c Test.cpp -o Test.o

Test.class: Test.java
	javac Test.java

$(TARGET): $(OBJECTS) Test.o Test.class
	$(LINK) -o $(TARGET) $(OBJECTS) Test.o $(LIBS)

clean:
	rm -f Test.cpp
	rm -f Test.o
	rm -f $(OBJECTS)
	rm -f .depend

spotless: clean
	rm -f Test.class
	rm -f $(TARGET)
