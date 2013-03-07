/**********************************************************************************************
 * File         : all_knob.cc
 * Description  : knob template 
 * This file comes from MacSim Simulator 
 * All knobs have to be added in this file 
 *********************************************************************************************/


#include "all_knobs.h"

#include <string>

all_knobs_c::all_knobs_c() {
	KNOB_MAX_SIM_COUNT = new KnobTemplate< unsigned long long > ("max_sim_count", 0);
	KNOB_MAX_INST_COUNT = new KnobTemplate< unsigned long long > ("max_inst_count", 0);
	KNOB_OUTPUT_FILE = new KnobTemplate< string > ("output_file", "sim.out");
	KNOB_PRINT_INST = new KnobTemplate< bool > ("print_inst", 0);
	KNOB_PERFECT_ICACHE = new KnobTemplate< bool > ("perfect_icache", 1);
	KNOB_PERFECT_DCACHE = new KnobTemplate< bool > ("perfect_dcache", 0);
	KNOB_PERFECT_BR = new KnobTemplate< bool > ("perfect_br", 1);
	KNOB_PERFECT_L2 = new KnobTemplate< bool > ("perfect_l2", 1);
	KNOB_READ_TRACE = new KnobTemplate< bool > ("read_trace", 1);
	KNOB_TRACE_FILE = new KnobTemplate< string > ("trace_file", "trace.pzip");
	KNOB_ISSUE_WIDTH = new KnobTemplate< unsigned > ("issue_width", 1);
	KNOB_ICACHE_LATENCY = new KnobTemplate< unsigned > ("icache_latency", 1);
	KNOB_DCACHE_LATENCY = new KnobTemplate< unsigned > ("dcache_latency", 5);
	KNOB_MEM_LATENCY = new KnobTemplate< unsigned > ("mem_latency", 1);
	KNOB_L2CACHE_LATENCY = new KnobTemplate< unsigned > ("l2cache_latency", 1);
	KNOB_PRINT_PIPE_FREQ = new KnobTemplate< unsigned long long > ("print_pipe_freq", 0);
	
	
	/* lab2 additional Knobs */ 
	
	
	KNOB_DCACHE_SIZE  =  new KnobTemplate< unsigned > ("dcache_size", 512);
	KNOB_DCACHE_WAY =  new KnobTemplate< unsigned > ("dcache_way", 4); 
	KNOB_BLOCK_SIZE = new KnobTemplate< unsigned > ("block_size", 64); 
	KNOB_DCACHE_FIXED_PENALTY =  new KnobTemplate< unsigned > ("dcache_fixed_penalty", 200); 
	KNOB_MEM_LATENCY_ROW_HIT =  new KnobTemplate< unsigned > ("mem_latency_row_hit", 100); 
	KNOB_MEM_LATENCY_ROW_MISS  =  new KnobTemplate< unsigned > ("mem_latency_row_miss", 200); 
	KNOB_MSHR_SIZE  =  new KnobTemplate< unsigned > ("mshr_size", 4); 
	KNOB_DRAM_BANK_NUM =  new KnobTemplate< unsigned >  ("dram_bank_num", 4);
	KNOB_DRAM_PAGE_SIZE  =  new KnobTemplate< unsigned > ("dram_page_size", 2); 
	KNOB_PRINT_MEM_DEBUG = new KnobTemplate< bool > ("print_mem_debug", 0);


	/* lab3 additional knobs  */
	KNOB_USE_BPRED        = new KnobTemplate< bool > ("use_bpred", 1);
	KNOB_BPRED_TYPE       = new KnobTemplate< unsigned > ("bpred_type", 3);
	KNOB_BPRED_HIST_LEN   = new KnobTemplate< unsigned > ("bpred_hist_len", 12);

	KNOB_ENABLE_VMEM      = new KnobTemplate< bool > ("enable_vmem", 0);
	KNOB_TLB_ENTRIES      = new KnobTemplate< unsigned > ("tlb_entries", 4);
	KNOB_VMEM_PAGE_SIZE   = new KnobTemplate< unsigned > ("vmem_page_size", 4096);

	/* lab4 additional knobs  */

	KNOB_TRACE_FILE2 = new KnobTemplate< string > ("trace_file2", "trace2.pzip");
	KNOB_TRACE_FILE3 = new KnobTemplate< string > ("trace_file3", "trace3.pzip");
	KNOB_TRACE_FILE4 = new KnobTemplate< string > ("trace_file4", "trace4.pzip");
	KNOB_RUN_THREAD_NUM = new KnobTemplate< unsigned > ("run_thread_num", 3); 


}

