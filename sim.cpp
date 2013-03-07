#include "common.h"
#include "sim.h"
#include "trace.h" 
#include "cache.h"  	/* NEW-LAB2 */ 
#include "memory.h"	/* NEW-LAB2 */ 
#include "bpred.h" 	/* NEW-LAB3 */
#include "vmem.h"   	/* NEW-LAB3 */
#include <stdlib.h>
#include <string.h> /* String operations library */ 
#include <ctype.h> /* Library for useful character operations */


/*******************************************************************/
/* Simulator frame */ 
/*******************************************************************/

bool run_a_cycle(memory_c *m ); // please modify run_a_cycle function argument  /** NEW-LAB2 */ 
void init_structures(memory_c *m); // please modify init_structures function argument  /** NEW-LAB2 */ 


/* My local functions*/
void init_regfile(void);
int total_cf_count=0;

/* uop_pool related variables */ 
uint32_t free_op_num;
uint32_t active_op_num; 
Op *op_pool; 
Op *op_pool_free_head = NULL; 


/* simulator related local functions */ 
bool icache_access(ADDRINT addr);  /** please change uint32_t to ADDRINT NEW-LAB2 */  
bool dcache_access(ADDRINT addr);  /** please change uint32_t to ADDRINT NEW-LAB2 */   
void  init_latches(void);


#include "knob.h"
#include "all_knobs.h"

// knob variables
KnobsContainer *g_knobsContainer; /* < knob container > */
all_knobs_c    *g_knobs; /* < all knob variables > */

#define HW_MAX_THREAD 4  /** NEW-LAB 4 */
gzFile g_stream[HW_MAX_THREAD];  /** NEW-LAB 4*/ 

void init_knobs(int argc, char** argv) {
  // Create the knob managing class
  g_knobsContainer = new KnobsContainer();

  // Get a reference to the actual knobs for this component instance
  g_knobs = g_knobsContainer->getAllKnobs();

  // apply the supplied command line switches
  char* pInvalidArgument = NULL;
  g_knobsContainer->applyComandLineArguments(argc, argv, &pInvalidArgument);

  g_knobs->display();
}

void read_trace_file(void) {
  g_stream[0] = gzopen((KNOB(KNOB_TRACE_FILE)->getValue()).c_str(), "r");

  if ((KNOB(KNOB_RUN_THREAD_NUM)->getValue())<2) return;    /** NEW-LAB4 **/
   g_stream[1] = gzopen((KNOB(KNOB_TRACE_FILE2)->getValue()).c_str(), "r"); /** NEW-LAB4 **/
   if ((KNOB(KNOB_RUN_THREAD_NUM)->getValue())<3) return;  /** NEW-LAB4 **/
   g_stream[2] = gzopen((KNOB(KNOB_TRACE_FILE3)->getValue()).c_str(), "r"); /** NEW-LAB4 **/
   if ((KNOB(KNOB_RUN_THREAD_NUM)->getValue())<4) return;  /** NEW-LAB4 **/
   g_stream[3] = gzopen((KNOB(KNOB_TRACE_FILE4)->getValue()).c_str(), "r"); /** NEW-LAB4 **/
}

// simulator main function is called from outside of this file 



void simulator_main(int argc, char** argv) {
  init_knobs(argc, argv);

  // trace driven simulation 
  read_trace_file();
  
  /* just note: passing main memory pointers is hack to mix up c++ objects and c-style code */  /* Not recommended at all */ 

  memory_c *main_memory = new memory_c();  // /** NEW-LAB2 */ 
  
  init_structures(main_memory);  // please modify run_a_cycle function argument  /** NEW-LAB2 */ 
  run_a_cycle(main_memory); // please modify run_a_cycle function argument  /** NEW-LAB2 */ 

}
int op_latency[NUM_OP_TYPE]; 

void init_op_latency(void) {
  op_latency[OP_INV]   = 1; 
  op_latency[OP_NOP]   = 1; 
  op_latency[OP_CF]    = 1; 
  op_latency[OP_CMOV]  = 1; 
  op_latency[OP_LDA]   = 1;
  op_latency[OP_LD]    = 1; 
  op_latency[OP_ST]    = 1; 
  op_latency[OP_IADD]  = 1; 
  op_latency[OP_IMUL]  = 2; 
  op_latency[OP_IDIV]  = 4; 
  op_latency[OP_ICMP]  = 2; 
  op_latency[OP_LOGIC] = 1; 
  op_latency[OP_SHIFT] = 2; 
  op_latency[OP_BYTE]  = 1; 
  op_latency[OP_MM]    = 2; 
  op_latency[OP_FMEM]  = 2; 
  op_latency[OP_FCF]   = 1; 
  op_latency[OP_FCVT]  = 4; 
  op_latency[OP_FADD]  = 2; 
  op_latency[OP_FMUL]  = 4; 
  op_latency[OP_FDIV]  = 16; 
  op_latency[OP_FCMP]  = 2; 
  op_latency[OP_FBIT]  = 2; 
  op_latency[OP_FCMO]  = 2; 
}

void init_op(Op *op) {
  op->num_src               = 0; 
  op->src[0]                = -1; 
  op->src[1]                = -1;
  op->dst                   = -1; 
  op->opcode                = 0; 
  op->is_fp                 = false;
  op->cf_type               = NOT_CF;
  op->mem_type              = NOT_MEM;
  op->write_flag             = 0;
  op->inst_size             = 0;
  op->ld_vaddr              = 0;
  op->st_vaddr              = 0;
  op->instruction_addr      = 0;
  op->branch_target         = 0;
  op->actually_taken        = 0;
  op->mem_read_size         = 0;
  op->mem_write_size        = 0;
  op->valid                 = FALSE;
  op->mem_decrement	    = true;
  op->miss_predict          = FALSE;
  op->tlb_miss              = FALSE;
  op->pte_addr              = 0;
  op->physical_addr         = 0;
  op->tlb_accessed          = FALSE;
  op->dcache_result          = 2;
  op->thread_id		    = 0;
  /* you might add more features here */ 
}


void init_op_pool(void) {
  /* initialize op pool */ 
  op_pool = new Op [1024];
  free_op_num = 1024; 
  active_op_num = 0; 
  uint32_t op_pool_entries = 0; 
  int ii;
  for (ii = 0; ii < 1023; ii++) {

    op_pool[ii].op_pool_next = &op_pool[ii+1]; 
    op_pool[ii].op_pool_id   = op_pool_entries++; 
    init_op(&op_pool[ii]); 
  }
  op_pool[ii].op_pool_next = op_pool_free_head; 
  op_pool[ii].op_pool_id   = op_pool_entries++;
  init_op(&op_pool[ii]); 
  op_pool_free_head = &op_pool[0]; 
}


