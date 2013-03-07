
#include "common.h"
#include "sim.h"
#include "trace.h"


ADDRINT g_vaddr, g_st_vaddr, g_target, g_eflags;
uint32_t g_mem_read_size, g_mem_write_size;
bool g_actually_taken;
uint32_t eflag_value =0; 
map<ADDRINT,Trace_op *> inst_storage;

void init_trace_op(Trace_op *); 
VOID write_inst(ADDRINT iaddr);


VOID get_ld_ea(ADDRINT addr, UINT32 mem_read_size){
  g_vaddr = addr;
  g_mem_read_size = mem_read_size;
}

VOID get_st_ea(ADDRINT addr, UINT32  mem_st_size){
  g_st_vaddr = addr;
  g_mem_write_size = mem_st_size;
}

VOID get_target(ADDRINT target, bool taken){
  g_target = target;
  g_actually_taken = taken;
}



void ins_decode(INS ins, gzFile stream) 
{
  const ADDRINT iaddr = INS_Address(ins);
  Trace_op *trace_op; 
  bool decode_is_done = FALSE; 
  if (inst_storage.find(iaddr) == inst_storage.end()) {
    trace_op = new Trace_op;
    inst_storage[iaddr] = trace_op; 
    init_trace_op(trace_op); 
  }
  else {
    trace_op = inst_storage[iaddr]; // in this case we do not need to decode the code again; 
    decode_is_done = TRUE; 
  }

  if (!decode_is_done) {
    trace_op->inst_size = INS_Size(ins);
    trace_op->instruction_addr = iaddr;

    int num_src = 0; 

    for(UINT32 ii = 0; ii < INS_OperandCount(ins); ii++)
    {

      if (INS_OperandWritten(ins,ii)) {
        REG reg = INS_OperandReg(ins, ii); 
        if (((int)reg > 0 )
          && ((int)reg < 32)) {
            trace_op->dst = ((int) reg)%NUM_REG; 
        }
      }
      if (INS_OperandRead(ins,ii)) {
        REG reg = INS_OperandReg(ins, ii); 
        if (REG_is_fr(reg)) trace_op->is_fp = true; 
        if (((int)reg > 0 )
          && ((int)reg < 32)) { 
            trace_op->src[MIN2(num_src,2)] = ((int) reg)%NUM_REG; 
            num_src++; 
        }
      }
    }

    trace_op->num_src = MIN2(num_src, 2); 
    /* instruction decoding */ 
    xed_iclass_enum_t iclass = static_cast<xed_iclass_enum_t>(INS_Opcode(ins));
    xed_extension_enum_t extension = static_cast<xed_extension_enum_t>(INS_Extension(ins));
    /* converting X86 instructions into RISC-style instructions */ 
    /* students do not need to understand this code */

    if ( (iclass == XED_ICLASS_NOP) ||
      (iclass == XED_ICLASS_FNOP))
      trace_op->opcode = OP_NOP;
    else if ( (iclass == XED_ICLASS_IMUL) ||
      (iclass == XED_ICLASS_MUL) ||
      (iclass == XED_ICLASS_PMULLW) ||
      (iclass == XED_ICLASS_PMULUDQ) ||
      (iclass == XED_ICLASS_PMULHUW) ||
      (iclass == XED_ICLASS_PMULHW) ||
      (iclass == XED_ICLASS_MULPS) ||
      (iclass == XED_ICLASS_MULSS ) ||
      (iclass == XED_ICLASS_MULPD) ||
      (iclass == XED_ICLASS_MULSD))
      trace_op->opcode = OP_IMUL;
    else if ( (iclass == XED_ICLASS_DIV) ||
      (iclass == XED_ICLASS_IDIV) ||
      (iclass == XED_ICLASS_DIVPS) ||
      (iclass == XED_ICLASS_DIVSS) ||
      (iclass == XED_ICLASS_DIVPD) ||
      (iclass == XED_ICLASS_DIVSD))
      trace_op->opcode = OP_IDIV;
    else if ((iclass == XED_ICLASS_FDIV) ||
      (iclass == XED_ICLASS_FDIVR) ||
      (iclass == XED_ICLASS_FDIVRP) ||
      (iclass == XED_ICLASS_FDIVP))
      trace_op->opcode = OP_FDIV;
    else if ( (iclass == XED_ICLASS_FMUL) ||
      (iclass == XED_ICLASS_PFMUL) ||
      (iclass == XED_ICLASS_FMULP))
      trace_op->opcode = OP_FMUL;
    else
      trace_op->opcode = OP_IMUL; 

    if (extension == XED_EXTENSION_SSE ||
      extension == XED_EXTENSION_SSE2 ||
      extension == XED_EXTENSION_SSE3 ||
      extension == XED_EXTENSION_SSSE3 ||
      extension == XED_EXTENSION_SSE4) {
        trace_op->opcode = OP_MM; 
        trace_op->is_fp = true;
    }

  }


  /* branch instruction decoding */ 


  if( INS_IsRet(ins) )
  {
    trace_op->cf_type = CF_RET;
  }
  else if( INS_IsSyscall(ins) )
  {
    trace_op->cf_type = CF_ICO; 
  }
  else if (INS_IsDirectBranchOrCall(ins))
  {
    if( INS_IsCall(ins) )
      trace_op->cf_type = CF_CALL;
    else
      trace_op->cf_type = CF_CBR;
  }
  else if( INS_IsIndirectBranchOrCall(ins) )
  {
    if( INS_IsCall(ins) )
      trace_op->cf_type = CF_ICALL;
    else
      trace_op->cf_type = CF_IBR;  
  }

  if (trace_op->cf_type) {

    trace_op->opcode = OP_CF; 

    trace_op->dst = -1; // control flow instruction's write operation is ignored. 

    INS_InsertCall(ins,
      IPOINT_BEFORE,
      (AFUNPTR)get_target,
      IARG_BRANCH_TARGET_ADDR,
      IARG_BRANCH_TAKEN,
      IARG_END);
  }
  else { 

    // if control flow instruction access memory, we ignore memory operations 
    //  memory instruction decoding

    if (INS_IsMemoryWrite(ins))
    {

      trace_op->opcode = OP_ST; 
      trace_op->mem_type = MEM_ST; 
      INS_InsertCall(ins,
        IPOINT_BEFORE,
        (AFUNPTR)get_st_ea,
        IARG_MEMORYWRITE_EA,
        IARG_MEMORYWRITE_SIZE,
        IARG_END);

    }
    else if (INS_IsMemoryRead(ins)){
      trace_op->opcode = OP_LD; 
      trace_op->mem_type = MEM_LD; 
      INS_InsertCall(ins,
        IPOINT_BEFORE,
        (AFUNPTR)get_ld_ea,
        IARG_MEMORYREAD_EA,
        IARG_MEMORYREAD_SIZE,
        IARG_END);

    }
    /* to simplify the decode, only store or load information is simulated */ 


  }


  INS_InsertCall(ins,
    IPOINT_BEFORE,
    (AFUNPTR)write_inst,
    IARG_INST_PTR, 
    IARG_END);

  if (KNOB_PRINT_INST.Value()) {
    INS_InsertCall(ins,
      IPOINT_BEFORE,
      (AFUNPTR)dprint_inst,
      IARG_INST_PTR,
      IARG_PTR, new string(INS_Disassemble(ins)),
      IARG_END);
  }

}



