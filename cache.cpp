
#include "common.h"
#include "sim.h"
#include "trace.h" 
#include "cache.h"
#include <stdlib.h>
#include <string.h> /* String operations library */ 
#include <ctype.h> /* Library for useful character operations */
#include "knob.h"
#include "all_knobs.h"

extern uint64_t cold_misses;
//************************************************************************************
void cache_init(Cache *cache,unsigned int cache_size, int block_size,unsigned int assoc, const char *s)
{
  int ii;
  int jj;

  cache->cache_size  = cache_size;
  cache->block_size  = block_size;
  cache->assoc       = assoc;
  cache->entry_size  = cache_size*1024/(block_size)/assoc;   /* number of cache entry */
  strcpy(cache->name,s);
  /* cache entry dynmaic memory allocation */
  cache->cache_entry = (Cache_Entry**) malloc(sizeof(Cache_Entry *)*(cache->entry_size));
    
  for( ii = 0; ii < cache->entry_size; ii++) {
    /* create 2dimensional array size */
    cache->cache_entry[ii] = (Cache_Entry *) malloc(sizeof(Cache_Entry) * assoc);
    for( jj = 0 ; jj < assoc; jj++) {
	  cache->cache_entry[ii][jj].tag = 0; //Initialize the tag to 0
      cache->cache_entry[ii][jj].valid = false;
      cache->cache_entry[ii][jj].lru = 0;
      cache->cache_entry[ii][jj].dirty = false;
    }
  }
    
#if PRINT_DEBUG 
  printf("cache_init:%s is done, cache_entry size is %d\n", cache->name, cache->entry_size);
#endif 
}

int cache_access(Cache *cache, ADDRINT addr) 
{
  int cache_hit = false; 
  ADDRINT cache_line = (addr >> (LOG2(cache->block_size)) );    /*cache line address */
  cache_hit = cache_read(cache, addr);
  return cache_hit; 

}

int cache_read(Cache *cache, ADDRINT addr)
{
    
  int ii;
  int lru_index = 0;
    
  ADDRINT cache_line = (ADDRINT) (addr >> (LOG2(cache->block_size)) );    /*cache line address */
  int line_num   = cache_line % cache->entry_size;   /* cache index */
  ADDRINT tag        = (ADDRINT) cache_line / cache->entry_size;   /* cache tag */
  int cache_cold = false;
    
    
#if PRINT_DEBUG 
  printf("[%s] addr:%5u cache_line:%5d line_num:%5d tag:%5d log2_block:%3d ", 
	 cache->name, addr, cache_line, line_num, tag , LOG2(cache->block_size));
#endif 
  /* set associative */ 
  for(ii = 0; ii < cache->assoc; ii++) {
    if(cache->cache_entry[line_num][ii].tag == tag ) {   // tag match 
      if(cache->cache_entry[line_num][ii].valid == false) {  // data should be valid 
	cache_cold = true;
	cold_misses++;                   
      }
 //     cache->cache_entry[line_num][ii].lru = cycle_count;    // BUG in code update lru time 
 //      cache->cache_entry[line_num][ii].valid = true;			//BUG in code

      if(!cache_cold) {
	cache->cache_entry[line_num][ii].lru = cycle_count; 
	return HIT;
      }
      else return MISS; // cache block was empty so we don't need to evict cache block 
    }
  }
    
  return MISS;     
}


void cache_insert(Cache *cache, ADDRINT addr) {

      
  ADDRINT cache_line = (ADDRINT) (addr >> (LOG2(cache->block_size)) );    /*cache line address */
  int line_num   = cache_line % cache->entry_size;   /* cache index */
  ADDRINT tag        = (ADDRINT) cache_line / cache->entry_size;   /* cache tag */

  /* cache replacement */
  int e_index = get_repl_index(cache, line_num) ;      /* cache replacement policy */ 
  /* you can implement different replacement policies */ 

//  printf("dcache....addr=%u, line_num=%d, e_index=%d, tag=%d\n", addr, line_num, e_index, tag);


  if(cache->cache_entry[line_num][e_index].valid == true) {
#if PRINT_DEBUG 
    printf("evict! tag :%5d time: %d ", cache->cache_entry[line_num][e_index].tag , cache->cache_entry[line_num][e_index].lru);
#endif 
       
    if(cache->cache_entry[line_num][e_index].dirty) {

      // for a write-back cache, we have to insert a new memory request into the memory system 
      // for this assignment, we will just drop this request 
	 
    }
       
  }

  cache->cache_entry[line_num][e_index].tag   = tag;
  cache->cache_entry[line_num][e_index].lru   = cycle_count;
  cache->cache_entry[line_num][e_index].valid = true;
  cache->cache_entry[line_num][e_index].dirty = false;  // it should set a dirty bit correctly 
#if PRINT_DEBUG 
  printf("%d: replace  %d\n ",cycle_count, e_index);
#endif
   
}



int get_repl_index(Cache *cache, int line_num)  {      
  /* you can implement different polices */ 
  /* here LRU replacement policy is implemented */ 
  
  int lru_index=0; 
  /* searching for oldest  cache data  PERFECT LRU method  */
  for (int ii = 0; ii < cache->assoc; ii++) {
    if(cache->cache_entry[line_num][ii].lru < cache->cache_entry[line_num][lru_index].lru) {
      lru_index = ii;
    }
  }
  return lru_index; 
}
