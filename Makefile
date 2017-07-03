
INC+=-I/usr/include/ncurses
DEFS=

CXX=c++
CXXFLAGS=-Wall -O2 -pedantic -std=c++11 $(INC) $(DEFS)
LD=c++
LDFLAGS=


LIBS+=-lncurses -lmenu

all: dropsview.o config.o gui.o misc.o
	$(LD) $^ $(LDFLAGS) $(LIBS) -o dropsview

clean:
	rm -f *.o

dropsview.o: dropsview.cc gui.h
	$(CXX) $(CXXFLAGS) -c $<

config.o: config.cc config.h
	$(CXX) $(CXXFLAGS) -c $<

gui.o: gui.cc gui.h
	$(CXX) $(CXXFLAGS) -c $<

misc.o: misc.cc misc.h
	$(CXX) $(CXXFLAGS) -c $<

