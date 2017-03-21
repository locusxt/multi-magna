DEBUGFLAGS = -O3

CXX      = g++
CXXFLAGS = -I. -I/usr/include $(DEBUGFLAGS) -pedantic
LDFLAGS  = -lpthread
LDSTATIC = -lpthread -static
LINK     = $(CXX)

TARGET = magna
OBJS = magna.o genetic.o graph.o multipermutation.o permutation.o carrier.o utils.o
SRCS = magna.cpp genetic.cpp graph.cpp multipermutation.cpp permutation.cpp carrier.cpp utils.cpp

.SUFFIXES: .o .cpp .h
%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $<

all: $(OBJS)
	$(LINK) -o $(TARGET) $(OBJS) $(LDFLAGS)

# $(TARGET): $(OBJS)

FLCXX      = $(shell fltk-config --cxx)
FLCXXFLAGS = $(shell fltk-config --cxxflags ) -I. -I/usr/include $(DEBUGFLAGS) 
FLLDFLAGS  = $(shell fltk-config --ldflags )
FLLDSTATIC = $(shell fltk-config --ldstaticflags )
FLLINK     = $(FLCXX)

FLTARGET = magnafluidui
FLOBJS = magnaui.o magnauimain.o genetic.o graph.o multipermutation.o permutation.o carrier.o
FLSRCS = magnaui.cpp magnauimain.cpp genetic.cpp graph.cpp multipermutation.cpp permutation.cpp carrier.cpp

magnaui.o: magnaui.cpp magnaui.h magnauifluidadd.h magnauibase.h
	$(FLCXX) $(FLCXXFLAGS) -c magnaui.cpp
magnauimain.o: magnauimain.cpp magnaui.h magnauifluidadd.h magnauibase.h
	$(FLCXX) $(FLCXXFLAGS) -c magnauimain.cpp

.SUFFIXES: .o .cpp .h
# %.o: %.cpp %.h
# 	$(FLCXX) $(FLCXXFLAGS) -c $<

gui: $(FLOBJS)
	$(FLLINK) -o $(FLTARGET) $(FLOBJS) $(FLLDSTATIC)

clean:
	rm -f *.o 2> /dev/null
	rm -f $(TARGET) $(TARGET).exe $(FLTARGET) $(FLTARGET).exe 2> /dev/null