Op *get_free_op(void) {
  /* return a free op from op pool */ 

  if (op_pool_free_head == NULL || (free_op_num == 1)) {
    std::cout <<"ERROR! OP_POOL SIZE is too small!! " << endl; 
    std::cout <<"please check free_op function " << endl; 
    assert(1); 
    exit(1);
  }

  free_op_num--;
  assert(free_op_num); 

  Op *new_op = op_pool_free_head; 
  op_pool_free_head = new_op->op_pool_next; 
  assert(!new_op->valid); 
  init_op(new_op);
  active_op_num++; 
  return new_op; 
}

void free_op(Op *op)
{
  free_op_num++;
  active_op_num--; 
  op->valid = FALSE; 
  op->op_pool_next = op_pool_free_head;
  op_pool_free_head = op; 
}



/*******************************************************************/
/*  Data structure */
/*******************************************************************/

typedef struct pipeline_latch_struct {
  Op *op; /* you must update this data structure. */
  bool op_valid; 
  bool used;
   /* you might add more data structures. But you should complete the above data elements */ 
}pipeline_latch; 


typedef struct Reg_element_struct{
  bool valid;
  int count;
  // data is not needed 
  /* you might add more data structures. But you should complete the above data elements */ 
}REG_element; 

REG_element register_file[HW_MAX_THREAD][NUM_REG];


/*******************************************************************/
/* These are the functions you'll have to write.  */ 
/*******************************************************************/
void FE_stage(memory_c *main_memory);
void ID_stage();
void EX_stage(); 
void MEM_stage(memory_c *main_memory); // please modify MEM_stage function argument  /** NEW-LAB2 */ 
void WB_stage(memory_c *main_memory); 
void MEM_WB_stage(); 


/*******************************************************************/
/*  These are the variables you'll have to write.  */ 
/*******************************************************************/
bool sim_end_condition = FALSE;     /* please complete the condition. */  
bool sim_end_condition_thread[HW_MAX_THREAD] = {0};

uint64_t cycle_count = 0;            /* total number of cycles */ 
uint64_t retired_instruction = 0;    /* number of retired instruction. (only correct instructions) */ 
uint64_t dcache_miss_count = 0; 
uint64_t dcache_hit_count = 0; 
uint64_t data_hazard_count = 0;  
uint64_t control_hazard_count = 0; 
uint64_t dram_row_buffer_hit_count = 0;  /* total number of dram row buffer hit */
uint64_t dram_row_buffer_miss_count = 0;   /* total number of dram row buffer hit */
uint64_t store_load_forwarding_count = 0; /* total number of store load forwarding */
uint64_t bpred_mispred_count = 0;  /* total number of branch mispredictions */
uint64_t bpred_okpred_count = 0;   /* total number of correct branch predictions */
uint64_t dtlb_hit_count = 0;       /* total number of DTLB hits */
uint64_t dtlb_miss_count = 0;      /* total number of DTLB miss */

uint64_t retired_instruction_thread[HW_MAX_THREAD] = {0};	// NEW for LAB4
uint64_t dcache_miss_count_thread[HW_MAX_THREAD] = {0}; 	// NEW for LAB4
uint64_t dcache_hit_count_thread[HW_MAX_THREAD] = {0}; 	// NEW for LAB4
uint64_t data_hazard_count_thread[HW_MAX_THREAD] = {0};  	// NEW for LAB4
uint64_t control_hazard_count_thread[HW_MAX_THREAD] = {0}; 	// NEW for LAB4
uint64_t store_load_forwarding_count_thread[HW_MAX_THREAD] = {0};	// NEW for LAB4
uint64_t bpred_mispred_count_thread[HW_MAX_THREAD] = {0};	// NEW for LAB4
uint64_t bpred_okpred_count_thread[HW_MAX_THREAD] = {0};	// NEW for LAB4
uint64_t dtlb_hit_count_thread[HW_MAX_THREAD] = {0};	// NEW for LAB4
uint64_t dtlb_miss_count_thread[HW_MAX_THREAD] = {0};	// NEW for LAB4

uint64_t dcache_pte_hit_count = 0;
uint64_t dcache_pte_miss_count = 0;
uint64_t dcache_replay_miss_count = 0;
uint64_t mem_inst = 0;
uint64_t ld_inst = 0;
uint64_t st_inst = 0;
uint64_t mshr_full = 1;
uint64_t mshr_full_pte = 1;
uint64_t wrong_cache_count = 0;

uint64_t cold_misses = 0; //For TA reference
uint64_t mem_ops_mshr = 0; // This tells us the number of memory ops in MSHR
list<unsigned long long int> oplist; //The broadcase op-ids
list<unsigned long long int>::iterator op_iterator; //Iterator for broadcast ops

int saver=0;

bpred *branchpred; // NEW-LAB3 (student need to initialize)
tlb *dtlb;        // NEW-LAB3 (student need to intialize)


/*******************************************************************/
/*  My Variables  */
/*******************************************************************/
bool store_retire=false;
int store_reg=-1;
int EX_latency_countdown = 0;
int MEM_latency_countdown = 0;
int mem_stall=-1;
bool queue_empty = false;
bool broadcast_on=false;
bool mem_latch_busy=false;
bool memory_stall = false; 
bool control_stall = false;
bool data_stall[HW_MAX_THREAD] = {0};
bool read_trace_error = false;
long long unsigned int cache_delay=0;
int was_stalled = 0;
bool TLB_replay = false;
bool TLB_miss = false;
bool br_stall[HW_MAX_THREAD] = {0};

list<Op *> MEM_WB_latch; // MEM latch for storing OPs
list<Op *>::iterator MEM_WB_latch_iterator; //MEM latches iterator

list<Op *> TLB_replay_queue; // MEM latch for storing OPs
list<Op *>::iterator TLB_replay_queue_iterator; //MEM latches iterator

pipeline_latch *MEM_latch;  
pipeline_latch *EX_latch;
pipeline_latch *ID_latch;
pipeline_latch *FE_latch[HW_MAX_THREAD];
UINT64 ld_st_buffer[LD_ST_BUFFER_SIZE]; 
UINT64 next_pc; 
Cache *data_cache;  // NEW-LAB2 

//memory_c main_memory();   // NEW-LAB2 -- BUG COMMENT