all_knobs_c::~all_knobs_c() {
	delete KNOB_MAX_SIM_COUNT;
	delete KNOB_MAX_INST_COUNT;
	delete KNOB_OUTPUT_FILE;
	delete KNOB_PRINT_INST;
	delete KNOB_PERFECT_ICACHE;
	delete KNOB_PERFECT_DCACHE;
	delete KNOB_PERFECT_BR;
	delete KNOB_PERFECT_L2;
	delete KNOB_READ_TRACE;
	delete KNOB_TRACE_FILE;
	delete KNOB_ISSUE_WIDTH;
	delete KNOB_ICACHE_LATENCY;
	delete KNOB_DCACHE_LATENCY;
	delete KNOB_MEM_LATENCY;
	delete KNOB_L2CACHE_LATENCY;
	delete KNOB_PRINT_PIPE_FREQ;
	delete KNOB_DCACHE_SIZE; 
	delete KNOB_DCACHE_WAY;
	delete KNOB_BLOCK_SIZE;
	delete KNOB_DCACHE_FIXED_PENALTY; 
	delete KNOB_MEM_LATENCY_ROW_HIT; 
	delete KNOB_MEM_LATENCY_ROW_MISS;
	delete KNOB_MSHR_SIZE; 
	delete KNOB_DRAM_BANK_NUM; 
	delete KNOB_DRAM_PAGE_SIZE;
	delete KNOB_PRINT_MEM_DEBUG;
	delete KNOB_USE_BPRED;
	delete KNOB_BPRED_TYPE;
	delete KNOB_BPRED_HIST_LEN;
	delete KNOB_ENABLE_VMEM;
	delete KNOB_TLB_ENTRIES;
	delete KNOB_VMEM_PAGE_SIZE;
	delete KNOB_TRACE_FILE2;
	delete KNOB_TRACE_FILE3;
	delete KNOB_TRACE_FILE4;
	delete KNOB_RUN_THREAD_NUM;
}

void all_knobs_c::registerKnobs(KnobsContainer *container) {
	container->insertKnob( KNOB_MAX_SIM_COUNT );
	container->insertKnob( KNOB_MAX_INST_COUNT );
	container->insertKnob( KNOB_OUTPUT_FILE );
	container->insertKnob( KNOB_PRINT_INST );
	container->insertKnob( KNOB_PERFECT_ICACHE );
	container->insertKnob( KNOB_PERFECT_DCACHE );
	container->insertKnob( KNOB_PERFECT_BR );
	container->insertKnob( KNOB_PERFECT_L2 );
	container->insertKnob( KNOB_READ_TRACE );
	container->insertKnob( KNOB_TRACE_FILE );
	container->insertKnob( KNOB_ISSUE_WIDTH );
	container->insertKnob( KNOB_ICACHE_LATENCY );
	container->insertKnob( KNOB_DCACHE_LATENCY );
	container->insertKnob( KNOB_MEM_LATENCY );
	container->insertKnob( KNOB_L2CACHE_LATENCY );
	container->insertKnob( KNOB_PRINT_PIPE_FREQ );
	container->insertKnob( KNOB_DCACHE_SIZE); 
	container->insertKnob( KNOB_DCACHE_WAY);
	container->insertKnob( KNOB_BLOCK_SIZE);
	container->insertKnob( KNOB_DCACHE_FIXED_PENALTY); 
	container->insertKnob( KNOB_MEM_LATENCY_ROW_HIT); 
	container->insertKnob( KNOB_MEM_LATENCY_ROW_MISS);
	container->insertKnob( KNOB_MSHR_SIZE); 
	container->insertKnob( KNOB_DRAM_BANK_NUM); 
	container->insertKnob( KNOB_DRAM_PAGE_SIZE);
	container->insertKnob( KNOB_PRINT_MEM_DEBUG);
	container->insertKnob( KNOB_USE_BPRED);
	container->insertKnob( KNOB_BPRED_TYPE);
	container->insertKnob( KNOB_BPRED_HIST_LEN);
	container->insertKnob( KNOB_ENABLE_VMEM);
	container->insertKnob( KNOB_TLB_ENTRIES);
	container->insertKnob( KNOB_VMEM_PAGE_SIZE);
	container->insertKnob( KNOB_TRACE_FILE2);
	container->insertKnob( KNOB_TRACE_FILE3);
	container->insertKnob( KNOB_TRACE_FILE4);
	container->insertKnob( KNOB_RUN_THREAD_NUM);
}