VOID write_inst(ADDRINT iaddr)
{
  Trace_op *trace_op        = inst_storage[iaddr]; 

  trace_op->ld_vaddr         = g_vaddr; 
  trace_op->st_vaddr         = g_st_vaddr;
  trace_op->branch_target    = g_target;
  trace_op->actually_taken   = g_actually_taken;
  trace_op->mem_read_size    = g_mem_read_size;
  trace_op->mem_write_size   = g_mem_write_size;
  trace_op->write_flag        = g_eflags ? true : false; 


  g_vaddr          = 0;
  g_st_vaddr       = 0;
  g_target         = 0;
  g_actually_taken = 0;
  g_mem_read_size  = 0;
  g_mem_write_size = 0;
  g_eflags         = 0;

  if (gzwrite(stream, trace_op,sizeof(Trace_op)) != sizeof(Trace_op))
    cout << "Error when writting instruction " << inst_count << endl;
  inst_count++;
}


VOID dprint_trace(Trace_op *trace_op)
{
  std::cout << "*****Trace Data **** " << endl;
  std::cout <<"inst_addr:" << hex << trace_op->instruction_addr << endl;
  std::cout <<"inst_size:" << (int)  trace_op->inst_size << endl;
  std::cout <<"num_src:" << (int) trace_op->num_src << endl;
  std::cout <<"src[0]:"<< (int) trace_op->src[0] << endl;
  std::cout <<"src[1]:"<< (int) trace_op->src[1] << endl;
  std::cout <<"dst: " <<(int) trace_op->dst << endl;
  std::cout <<"opcode: " << (int) trace_op->opcode << endl;
  std::cout <<"is_fp: " << (int) trace_op->is_fp << endl;
  std::cout <<"cf_type: " << (int) trace_op->cf_type << endl;
  std::cout <<"mem_type: " << (int) trace_op->mem_type << endl;
  std::cout <<"write_flag: " << (int) trace_op->write_flag << endl;
  std::cout <<"ld_vaddr: " << hex << trace_op->ld_vaddr << endl;
  std::cout <<"st_vaddr: " << hex <<  trace_op->st_vaddr << endl;
  std::cout <<"instruction_addr: " << (int) trace_op->instruction_addr << endl;
  std::cout <<"branch_traget: " << (int) trace_op->branch_target << endl;
  std::cout <<"actually_taken: " << (int) trace_op->actually_taken << endl;
  std::cout <<"mem_read_size: " << (int)trace_op->mem_read_size << endl;
  std::cout <<"mem_write_size: " << (int) trace_op->mem_write_size << endl;
}