/*******************************************************************/
/*  Print messages  */
/*******************************************************************/
void print_stats() {
  std::ofstream out((KNOB(KNOB_OUTPUT_FILE)->getValue()).c_str());
  /* Do not modify this function. This messages will be used for grading */ 

  out << "Total instruction: " << retired_instruction << endl;
  out << "Total cycles: " << cycle_count << endl;
  float ipc = (cycle_count ? ((float)retired_instruction/(float)cycle_count): 0 );
  out << "Total IPC: " << ipc << endl;
  out << "Total D-cache miss: " << dcache_miss_count << endl;
  out << "Total D-cache hit: " << dcache_hit_count << endl;
  out << "Total data hazard: " << data_hazard_count << endl;
  out << "Total control hazard : " << control_hazard_count << endl;
  out << "Total DRAM ROW BUFFER Hit: " << dram_row_buffer_hit_count << endl;
  out << "Total DRAM ROW BUFFER Miss: "<< dram_row_buffer_miss_count << endl;
  out << "Total Store-load forwarding: " << store_load_forwarding_count << endl;
  out << "Total Branch Predictor Mispredictions: " << bpred_mispred_count << endl;
  out << "Total Branch Predictor OK predictions: " << bpred_okpred_count << endl;
  out << "Total DTLB Hit: " << dtlb_hit_count << endl;
  out << "Total DTLB Miss: " << dtlb_miss_count << endl;

  out << endl << endl << endl;
  
  for (int ii = 0; ii < (KNOB(KNOB_RUN_THREAD_NUM)->getValue()); ii++ ) { 
    out << "THREAD instruction: " << retired_instruction_thread[ii] << " Thread id: " << ii << endl; 
    float thread_ipc = (cycle_count ? ((float)retired_instruction_thread[ii]/(float)cycle_count): 0 );
    out << "THREAD IPC: " << thread_ipc << endl; 
    out << "THREAD D-cache miss: " << dcache_miss_count_thread[ii] << " Thread id: " << ii << endl; 
    out << "THREAD D-cache hit: " << dcache_hit_count_thread[ii] << " Thread id: " << ii << endl;
    out << "THREAD data hazard: " << data_hazard_count_thread[ii] << " Thread id: " << ii <<   endl;
    out << "THREAD control hazard : " << control_hazard_count_thread[ii] << " Thread id: " << ii << endl;   
    out << "THREAD Store-load forwarding: " << store_load_forwarding_count_thread[ii] << " Thread id: " << ii << endl; 
    out << "THREAD Branch Predictor Mispredictions: " << bpred_mispred_count_thread[ii] << " Thread id: " << ii << endl;   
    out << "THREAD Branch Predictor OK predictions: " << bpred_okpred_count_thread[ii] << " Thread id: " << ii << endl;   
    out << "THREAD DTLB Hit: " << dtlb_hit_count_thread[ii] << " Thread id: " << ii << endl; 
    out << "THREAD DTLB Miss: " << dtlb_miss_count_thread[ii] << " Thread id: " << ii << endl; 
  }
  out.close();
}



/*******************************************************************/
/*  Support Functions  */ 
/*******************************************************************/

/** This function is updated for Lab #4 NEW-LAB4  **/ 

