#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <map>
#include <zlib.h>

#include "common.h"
#include "sim.h"
#include "trace.h" 
#include "cache.h"


int main(int argc, char** argv) { simulator_main(argc, argv); }

VOID dprint_trace(Trace_op *trace_op)
{
  std::cout << "*****Trace Data **** " << std::endl;
  std::cout <<"inst_addr:" << std::hex << trace_op->instruction_addr << std::endl;
  std::cout <<"inst_size:" << (int)  trace_op->inst_size << std::endl;
  std::cout <<"num_src:" << (int) trace_op->num_src << std::endl;
  std::cout <<"src[0]:"<< (int) trace_op->src[0] << std::endl;
  std::cout <<"src[1]:"<< (int) trace_op->src[1] << std::endl;
  std::cout <<"dst: " <<(int) trace_op->dst << std::endl;
  std::cout <<"opcode: " << (int) trace_op->opcode << std::endl;
  std::cout <<"is_fp: " << (int) trace_op->is_fp << std::endl;
  std::cout <<"cf_type: " << (int) trace_op->cf_type << std::endl;
  std::cout <<"mem_type: " << (int) trace_op->mem_type << std::endl;
  std::cout <<"write_flag: " << (int) trace_op->write_flag << std::endl;
  std::cout <<"ld_vaddr: " << std::hex << trace_op->ld_vaddr << std::endl;
  std::cout <<"st_vaddr: " << std::hex <<  trace_op->st_vaddr << std::endl;
  std::cout <<"instruction_addr: " << (int) trace_op->instruction_addr << std::endl;
  std::cout <<"branch_traget: " << (int) trace_op->branch_target << std::endl;
  std::cout <<"actually_taken: " << (int) trace_op->actually_taken << std::endl;
  std::cout <<"mem_read_size: " << (int)trace_op->mem_read_size << std::endl;
  std::cout <<"mem_write_size: " << (int) trace_op->mem_write_size << std::endl;
}


void copy_trace_op(Trace_op *trace_op, Op *op)
{
  op->num_src               = trace_op->num_src;
  op->src[0]                = trace_op->src[0];
  op->src[1]                = trace_op->src[1];
  op->dst                   = trace_op->dst;
  op->opcode                = trace_op->opcode; 
  op->is_fp                 = trace_op->is_fp;
  op->cf_type               = trace_op->cf_type;
  op->mem_type              = trace_op->mem_type;
  op->write_flag             = trace_op->write_flag;
  op->inst_size             = trace_op->inst_size;
  op->ld_vaddr              = trace_op->ld_vaddr;
  op->st_vaddr              = trace_op->st_vaddr;
  op->instruction_addr      = trace_op->instruction_addr;
  op->branch_target         = trace_op->branch_target;
  op->actually_taken        = trace_op->actually_taken;
  op->mem_read_size         = trace_op->mem_read_size;
  op->mem_write_size        = trace_op->mem_write_size;

}

