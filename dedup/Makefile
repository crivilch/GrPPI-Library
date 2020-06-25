# Makefile for dedup kernel

PREFIX=${PARSECDIR}/pkgs/kernels/dedup/inst/${PARSECPLAT}

TARGET=dedup

CFLAGS += -Wall -fno-strict-aliasing -D_XOPEN_SOURCE=600 --std=c++14  -mno-sse2 -O3

ostype=$(findstring solaris, ${PARSECPLAT})

ifeq "$(ostype)" "solaris"
    CFLAGS += -std=gnu99
endif

LIBS += -lm -I/home/crivilch/eclipse-workspace/dedup/src

DEDUP_OBJ = hashtable.o util.o dedup.o rabin.o decoder.o  mbuffer.o sha.o hooks.o

# Uncomment the following to enable gzip compression
CFLAGS += -DENABLE_GZIP_COMPRESSION
LIBS += -lz

# Uncomment the following to enable bzip2 compression
#CFLAGS += -DENABLE_BZIP2_COMPRESSION
#LIBS += -lbz2

ifdef version
  ifeq "$(version)" "pthreads"
    CFLAGS += -DENABLE_PTHREADS -pthread -DENABLE_STATISTICS
    DEDUP_OBJ += queue.o binheap.o tree.o encoder.o
  endif
  ifeq "$(version)" "grppi"
    CFLAGS += -g -pthread -fopenmp -DGRPPI_OMP -DENABLE_PTHREADS 
    LIBS += -I/home/crivilch/Downloads/grppi-master/include
    DEDUP_OBJ += queue.o binheap.o tree.o encoder_grppi.o
  endif
  ifeq "$(version)" "farm"
    CFLAGS += -pthread -DENABLE_PTHREADS
    LIBS += -I/home/crivilch/Downloads/grppi-master/include
    DEDUP_OBJ += queue.o binheap.o tree.o encoder_farm.o
  endif
else
  DEDUP_OBJ += encoder.o
endif


all: $(TARGET)

.c.o:
	$(CXX) -c $(CFLAGS) $(LIBS) $< -o $@

.cpp.o:
	$(CXX) -c $(CFLAGS) $(LIBS) $< -o $@

$(TARGET): $(DEDUP_OBJ)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(DEDUP_OBJ) $(LIBS)

clean:
	rm -f *~ *.o $(TARGET)

install:
	mkdir -p $(PREFIX)/bin
	cp -f $(TARGET) $(PREFIX)/bin/$(TARGET)

