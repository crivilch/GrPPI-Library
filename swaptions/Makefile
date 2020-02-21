PREFIX := ${PARSECDIR}/pkgs/apps/swaptions/inst/${PARSECPLAT}
DEF = -DENABLE_PARSEC_HOOKS -DOUTPUT_FILE
INCLUDE = -I/home/crivilch/eclipse-workspace/swaptions
CXXFLAGS = --std=c++14 -mno-sse2 -O3 -Wall 
EXEC = swaptions 

ifdef version
  ifeq "$(version)" "pthreads" 
    DEF := $(DEF) -DENABLE_THREADS
    CXXFLAGS := $(CXXFLAGS) -pthread
  endif
  ifeq "$(version)" "tbb"
    DEF := $(DEF) -DENABLE_THREADS -DTBB_VERSION
    LIBS := $(LIBS) -ltbb
  endif
  ifeq "$(version)" "grppi"
    DEF := $(DEF) -DENABLE_GRPPI -DGRPPI_OMP
    INCLUDE := $(INCLUDE) -I/home/crivilch/Downloads/grppi-master/include
    CXXFLAGS := $(CXXFLAGS) -pthread -fopenmp 
  endif
  ifeq "$(version)" "omp"
    DEF := $(DEF) -DENABLE_OPENMP
    CXXFLAGS := $(CXXFLAGS) -fopenmp
  endif
endif

OBJS= CumNormalInv.o MaxFunction.o RanUnif.o nr_routines.o icdf.o \
	HJM_SimPath_Forward_Blocking.o HJM.o HJM_Swaption_Blocking.o  \
	HJM_Securities.o hooks.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(DEF) $(OBJS) $(INCLUDE) $(LIBS) -o $(EXEC)

.cpp.o:
	$(CXX) $(CXXFLAGS) $(DEF) $(INCLUDE) -c $*.cpp -o $*.o

.c.o:
	$(CXX) $(CXXFLAGS) $(DEF) -c $*.c -o $*.o

clean:
	rm -f $(OBJS) $(EXEC)

install:
	mkdir -p $(PREFIX)/bin
	cp -f $(EXEC) $(PREFIX)/bin/$(EXEC)

