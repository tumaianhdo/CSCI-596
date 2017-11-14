CC := mpicc
CXX := mpicxx
FC := mpif90

SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c, %.o, ${SOURCES})
EXECS	:= $(patsubst %.c, %, ${SOURCES})

all : ${EXECS}

%: %.c
	${CC} $< -o $@ -lm

clean:
	rm -f ${EXECS}