int get_op(Op *op)
{
  static UINT64 unique_count = 0; 
	  static UINT64 fetch_arbiter; 
	  
	  Trace_op trace_op; 
	  bool success = FALSE; 
	  // read trace 
	  // fill out op info 
	  // return FALSE if the end of trace
	  int read_size;
	  int fetch_id = -1; 
//	  bool br_stall_fail = false;
//	  bool br_stall_fail_2 = false;

	  // read trace 
	  // fill out op info 
	  // return FALSE if the end of trace 
	    fetch_id = (fetch_arbiter++%(KNOB(KNOB_RUN_THREAD_NUM)->getValue()));

//	    if ( data_stall[fetch_id]) {
//	    br_stall_fail_2 = true; 
//	    continue; 
//	    }

	    if (br_stall[fetch_id]) {
			control_hazard_count_thread[fetch_id]++;
			control_hazard_count++;
	//    br_stall_fail = true;  
	    }

	    read_size = gzread(g_stream[fetch_id], &trace_op, sizeof(Trace_op));
	    success = read_size>0;
	    if(read_size!=sizeof(Trace_op) && read_size>0) {
	      printf( "ERROR!! gzread reads corrupted op! @cycle:%d\n", cycle_count);
	      success = false;
	    }

	    /* copy trace structure to op */ 
	    if (success) { 
		copy_trace_op(&trace_op, op); 

		op->inst_id  = unique_count++;
		op->valid    = TRUE; 
		op->thread_id = fetch_id; 
		    return success;  // get op so return 
	    }
	    
		// if not success and go to another trace. 
//	if (br_stall_fail) return -1;
//	if (br_stall_fail_2) return -2; 
	return success; 
}

	/* return op execution cycle latency */ 

	int get_op_latency (Op *op) 
	{
	  assert (op->opcode < NUM_OP_TYPE); 
	  return op_latency[op->opcode];
	}

	/* Print out all the register values */ 
	void dump_reg() {
	for(int j=0; j < HW_MAX_THREAD; j++){
	  for (int ii = 0; ii < NUM_REG; ii++) {
	    std::cout << cycle_count << ":register[" << ii  << "]: V:" << register_file[j][ii].valid << endl; 
	  }
	}
	}

	void print_pipeline() {
	  int flag = 0;	
	  std::cout << "--------------------------------------------" << endl; 
	  std::cout <<"cycle count : " << dec << cycle_count << " retired_instruction : " << retired_instruction << endl; 
	  std::cout << (int)cycle_count << " FE: " ;
	for(int i=0; i< HW_MAX_THREAD; i++){
	  if (FE_latch[i]->op_valid) {
	    Op *op = FE_latch[i]->op; 
	    cout << (int)op->inst_id<<" ";
	  }
	  else {
	    cout <<"### ";
	  }
	}
	  std::cout << " ID: " ;
	  if (ID_latch->op_valid) {
	    Op *op = ID_latch->op; 
	    cout << (int)op->inst_id ;
	  }
	  else {
	    cout <<"####";
	  }
	  std::cout << " EX: " ;
	  if (EX_latch->op_valid ) {
	    Op *op = EX_latch->op; 
	    cout << (int)op->inst_id ;
	  }
	  else {
	    cout <<"####";
	  }

	  if (TLB_replay_queue.size()!=0) {
	    cout << " MEM_REPLAY: ";
	    for(TLB_replay_queue_iterator=TLB_replay_queue.begin(); TLB_replay_queue_iterator!=TLB_replay_queue.end(); ++TLB_replay_queue_iterator) 
	      cout << (*TLB_replay_queue_iterator)->inst_id ;
	  }
	 
	  if((int)oplist.size()!=0){
		  for(op_iterator=oplist.begin(); op_iterator!=oplist.end(); ++op_iterator)
		   {
			   std::cout << " MEM_MSHR: " ;
			   cout << *op_iterator ;
			   flag=1;
		   }
		   oplist.clear();
		}
	  if (MEM_latch->op_valid) {
		std::cout << " MEM_PIPE: " ;
	    Op *op = MEM_latch->op; 
	    cout << (int)op->inst_id ;
	    flag=1;
	  }
	  if(flag==0){
		std::cout << " MEM: " ;
	    cout <<"####";
	  }
	  cout << endl; 
	  //  dump_reg();   
	  std::cout << "--------------------------------------------" << endl; 
	}

	/** This function is updated for Lab #4 NEW-LAB4  **/ 
	void print_heartbeat()
	{
	  static uint64_t last_cycle_thread[HW_MAX_THREAD] ;
	  static uint64_t last_inst_count_thread[HW_MAX_THREAD]; 
	  
	  for (int ii = 0; ii < (KNOB(KNOB_RUN_THREAD_NUM)->getValue()); ii++ ) { 

	    float temp_ipc = float(retired_instruction_thread[ii] - last_inst_count_thread[ii]) /(float)(cycle_count-last_cycle_thread[ii]) ;
	    float ipc = float(retired_instruction_thread[ii]) /(float)(cycle_count) ;
	    /* Do not modify this function. This messages will be used for grading */ 
	    cout <<"**Heartbeat** cycle_count: " << cycle_count << " inst:" << retired_instruction_thread[ii] << " IPC: " << temp_ipc << " Overall IPC: " << ipc << " thread_id " << ii << endl; 
	    last_cycle_thread[ii] = cycle_count;
	    last_inst_count_thread[ii] = retired_instruction_thread[ii]; 
	  }
	}
	/*******************************************************************/
	/*                                                                 */
	/*******************************************************************/

	bool run_a_cycle(memory_c *main_memory){   // please modify run_a_cycle function argument  /** NEW-LAB2 */ 

	  for (;;) { 
	    if (((KNOB(KNOB_MAX_SIM_COUNT)->getValue() && (cycle_count >= KNOB(KNOB_MAX_SIM_COUNT)->getValue())) || 
	      (KNOB(KNOB_MAX_INST_COUNT)->getValue() && (retired_instruction >= KNOB(KNOB_MAX_INST_COUNT)->getValue())) ||  (sim_end_condition))) { 
		// please complete sim_end_condition 
		// finish the simulation
		print_heartbeat(); 
		print_stats();
		return TRUE;
	    }	
	    cycle_count++; 
	    if (!(cycle_count%5000)) {
	      print_heartbeat(); 
	    }
	    MEM_WB_stage(); 
	    main_memory->run_a_cycle();          // *NEW-LAB2 
	    
	    WB_stage(main_memory); 
	    MEM_stage(main_memory);  // please modify MEM_stage function argument  /** NEW-LAB2 */ 
	    EX_stage();
	    ID_stage();
	    FE_stage(main_memory); 
	    if (KNOB(KNOB_PRINT_PIPE_FREQ)->getValue() && !(cycle_count%KNOB(KNOB_PRINT_PIPE_FREQ)->getValue())) print_pipeline();
	  }
	  return TRUE; 
	}


	/*******************************************************************/
	/* Complete the following fuctions.  */
	/* You can add new data structures and also new elements to Op, Pipeline_latch data structure */ 
	/*******************************************************************/

	void init_structures(memory_c *main_memory) {
	  init_op_pool(); 
	  init_op_latency();
	  main_memory->init_mem(); 
	  init_dcache(); 
	  init_latches();
	  init_regfile();

	  // Initialize branch predictor and TLB
	  branchpred =  bpred_new((bpred_type)KNOB(KNOB_BPRED_TYPE)->getValue(), KNOB(KNOB_BPRED_HIST_LEN)->getValue()); 
	  dtlb = tlb_new( (int)KNOB(KNOB_TLB_ENTRIES)->getValue() ); 

	}

	void MEM_WB_stage(){
	  for(MEM_WB_latch_iterator=MEM_WB_latch.begin(); MEM_WB_latch_iterator!=MEM_WB_latch.end(); ++MEM_WB_latch_iterator) {
	    Op* op = *MEM_WB_latch_iterator;
	    if((op->dst!= -1) && (op->mem_decrement==true)) {
	      register_file[op->thread_id][op->dst].count--;
	      if( register_file[op->thread_id][op->dst].count==0 ) {
		register_file[op->thread_id][op->dst].valid = true;
		if(data_stall[op->thread_id])
		  data_stall[op->thread_id] = false;
		} 
	      }	 
	      if(mem_ops_mshr>0)
		mem_ops_mshr--;
		
	    if(op->valid)
	    {
		    retired_instruction++; 
		    retired_instruction_thread[op->thread_id]++;
	     	    free_op(op);
	    }
	    else
	      printf("ERROR!!! Why free an invalid op?!\n");
	  }
	  MEM_WB_latch.clear();
	}

	void WB_stage(memory_c *main_memory) {
	  if( MEM_latch->op_valid == true ) {
	    //if no branch predictor
	    if((MEM_latch->op->opcode == OP_CF) && br_stall[MEM_latch->op->thread_id] && (KNOB(KNOB_USE_BPRED)->getValue()==0))
	      br_stall[MEM_latch->op->thread_id] = false;

	    //branch predictor
	    if(MEM_latch->op->miss_predict && (KNOB(KNOB_USE_BPRED)->getValue()==1)) {
	      MEM_latch->op->miss_predict = false;
	      br_stall[MEM_latch->op->thread_id] = false;
	    }

	    if( MEM_latch->op->dst!= -1 ) {
	      if(register_file[MEM_latch->op->thread_id][ MEM_latch->op->dst ].count>0)
	      register_file[MEM_latch->op->thread_id][ MEM_latch->op->dst ].count--;

	      list<m_mshr_entry_s*>::iterator cii;
	      for (cii= main_memory->m_mshr.begin() ; cii !=main_memory->m_mshr.end(); cii++) {
		m_mshr_entry_s* entry=(*cii);
		list<Op *>::iterator cii_memop;

		for(cii_memop=entry->req_ops.begin() ; cii_memop !=entry->req_ops.end(); cii_memop++) {
		  Op *m_op=(*cii_memop);
		  if(m_op->dst==MEM_latch->op->dst) {
		    if((register_file[MEM_latch->op->thread_id][ MEM_latch->op->dst ].count>0) && (MEM_latch->op->inst_id > m_op->inst_id) && (m_op->mem_decrement==true)) {
		      register_file[m_op->thread_id][ m_op->dst ].count--;
		      m_op->mem_decrement=false;
		    }
		  }
		}
	      }
		
	      if( register_file[MEM_latch->op->thread_id][ MEM_latch->op->dst ].count==0 ) {
		register_file[MEM_latch->op->thread_id][ MEM_latch->op->dst ].valid = true;
		if(data_stall[MEM_latch->op->thread_id])
		  data_stall[MEM_latch->op->thread_id] = false;
	      } 
	    }
		
	    if(MEM_latch->op->valid)
	    {
		    retired_instruction++;
		    retired_instruction_thread[MEM_latch->op->thread_id]++;
	      	    free_op(MEM_latch->op);
	    }
	    else
	      printf("ERROR!!! Why free and invalid op?!\n");
	  }
	}

	void MEM_stage(memory_c *main_memory) { 

	  MEM_latch->op_valid=false;
	  MEM_latch->op=NULL;
	  ADDRINT dcache_address;
	  uint64_t local_pfn = 0;
	  uint64_t local_vpn = 0;
	  uint64_t pte_addr;
	  bool tlb_result;
	  unsigned int vmem_page_size = KNOB(KNOB_VMEM_PAGE_SIZE)->getValue();
	  Op* op_ptr;
	  bool ldst_inst;
	  bool enable_vmem = KNOB(KNOB_ENABLE_VMEM)->getValue();

	  if( (TLB_replay_queue.size()!=0) && (cache_delay==0) )
	    TLB_replay = true; 

	  if(TLB_replay && !enable_vmem)
	    printf("ERROR!! Why disable virtual memory but still have TLB replay?!\n");    

	  //TLB replay
	  if(TLB_replay) {
	    if((TLB_replay_queue.size()==1) && cache_delay==1) 
	      mem_stall = -1;
	    else
	      mem_stall = 1;

	    if(cache_delay==0) 
	      cache_delay = KNOB(KNOB_DCACHE_LATENCY)->getValue()-1; //As the next if statement will decrement it
	    else
	      cache_delay--;
	    
	    if(cache_delay!=0)
	      return;

	    //Get op
	    op_ptr = TLB_replay_queue.front();
	    dcache_address = op_ptr->ld_vaddr;
	    if( op_ptr->mem_type==MEM_ST )
	      dcache_address = op_ptr->st_vaddr;
	    else
	      dcache_address = op_ptr->ld_vaddr;

	    local_vpn =  dcache_address/vmem_page_size;
	    tlb_result = tlb_access (dtlb, local_vpn, op_ptr->thread_id, &local_pfn);

	    //TLB hit!!
	    if(tlb_result) {
	      op_ptr->tlb_miss = false;
	      dcache_address = (local_pfn*vmem_page_size) + (dcache_address%vmem_page_size);  //Generate physical address
	      TLB_miss = false;
	    }  
	    else {  
	      TLB_miss = true;
	      pte_addr = vmem_get_pteaddr(local_vpn, op_ptr->thread_id);
	      if(dcache_access(pte_addr)) { //found in cache,
		local_pfn = vmem_vpn_to_pfn(local_vpn, op_ptr->thread_id);
		tlb_install(dtlb, local_vpn, op_ptr->thread_id, local_pfn);
		mem_stall = 1;
		dcache_hit_count_thread[op_ptr->thread_id]++;
		dcache_hit_count++;
		return;
	      }
	      else 
		printf("ERROR!! TLB replay should not have TLB miss agian!!!\n");
	    }


	    if(op_ptr->mem_type==MEM_LD) {
	      if(dcache_access(dcache_address)) {
		if(was_stalled==0)
		  dcache_hit_count_thread[op_ptr->thread_id]++;
		  dcache_hit_count++;
		was_stalled = 0;
		MEM_latch->op_valid=true;
		MEM_latch->op = op_ptr;
	      }
	      else {
		MEM_latch->op=NULL;
		MEM_latch->op_valid=false;
		op_ptr->physical_addr = dcache_address;

		if(main_memory->store_load_forwarding(op_ptr)) {
		  MEM_latch->op_valid=true;
		  MEM_latch->op = op_ptr;
		  store_load_forwarding_count++;
		  store_load_forwarding_count_thread[MEM_latch->op->thread_id]++;
		  dcache_miss_count_thread[MEM_latch->op->thread_id]++;
		  dcache_miss_count++;
		  dcache_replay_miss_count++;
		  was_stalled = 0;
		}
		else if(main_memory->check_piggyback(op_ptr)) {
		  was_stalled = 0;
		  dcache_miss_count_thread[op_ptr->thread_id]++;
		  dcache_miss_count++;
		  dcache_replay_miss_count++;
		  mem_ops_mshr++;
		}
		else if(main_memory->insert_mshr(op_ptr)) {
		  was_stalled = 0;
		  dcache_miss_count_thread[op_ptr->thread_id]++;
		  dcache_miss_count++;
		  dcache_replay_miss_count++;
		  mem_ops_mshr++;
		}
		else {  //MSHR full
		  mem_stall = 1;
		  was_stalled = 1;
		  mshr_full++;
		  return;
		}
	      }
	    }
	    else {   //if(EX_latch->op->mem_type==MEM_ST) {   
	      if(dcache_access(dcache_address)) {
		if(was_stalled==0)
		  dcache_hit_count_thread[op_ptr->thread_id]++;
		  dcache_hit_count++;
		was_stalled = 0;
		MEM_latch->op_valid=true;
		MEM_latch->op = op_ptr;
	      }
	      else {
		op_ptr->physical_addr = dcache_address;
		MEM_latch->op=NULL;
		MEM_latch->op_valid=false;
		if(main_memory->store_load_forwarding(op_ptr)) {
		  MEM_latch->op_valid=true;
		  MEM_latch->op = op_ptr;
		  was_stalled = 0;
		  dcache_miss_count_thread[op_ptr->thread_id]++;
		  dcache_miss_count++;
		}
		else if(main_memory->check_piggyback(op_ptr)) {
		  was_stalled = 0;
		  dcache_miss_count_thread[op_ptr->thread_id]++;
		  dcache_miss_count++;
		  mem_ops_mshr++;
		}
		else if(main_memory->insert_mshr(op_ptr)) {
		  was_stalled = 0;
		  dcache_miss_count_thread[op_ptr->thread_id]++;
		  dcache_miss_count++;
		  mem_ops_mshr++;
		}
		else {  //MSHR full
		  mem_stall = 1;
		  was_stalled = 1;
		  mshr_full++;
		  return;
		}
	      }
	    }

	    //remove entry from TLB replay queue
	    TLB_replay_queue.pop_front();
	    if(TLB_replay_queue.size()==0) 
	      TLB_replay = false;
	    return;
	  }  

	  if(EX_latch->op_valid) {
	    ldst_inst = (EX_latch->op->mem_type==MEM_LD || EX_latch->op->mem_type==MEM_ST);
	    if(cache_delay==0 && ldst_inst) {
	      cache_delay = KNOB(KNOB_DCACHE_LATENCY)->getValue(); //As the next if statement will decrement it
	      mem_stall = 1;
	    }

	    if(cache_delay>0) {
	      cache_delay--;
	      mem_stall = 1;
	    }
	    else
	      mem_stall = -1;
	    

	    //Access TLB
	    dcache_address = EX_latch->op->ld_vaddr;
	    if( EX_latch->op->mem_type==MEM_ST )
	      dcache_address = EX_latch->op->st_vaddr;
	    else
	      dcache_address = EX_latch->op->ld_vaddr;
		      
	    if(enable_vmem && ldst_inst && (cache_delay==0)) {  //Virtual memory system, TLB lookup
	      local_vpn =  dcache_address/vmem_page_size;

	      tlb_result = tlb_access (dtlb, local_vpn, EX_latch->op->thread_id, &local_pfn);
	 
	      //TLB miss!!
	      if(!tlb_result) {
		if(!EX_latch->op->tlb_accessed) {
		  dtlb_miss_count++;
		  dtlb_miss_count_thread[EX_latch->op->thread_id]++;
		  EX_latch->op->tlb_accessed = true;
		}
		TLB_miss = true;
		pte_addr = vmem_get_pteaddr(local_vpn, EX_latch->op->thread_id);
		if(dcache_access(pte_addr)) { //found in cache,
		  local_pfn = vmem_vpn_to_pfn(local_vpn, EX_latch->op->thread_id);
		  tlb_install(dtlb, local_vpn, EX_latch->op->thread_id, local_pfn);
		  TLB_replay_queue.push_back(EX_latch->op);
		  mem_stall = 1;
		  dcache_hit_count_thread[EX_latch->op->thread_id]++;
		  dcache_hit_count++;
		  EX_latch->op_valid=false;
		  EX_latch->op=NULL;
		  dcache_pte_hit_count++;
		}
		else {  //Access MSHR, put PTE in cache, install TLB entry
		  MEM_latch->op = NULL;
		  MEM_latch->op_valid = false;
		  EX_latch->op->tlb_miss = true;
		  EX_latch->op->pte_addr = pte_addr;
		  dcache_pte_miss_count++;
		  if(main_memory->store_load_forwarding(EX_latch->op)) {
		    store_load_forwarding_count_thread[EX_latch->op->thread_id]++;
		    dcache_miss_count_thread[EX_latch->op->thread_id]++;
		    EX_latch->op_valid=false;
		    EX_latch->op=NULL;
		    mem_stall = 1;
		    store_load_forwarding_count++;
		    dcache_miss_count++;
		    was_stalled = 0;
		  }
		  else if(main_memory->check_piggyback(EX_latch->op)) {
		    mem_stall = 1;
		    was_stalled = 0;
		    dcache_miss_count++;
		    dcache_miss_count_thread[EX_latch->op->thread_id]++;
		    mem_ops_mshr++;
		    EX_latch->op_valid=false;
		    EX_latch->op=NULL;
		  }
		  else if(main_memory->insert_mshr(EX_latch->op)) {
		    mem_stall = 1;
		    was_stalled = 0;
		    dcache_miss_count++;
		    dcache_miss_count_thread[EX_latch->op->thread_id]++;
		    mem_ops_mshr++;
		    EX_latch->op_valid=false;
		    EX_latch->op=NULL;
		  }
		  else {
		    mshr_full_pte++;
		    mem_stall = 1;
		    was_stalled = 1;
		  }
		}
		return;
	      }
	      else {  //TLB Hit
		dcache_address = (local_pfn*vmem_page_size) + (dcache_address%vmem_page_size);  //Generate physical address
		 if(!EX_latch->op->tlb_accessed) {
		  dtlb_hit_count++;
		  dtlb_hit_count_thread[EX_latch->op->thread_id]++;
		  EX_latch->op->tlb_accessed = true;
		}
		TLB_miss =false;
	      }
	    }

	    // Operate for load instructions
	    if(EX_latch->op->mem_type==MEM_LD) {
	      if(cache_delay==0) {
		if(dcache_access(dcache_address)) {
		  if(was_stalled==0)
		    dcache_hit_count_thread[EX_latch->op->thread_id]++;
		    dcache_hit_count++;
			
		  was_stalled = 0;
		  MEM_latch->op_valid=EX_latch->op_valid;
		  MEM_latch->op = EX_latch->op;
		  EX_latch->op_valid=false;
		  EX_latch->op=NULL;
		  mem_stall = -1;
		}
		else {
		  MEM_latch->op=NULL;
		  MEM_latch->op_valid=false;
		  EX_latch->op->physical_addr = dcache_address;	
		  // put op into the dram
		  if(main_memory->store_load_forwarding(EX_latch->op)) {
		    store_load_forwarding_count_thread[EX_latch->op->thread_id]++;
		    dcache_miss_count_thread[EX_latch->op->thread_id]++;
		    MEM_latch->op_valid=EX_latch->op_valid;
		    MEM_latch->op = EX_latch->op;
		    EX_latch->op_valid=false;
		    EX_latch->op=NULL;
		    mem_stall = -1;
		    store_load_forwarding_count++;
		    dcache_miss_count++;
		    was_stalled = 0;
		  }
		  else if(main_memory->check_piggyback(EX_latch->op)) {
		    mem_stall = -1;
		    was_stalled = 0;
		    dcache_miss_count++;
		    dcache_miss_count_thread[EX_latch->op->thread_id]++;
		    mem_ops_mshr++;
		    EX_latch->op_valid=false;
		    EX_latch->op=NULL;
		  }
		  else if(main_memory->insert_mshr(EX_latch->op)) {
		    mem_stall = -1;
		    was_stalled = 0;
		    dcache_miss_count++;
		    dcache_miss_count_thread[EX_latch->op->thread_id]++;
		    mem_ops_mshr++;
		    EX_latch->op_valid=false;
		    EX_latch->op=NULL;
		  }
		  else {
		    mem_stall = 1;
		    was_stalled = 1;
		    mshr_full++;
		  }
		}
	      }
	    }
	    else if(EX_latch->op->mem_type==MEM_ST) {	
	      if(cache_delay==0) {
		if(dcache_access(dcache_address)) {
		  if(was_stalled==0)
		    dcache_hit_count_thread[EX_latch->op->thread_id]++;
		    dcache_hit_count++;
					
		  was_stalled = 0;
		  MEM_latch->op_valid=EX_latch->op_valid;
		  MEM_latch->op = EX_latch->op;
		  EX_latch->op_valid=false;
		  EX_latch->op=NULL;
		  mem_stall = -1;		
		}
		else {
		  MEM_latch->op=NULL;
		  MEM_latch->op_valid=false;
		  EX_latch->op->physical_addr = dcache_address;

		  if(main_memory->store_load_forwarding(EX_latch->op)) {
		    dcache_miss_count_thread[EX_latch->op->thread_id]++;
		    MEM_latch->op_valid=EX_latch->op_valid;
		    MEM_latch->op = EX_latch->op;
		    EX_latch->op_valid=false;
		    EX_latch->op=NULL;
		    mem_stall = -1;
		    was_stalled = 0;
		    dcache_miss_count++;
		  }
		  else if(main_memory->check_piggyback(EX_latch->op)) {
		    mem_stall = -1;
		    was_stalled = 0;
		    dcache_miss_count++;
		    dcache_miss_count_thread[EX_latch->op->thread_id]++;
		    mem_ops_mshr++;
		    EX_latch->op_valid=false;
		    EX_latch->op=NULL;
		  }
		  else if(main_memory->insert_mshr(EX_latch->op)) {
		    mem_stall = -1;
		    was_stalled = 0;
		    dcache_miss_count++;
		    dcache_miss_count_thread[EX_latch->op->thread_id]++;
		    mem_ops_mshr++;
		    EX_latch->op_valid=false;
		    EX_latch->op=NULL;
		  }
		  else {
		    mem_stall = 1;
		    was_stalled = 1;
		    mshr_full++;
		  }
		}
	      } 
	    }
	    else {	//Operate for non-memory instructions
	      MEM_latch->op = EX_latch->op;
	      MEM_latch->op_valid = EX_latch->op_valid;
	      mem_stall = -1;
	      was_stalled = 0;
	    }
	  }
	  else {
	    if(TLB_miss) {
	      MEM_latch->op = EX_latch->op;
	      MEM_latch->op_valid = EX_latch->op_valid;
	      mem_stall = 1;
	      was_stalled = 0;
	    }
	    else {
	      MEM_latch->op = EX_latch->op;
	      MEM_latch->op_valid = EX_latch->op_valid;
	      mem_stall = -1;
	      was_stalled = 0;
	    }
	  }
	}
	void EX_stage() {
	  if( ID_latch->op_valid ) {
	    if(mem_stall == 1) 
	      EX_latency_countdown = -1;
	    else if(EX_latency_countdown >0 )
	      EX_latency_countdown--;
	    else
	      EX_latency_countdown =  get_op_latency(ID_latch->op)-1;
	     
	    if( EX_latency_countdown==0 ) {
	      EX_latch->op = ID_latch->op;
	      EX_latch->op_valid = ID_latch->op_valid;
	    }
	    else if(EX_latency_countdown==-1) {
	      EX_latch->op = EX_latch->op;
	      EX_latch->op_valid = EX_latch->op_valid;
	    }
	    else {
	      EX_latch->op = NULL;
	      EX_latch->op_valid = false;   	
	    }
	  }
	  else {
	    if(mem_stall == 1) {
	      EX_latch->op = EX_latch->op;
	      EX_latch->op_valid = EX_latch->op_valid;
	    }
	    else {
	      EX_latch->op = ID_latch->op;
	      EX_latch->op_valid = ID_latch->op_valid;
	    }
	  }
	}

