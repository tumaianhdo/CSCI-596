CC := cc
CXX := CC
FC := ftn

override ADIOS_DIR := ${SCRATCH}/software/install/adios
override ADIOS_INC := $(shell ${ADIOS_DIR}/bin/adios_config -c)
override ADIOS_LIB := $(shell ${ADIOS_DIR}/bin/adios_config -l)

SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c, %.o, ${SOURCES})

all : md pv stage_read_write

md: md.o
	${CC} -o $@ $^ ${ADIOS_LIB}

pv: pv.o decompose_block.o utils.o
	${CC} -o $@ $^ ${ADIOS_LIB}

stage_read_write: utils.o decompose_block.o stage_read_write.o
	${CC} -o $@ $^ ${ADIOS_LIB} -lm

%.o : %.c
	${CC} ${ADIOS_INC} -c $< -o $@


clean:
	rm -f md pv stage_read_write *.o
