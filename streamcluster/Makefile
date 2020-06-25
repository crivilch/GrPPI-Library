# Makefile for streamcluster

PREFIX=${PARSECDIR}/pkgs/kernels/streamcluster/inst/${PARSECPLAT}
DEF = -DENABLE_PARSEC_HOOKS 
INCLUDE = -I/home/crivilch/eclipse-workspace/streamcluster
CXXFLAGS = --std=c++14 -mno-sse2 -O3 -Wall
TARGET=streamcluster
OBJS=streamcluster.o hooks.o

ifdef version
  ifeq "$(version)" "pthreads"
    CXXFLAGS :=	$(CXXFLAGS) -DENABLE_THREADS -pthread
    OBJS += parsec_barrier.o
  endif
  ifeq "$(version)" "tbb"
    CXXFLAGS := $(CXXFLAGS) -DTBB_VERSION
    INCLUDE = -I/home/crivilch/Downloads/parsec-3.0-core/parsec-3.0/pkgs/libs/tbblib/src/include
    LIBS := $(LIBS) -ltbb
  endif
   ifeq "$(version)" "grppi"
    DEF := $(DEF) -DENABLE_GRPPI -DGRPPI_OMP 
    INCLUDE := $(INCLUDE) -I/home/crivilch/Downloads/grppi-master/include
    CXXFLAGS := $(CXXFLAGS) -pthread -fopenmp 
  endif
   ifeq "$(version)" "grppi_t"
    DEF := $(DEF) -DENABLE_GRPPI -DGRPPI_OMP  -DENABLE_THREADS
    INCLUDE := $(INCLUDE) -I/home/crivilch/Downloads/grppi-master/include
    CXXFLAGS := $(CXXFLAGS) -pthread -fopenmp 
    OBJS += parsec_barrier.o
  endif
endif

all: $(OBJS)
	$(CXX) $(CXXFLAGS) $(DEF) $(INCLUDE) $(LDFLAGS) $(OBJS) $(LIBS) -o $(TARGET)

%.o : %.cpp
	$(CXX) $(CXXFLAGS) $(DEF) $(INCLUDE) -c $<

.c.o:
	$(CXX) $(CXXFLAGS) $(DEF) -c $*.c -o $*.o

clean:
	rm -f *.o $(TARGET)

install:
	mkdir -p $(PREFIX)/bin
	cp -f $(TARGET) $(PREFIX)/bin/$(TARGET)