VOID dprint_inst(ADDRINT iaddr, string *disassemble_info) {
  Trace_op *trace_op = inst_storage[iaddr]; 
  std::cout << "*****begin of the data structure **** " << dec << inst_count << endl;
  std::cout << "disassemble : "<< *disassemble_info << endl; 
  std::cout <<"inst_addr:" << hex << trace_op->instruction_addr << endl;
  std::cout <<"inst_size:" << (int)  trace_op->inst_size << endl;
  std::cout <<"num_src:" << (int) trace_op->num_src << endl;
  std::cout <<"src[0]:"<< (int) trace_op->src[0] << endl;
  std::cout <<"src[1]:"<< (int) trace_op->src[1] << endl;
  std::cout <<"dst: " <<(int) trace_op->dst << endl;
  std::cout <<"opcode: " << (int) trace_op->opcode << endl;
  std::cout <<"is_fp: " << (int) trace_op->is_fp << endl;
  std::cout <<"cf_type: " << (int) trace_op->cf_type << endl;
  std::cout <<"mem_type: " << (int) trace_op->mem_type << endl;
  std::cout <<"write_flag: " << (int) trace_op->write_flag << endl;
  std::cout <<"ld_vaddr: " << hex << trace_op->ld_vaddr << endl;
  std::cout <<"st_vaddr: " << hex <<  trace_op->st_vaddr << endl;
  std::cout <<"instruction_addr: " << (int) trace_op->instruction_addr << endl;
  std::cout <<"branch_traget: " << (int) trace_op->branch_target << endl;
  std::cout <<"actually_taken: " << (int) trace_op->actually_taken << endl;
  std::cout <<"mem_read_size: " << (int)trace_op->mem_read_size << endl;
  std::cout <<"mem_write_size: " << (int) trace_op->mem_write_size << endl;
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



void init_trace_op(Trace_op *trace_op) 
{
  trace_op->num_src               = 0; 
  trace_op->src[0]                = -1; 
  trace_op->src[1]                = -1;
  trace_op->dst                   = -1; 
  trace_op->opcode                = 0; 
  trace_op->is_fp                 = false;
  trace_op->cf_type               = NOT_CF;
  trace_op->mem_type              = NOT_MEM;
  trace_op->write_flag             = 0;
  trace_op->inst_size             = 0;
  trace_op->ld_vaddr              = 0;
  trace_op->st_vaddr              = 0;
  trace_op->instruction_addr      = 0;
  trace_op->branch_target         = 0;
  trace_op->actually_taken        = 0;
  trace_op->mem_read_size         = 0;
  trace_op->mem_write_size        = 0;
}


