MODULES = NJSONParser.o

OBJECTS = parse.l.o parse.tab.o

all: .depend $(MODULES) $(OBJECTS)

.depend: $(MODULES:.o=.cpp)
	$(COMPILE) -MM $(MODULES:.o=.cpp) > .depend

-include .depend

%.o: %.cpp
	$(COMPILE) -c $< -o $@

parse.l.o: parse.l parse.y $(MODULES)
	bison -v -d parse.y
	flex -Pnjson_ -oparse.l.c parse.l
	$(COMPILE) -x c++ -c parse.l.c -o parse.l.o

parse.tab.o: parse.l.o $(MODULES)
	$(COMPILE) -x c++ -c parse.tab.c -o parse.tab.o

clean:
	rm -f $(MODULES)
	rm -f $(OBJECTS)
	rm -f parse.l.c
	rm -f parse.tab.c
	rm -f parse.tab.h
	rm -f .depend

spotless: clean
