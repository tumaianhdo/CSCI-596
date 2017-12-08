CC := cc
CXX := CC
FC := ftn

override DSPACES_DIR := ${SCRATCH}/software/install/dataspaces
override DSPACES_INC := $(shell ${DSPACES_DIR}/bin/dspaces_config -c)
override DSPACES_LIB := $(shell ${DSPACES_DIR}/bin/dspaces_config -l)

SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c, %.o, ${SOURCES})

all : test_writer test_reader 

test_writer: common.o test_put_run.o test_writer.o
	${CC} -o $@ $^ ${DSPACES_LIB}

test_reader: common.o test_get_run.o test_reader.o
	${CC} -o $@ $^ ${DSPACES_LIB}

%.o : %.c
	${CC} ${DSPACES_INC} -c $< -o $@

clean:
	rm -f test_reader test_writer *.o