void ID_stage() {

	  static UINT64 scheduler_arbiter;
	int jj, scheduler_id;
	for ( jj = 0; jj < (KNOB(KNOB_RUN_THREAD_NUM)->getValue()); jj++) {
    scheduler_id = (scheduler_arbiter++%(KNOB(KNOB_RUN_THREAD_NUM)->getValue()));


  if( FE_latch[scheduler_id]->op_valid) {

if(saver == 1000)
{
	saver = 0;
      ID_latch->op = FE_latch[scheduler_id]->op;  
      ID_latch->op_valid = FE_latch[scheduler_id]->op_valid;
      if( FE_latch[scheduler_id]->op->dst != -1 ) {
        register_file[FE_latch[scheduler_id]->op->thread_id][ FE_latch[scheduler_id]->op->dst ].valid = false; 
        register_file[FE_latch[scheduler_id]->op->thread_id][ FE_latch[scheduler_id]->op->dst ].count++;
      }
	FE_latch[scheduler_id]->op_valid = false;
	FE_latch[scheduler_id]->op = NULL;
	return;
}

    /* Checking for any source data hazard */
    if( EX_latency_countdown==0) {
      /* Checking for any source data hazard */   
      for (int ii = 0; ii < FE_latch[scheduler_id]->op->num_src; ii++) {
        if( register_file[FE_latch[scheduler_id]->op->thread_id][ FE_latch[scheduler_id]->op->src[ii] ].valid == false ) {
	  data_hazard_count_thread[FE_latch[scheduler_id]->op->thread_id]++;
          data_hazard_count++;  
          data_stall[FE_latch[scheduler_id]->op->thread_id] = true;
		break;
        }
      }
 
      if( FE_latch[scheduler_id]->op->opcode == OP_CF && !KNOB(KNOB_USE_BPRED)->getValue()) {
	control_hazard_count_thread[FE_latch[scheduler_id]->op->thread_id]++;
        control_hazard_count++;
        br_stall[FE_latch[scheduler_id]->op->thread_id] = true;
      }      
    }

    /* If no data hazard, then contiune */
    if(EX_latency_countdown!=0) {				//execution stall
      ID_latch->op = ID_latch->op;  
      ID_latch->op_valid = ID_latch->op_valid; 
	break; 
    }
    else if( !data_stall[FE_latch[scheduler_id]->op->thread_id] ) {      //no data stall
      ID_latch->op = FE_latch[scheduler_id]->op;  
      ID_latch->op_valid = FE_latch[scheduler_id]->op_valid;
      if( FE_latch[scheduler_id]->op->dst != -1 ) {
        register_file[FE_latch[scheduler_id]->op->thread_id][ FE_latch[scheduler_id]->op->dst ].valid = false; 
        register_file[FE_latch[scheduler_id]->op->thread_id][ FE_latch[scheduler_id]->op->dst ].count++;
      }
	FE_latch[scheduler_id]->op_valid = false;
	FE_latch[scheduler_id]->op = NULL;
	break;
    }
    else {
     // ID_latch->op = NULL;  
     // ID_latch->op_valid = false; 
	continue;
    }
  }
  else {
    if(EX_latency_countdown!=0) {
      ID_latch->op = ID_latch->op;
      ID_latch->op_valid = ID_latch->op_valid;
	break;
    }
   // else{							//TODO
      //ID_latch->op = FE_latch[scheduler_id]->op;  
      //ID_latch->op_valid = FE_latch[scheduler_id]->op_valid;
	//FE_latch[scheduler_id]->used = true;
	//break;
    //}
  } 
}
if(jj == (KNOB(KNOB_RUN_THREAD_NUM)->getValue()))
{
	saver++;
     ID_latch->op = NULL;  
     ID_latch->op_valid = false; 
}
}


