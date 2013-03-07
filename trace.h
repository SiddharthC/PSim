#ifndef TRACE_H
#define TRACE_H
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <map>
#include <zlib.h>

#define VOID void 

#include "sim.h"
using namespace std; 


VOID dprint_trace(Trace_op *trace_op);
void copy_trace_op(Trace_op *trace_op, Op *op);


#endif 
