#
# This is a makefile for lab 1.
#

CC = g++
LINK_OPTION = -lz
COMPILE_OPTION = -g

all: sim

clean:
	rm -f *.o *.out sim


sim: all_knobs.o knob.o cache.o sim_main.o memory.o bpred.o vmem.o sim.h sim.cpp
	${CC} ${COMPILE_OPTION} -o sim sim.cpp all_knobs.o knob.o cache.o sim_main.o memory.o bpred.o vmem.o ${LINK_OPTION}

all_knobs.o: all_knobs.cpp all_knobs.h
	${CC} -c ${COMPILE_OPTION} all_knobs.cpp

sim_main.o: sim_main.cpp 
	${CC} -c ${COMPILE_OPTION} sim_main.cpp

knob.o: knob.cpp knob.h
	${CC} -c ${COMPILE_OPTION} knob.cpp

cache.o: cache.cpp 
	${CC} -c ${COMPILE_OPTION} cache.cpp 

memory.o: memory.cpp memory.h
	${CC} -c ${COMPILE_OPTION} memory.cpp

bpred.o: bpred.cpp bpred.h
	${CC} -c ${COMPILE_OPTION} bpred.cpp

vmem.o: vmem.cpp vmem.h
	${CC} -c ${COMPILE_OPTION} vmem.cpp