void FE_stage(memory_c *main_memory) {

  for(int i=0; i < KNOB(KNOB_RUN_THREAD_NUM)->getValue(); i++){

    if(EX_latency_countdown!=0) {
      FE_latch[i]->op = FE_latch[i]->op;
      FE_latch[i]->op_valid = FE_latch[i]->op_valid;
	continue;
    }
	if(!FE_latch[i]->op_valid)
{
   Op *op = get_free_op();

    int result_get_op = get_op(op); 
 
    if( result_get_op > 0 ){  
      FE_latch[i]->op = op;
      FE_latch[i]->op_valid = true;


    if(op->mem_type==MEM_LD) {
      mem_inst++;
      ld_inst++;
    }
    else if(op->mem_type==MEM_ST) {
      mem_inst++;
      st_inst++;
    }
    
//      op->thread_id = 0;    //change this for SMT system
      op->miss_predict = false;
      if((op->opcode == OP_CF) && (op->cf_type == CF_CBR) && KNOB(KNOB_USE_BPRED)->getValue() ) {

        int bpred_result = bpred_access(branchpred, op->instruction_addr, op->thread_id);
       
        bpred_update(branchpred, op->instruction_addr, bpred_result, op->actually_taken, op->thread_id); 
        
        if(bpred_result!=op->actually_taken) {
          op->miss_predict = true;
          bpred_mispred_count++;
	  bpred_mispred_count_thread[op->thread_id]++;
	  br_stall[op->thread_id] = true;
        }
        else 
          bpred_okpred_count++;
	  bpred_okpred_count_thread[op->thread_id]++; 
      }
    }
//    else if ( result_get_op == -1){	
//    		FE_latch[i]->op = NULL;
//    		FE_latch[i]->op_valid = false;
//		free_op(op);    
//	}
//	else if(result_get_op == -2){
//		FE_latch[i]->op = FE_latch[i]->op;
//    		FE_latch[i]->op_valid = FE_latch[i]->op_valid;
//cout<<"data hazard\n";
//		free_op(op);
//	} 
    else {
      if((TLB_replay_queue.size()==0)&&(FE_latch[i]->op_valid==false) && (ID_latch->op_valid==false) && (EX_latch->op_valid==false) && (MEM_latch->op_valid==false) && (main_memory->mshr_ops()==0) && (MEM_WB_latch.size()==0) ) // mem_ops_mshr==0 )
    	  sim_end_condition_thread[i] = true;
    	FE_latch[i]->op = NULL;
      FE_latch[i]->op_valid = false;  
      free_op(op);
    }
}
//    next_pc = pc + op->inst_size;  // you need this code for building a branch predictor    
  }
int k;
for(k=0; k<KNOB(KNOB_RUN_THREAD_NUM)->getValue(); k++)
{
if(!sim_end_condition_thread[k])
	break;
}
if(k==KNOB(KNOB_RUN_THREAD_NUM)->getValue())
	sim_end_condition = true;

}

