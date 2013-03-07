#ifndef MEMORY_H 
#define MEMORY_H

#include<list>
#include<queue>
#include "knob.h"
#include "all_knobs.h"

typedef enum Mem_Req_State_enum {
  MEM_INV,
  MEM_NEW,
  MEM_DRAM_IN, 
  MEM_DRAM_SCH, 
  MEM_DRAM_DONE,
  MEM_DRAM_OUT, 
  MEM_MAX_REQ_STATE, 
} Mem_Req_State;


typedef enum Mem_Req_Type_enum {
  MRT_IFETCH,			
  MRT_DFETCH, 
  MRT_DSTORE,
  MRT_IPRF,
  MRT_DPRF,
  MRT_WB,
  MRT_SW_DPRF,
  MAX_MEM_REQ_TYPE,
} Mem_Req_Type;


typedef struct mem_req_s {


  Mem_Req_State m_state;        /**< memory request state */   
  Mem_Req_Type  m_type;         /**< request type */

  ADDRINT         m_addr;         /**< request address */ 
  uint8_t         m_size;         /**< request size */
  uint64_t        m_rdy_cycle;    /**< request ready cycle */
  
  bool          m_dirty;         /**< wb request? */
  bool          m_done;          /**< request done flag */
  
  uint64_t      m_id;            /**< unique request id */     
  

} mem_req_s;

typedef struct m_mshr_entry_s {
  mem_req_s *m_mem_req; 
  bool valid; 
  UINT64 insert_time; 
  list<Op*> req_ops;  /* array to hold request ops */ 
}m_mshr_entry_s; 


typedef struct m_dram_s {
  int m_bank_num; 
  int64_t *m_row_id;
  uint64_t *m_rdy_cycle;
}m_dram_s; 

class memory_c {
 public: 
  
  list <m_mshr_entry_s *> m_mshr;  // mshr is an array of mshr_entries 
  list <m_mshr_entry_s *> m_mshr_free_list;  // a collection of free mshr_entries 
  int m_mshr_size;
  int m_dram_bank_num; 
  int m_block_size; 
  uint64_t m_unique_m_count; 
  
  list <mem_req_s *> dram_in_queue;  /* in to DRAM */ 
  list <mem_req_s *> dram_out_queue;  /*out from DRAM */ 

  /* dram structure */ 
  list <mem_req_s *>* dram_bank_sch_queue;  /* scheduler buffer for each DRAM bank */ 
  int64_t *dram_bank_open_row; 
  uint64_t *dram_bank_rdy_cycle; 
  

  m_mshr_entry_s * allocate_new_mshr_entry(void); 
  mem_req_s *get_new_mem_req(void); 
  
  
  void run_a_cycle(void); 
  
  void send_bus_in_queue(void);
  void dram_schedule(void);
  void push_dram_sch_queue(void); 
  void send_bus_out_queue(void);
  void fill_queue(void);
  void fill_tlb(void);
  void init_mem();
  
  
  bool insert_mshr(Op *op); 
  void free_mshr_entry(m_mshr_entry_s *entry); 
  
  m_mshr_entry_s *search_matching_mshr(ADDRINT addr);
  list<m_mshr_entry_s*>::iterator search_matching_mshr_itr(ADDRINT addr); 
  
  bool check_piggyback(Op *op);
  bool store_load_forwarding(Op *op); 
    
  void dprint_queues(void);
  void dprint_dram_banks(void);
  
  int mshr_ops(void);
};

extern KnobsContainer *g_knobsContainer; /* < knob container > */
extern all_knobs_c    *g_knobs; /* < all knob variables > */


#endif // MEMORY_H 
