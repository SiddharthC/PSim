#ifndef SIM_H 
#define SIM_H

#define TRUE 1
#define FALSE 0

#define NUM_REG 32
#define LD_ST_BUFFER_SIZE 16 

#define MAX2(a,b) ((a)>(b) ? (a) : (b))
#define MIN2(a,b) ((a)<(b) ? (a) : (b))

#define MAX_CTR  0xffffffffffffffffULL
#define MAX_INT64  0x7fffffffffffffffLL

enum Op_code_num{
  OP_INV,       // invalid opcode           
  OP_NOP,       // is a decoded nop
  OP_CF,        // change of flow           
  OP_CMOV,      // conditional move         
  OP_LDA,       // load address             
  OP_LD,        // load operation 
  OP_ST,        // store operation 
  OP_IADD,      // integer add              
  OP_IMUL,      // integer multiply         
  OP_ICMP,      // integer compare          
  OP_IDIV,      // integer divide 
  OP_LOGIC,     // logical                  
  OP_SHIFT,     // shift                    
  OP_BYTE,      // byte manipulation        
  OP_MM,        // multimedia instructions  
  OP_FMEM,      // fp memory instruction    
  OP_FCF,
  OP_FCVT,      // floating point convert   
  OP_FADD,      // floating point add       
  OP_FMUL,      // floating point multiply  
  OP_FDIV,      // floating point divide    
  OP_FCMP,      // floating point compare   
  OP_FBIT,      // floating point bit       
  OP_FCMO,      // floating point cond move 
  NUM_OP_TYPE
};





typedef enum Cf_Type_enum { 
  NOT_CF,      // not a control flow instruction
  CF_BR,       // an unconditional branch
  CF_CBR,      // a conditional branch
  CF_CALL,     // a call
  CF_IBR,      // an indirect branch
  CF_ICALL,    // an indirect call
  CF_RET,      // a return
  CF_ICO,      // a system call 
  NUM_CF_TYPES, 
}Cf_Type;


typedef enum Mem_Type_enum{
  NOT_MEM,       // not a memory instruction
  MEM_LD,        // a load instruction
  MEM_ST,        // a store instruction
  NUM_MEM_TYPES,
}Mem_Type; 


/* Data structure for trace */ 
typedef struct Trace_op_struct{
  uint8_t num_src;   /* number or sources */
  int8_t  src[2];    /* source register id */ 
  int8_t  dst;       /* destiation register id */ 

  uint8_t opcode;    /* opcode */ 

  bool is_fp;        /* floating point instruction */ 
  Cf_Type cf_type;   /* control flow type */ 
  Mem_Type mem_type; /* memory operation type */ 
  bool write_flag;    /* Does instruction wrige flag (conditional code)? */

  uint8_t inst_size;   /* instruction size  (B) */ 

  ADDRINT/*uint32_t*/ ld_vaddr;           /* load virtual address */ 
  ADDRINT/*uint32_t*/ st_vaddr;           /* store virtual address */ 
  ADDRINT/*uint32_t*/ instruction_addr;   /* instruction address */ 
  ADDRINT/*uint32_t*/ branch_target;      /* branch target address */ 
  bool actually_taken;                    /* branch direction */ 

  uint8_t mem_read_size;   /* read memory size */ 
  uint8_t mem_write_size;  /* write memory size */ 

} Trace_op;


/* Note that Op structure is inherited from Trace_op */
typedef struct Op_struct : public Trace_op{
  uint64_t inst_id; 
  Op_struct *op_pool_next; 
  uint32_t  op_pool_id; 
  bool valid;
  bool mem_decrement;
  bool miss_predict;
  bool tlb_miss;
  uint64_t pte_addr;
  uint64_t physical_addr;
  int thread_id; 
  bool tlb_accessed;
  int dcache_result;
  /* when you add new element, you must change the init_op function also */ 
} Op; 


void init_op_latency(void); 

void ins_decode(INS ins, gzFile stream);
void copy_trace_op(Trace_op *trace_op, Op *op); 
void init_op_pool(void); 
Op *get_free_op(void);
void free_op(Op *op); 


void simulator_main(int argc, char** argv); 


extern uint64_t inst_count; 
extern uint64_t cycle_count;  
extern int64_t get_dram_row_id(ADDRINT addr); // NEW-LAB2 
extern int get_dram_bank_id(ADDRINT addr);  // NEW-LAB2 
extern void dcache_insert(ADDRINT addr); // NEW-LAB2 
extern void broadcast_rdy_op(Op* ops); // NEW-LAB2 
extern void broadcast_tlb_rdy_op(Op* op);
extern int  get_knob_value_mem_latency_row_hit(void); // NEW-LAB2 
extern int  get_knob_value_mem_latency_row_miss(void); // NEW-LAB2 
extern void tlb_install_from_mem(Op* op);


#endif