void init_dcache() {
  data_cache = (Cache *)malloc(sizeof(Cache));
  unsigned int  cache_size = KNOB(KNOB_DCACHE_SIZE)->getValue();
  unsigned int assoc = KNOB(KNOB_DCACHE_WAY)->getValue();
  int block_size = KNOB(KNOB_BLOCK_SIZE)->getValue();
  char dc[]="data_cache";
  cache_init (data_cache, cache_size, block_size, assoc, dc);
}


void  init_latches() {
  MEM_latch = new pipeline_latch();
  EX_latch = new pipeline_latch();
  ID_latch = new pipeline_latch();
for(int i=0; i<HW_MAX_THREAD; i++)
  FE_latch[i] = new pipeline_latch();

  MEM_latch->op = NULL;
  EX_latch->op = NULL;
  ID_latch->op = NULL;
for(int i=0; i<HW_MAX_THREAD; i++)
  FE_latch[i]->op = NULL;
   
  MEM_latch->op_valid = false;
  EX_latch->op_valid = false;
  ID_latch->op_valid = false;
for(int i=0; i<HW_MAX_THREAD; i++)
  FE_latch[i]->op_valid = false;

  MEM_latch->used = false;
  EX_latch->used = false;
  ID_latch->used = false;
for(int i=0; i<HW_MAX_THREAD; i++)
  FE_latch[i]->used = false;
}