void all_knobs_c::display() {
	KNOB_MAX_SIM_COUNT->display(cout); cout << endl;
	KNOB_MAX_INST_COUNT->display(cout); cout << endl;
	KNOB_OUTPUT_FILE->display(cout); cout << endl;
	KNOB_PRINT_INST->display(cout); cout << endl;
	KNOB_PERFECT_ICACHE->display(cout); cout << endl;
	KNOB_PERFECT_DCACHE->display(cout); cout << endl;
	KNOB_PERFECT_BR->display(cout); cout << endl;
	KNOB_PERFECT_L2->display(cout); cout << endl;
	KNOB_READ_TRACE->display(cout); cout << endl;
	KNOB_TRACE_FILE->display(cout); cout << endl;
	KNOB_ISSUE_WIDTH->display(cout); cout << endl;
	KNOB_ICACHE_LATENCY->display(cout); cout << endl;
	KNOB_DCACHE_LATENCY->display(cout); cout << endl;
	KNOB_MEM_LATENCY->display(cout); cout << endl;
	KNOB_L2CACHE_LATENCY->display(cout); cout << endl;
	KNOB_PRINT_PIPE_FREQ->display(cout); cout << endl;
	KNOB_DCACHE_SIZE->display(cout); cout << endl; 
	KNOB_DCACHE_WAY->display(cout); cout << endl;
	KNOB_BLOCK_SIZE->display(cout); cout << endl;
	KNOB_DCACHE_FIXED_PENALTY->display(cout); cout << endl; 
	KNOB_MEM_LATENCY_ROW_HIT->display(cout); cout << endl; 
	KNOB_MEM_LATENCY_ROW_MISS->display(cout); cout << endl;
	KNOB_MSHR_SIZE->display(cout); cout << endl; 
	KNOB_DRAM_BANK_NUM->display(cout); cout << endl; 
	KNOB_DRAM_PAGE_SIZE->display(cout); cout << endl;       
	KNOB_PRINT_MEM_DEBUG->display(cout); cout << endl;       
	KNOB_USE_BPRED->display(cout); cout << endl;       
	KNOB_BPRED_TYPE->display(cout); cout << endl;       
	KNOB_BPRED_HIST_LEN->display(cout); cout << endl;       
	KNOB_ENABLE_VMEM->display(cout); cout << endl;       
	KNOB_TLB_ENTRIES->display(cout); cout << endl;       
	KNOB_VMEM_PAGE_SIZE->display(cout); cout << endl;       
	KNOB_TRACE_FILE2->display(cout); cout << endl;
	KNOB_TRACE_FILE3->display(cout); cout << endl;
	KNOB_TRACE_FILE4->display(cout); cout << endl;
	KNOB_RUN_THREAD_NUM->display(cout); cout << endl;
}