void init_regfile() {
for(int j=0; j < HW_MAX_THREAD; j++){
  for (int ii = 0; ii < NUM_REG; ii++) {
    register_file[j][ii].valid = true;
    register_file[j][ii].count = 0;
  }
}
}



bool icache_access(ADDRINT addr) {  
  bool hit = FALSE; 

  if (KNOB(KNOB_PERFECT_ICACHE)->getValue()) 
    hit = TRUE; 

  return hit; 
}

bool dcache_access(ADDRINT addr) { 
  bool hit = false;

  if (KNOB(KNOB_PERFECT_DCACHE)->getValue())
    hit = TRUE;
  else
    hit = cache_access(data_cache, addr); 

  return hit; 
}

 
void dcache_insert(ADDRINT addr) {  
  cache_insert(data_cache, addr) ;   
} 

void broadcast_rdy_op(Op* op) { 
  MEM_WB_latch.push_back(op);
  oplist.push_back(op->inst_id);
}      

void broadcast_tlb_rdy_op(Op* op) {
  TLB_replay_queue.push_back(op);
}

void tlb_install_from_mem(Op* op) {
  uint64_t local_pfn;
  uint64_t local_vpn;
  unsigned int vmem_page_size = KNOB(KNOB_VMEM_PAGE_SIZE)->getValue();
  ADDRINT dcache_address;

  dcache_address = op->ld_vaddr;
  if( op->mem_type==MEM_ST )
    dcache_address = op->st_vaddr;
  else
    dcache_address = op->ld_vaddr;

  local_vpn =  dcache_address/vmem_page_size;

  local_pfn = vmem_vpn_to_pfn(local_vpn, op->thread_id);
  tlb_install(dtlb, local_vpn, op->thread_id, local_pfn);
}


int64_t get_dram_row_id(ADDRINT addr) { 
  int64_t row_id;
  
  row_id = addr/(KNOB(KNOB_DRAM_PAGE_SIZE)->getValue()*1024);

  return row_id;
}  

int get_dram_bank_id(ADDRINT addr) {  
  int bank_id;

  return bank_id = (addr/(KNOB(KNOB_DRAM_PAGE_SIZE)->getValue()*1024))%(KNOB(KNOB_DRAM_BANK_NUM)->getValue()); 
}  


