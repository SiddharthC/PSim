#ifndef CACHE_H 
#define CACHE_H

//**************Cache Structures for Lab2*************************************
#define HIT   1
#define MISS  0
#ifndef PRINT_DEBUG
#define PRINT_DEBUG 0
#endif

//Useful MACROS
#define MIN(x,y)  ((x) < (y) ? (x) : (y)) 

typedef struct Cache_Entry_Struct{
  ADDRINT tag;        /* address tag */
  int lru;        /* for replacement policy */
  int valid;      /* valid bit */
  bool dirty;     /* dirty bit for a writeback cache */ 
    /* no data storage */
}Cache_Entry;

typedef struct Cache_Struct{
    Cache_Entry **cache_entry;
    unsigned int cache_size;
    int block_size;
    unsigned int assoc;
    int entry_size;   /* number of cache entry */
    char name[128];
}Cache;

//Cache Access Functions

void init_dcache(void); 
void init_icache(void); 
void cache_init(Cache *cache, unsigned int cache_size, int block_size, unsigned int assoc,const char *s);
int cache_access(Cache *cache, ADDRINT  addr);
int cache_read(Cache *cache, ADDRINT addr);
void cache_insert(Cache *cache, ADDRINT addr); 
int get_repl_index(Cache *cache, int index); 

/* local variables for D-cache   */

// initial parameter 
static int cache_size, block_size, assoc;
static int cache_latency, memory_latency, bus_width;
static int memory_access_cycle;        /*number of access for bring the chunck of data */


#define LOG2(x) ( (x == 1 ) ? 0  : 		                  	\
                 ((x == (0x1 << 1 )) ? 1  :	              			\
		  ((x == (0x1 << 2 )) ? 2  :					\
		   ((x == (0x1 << 3 )) ? 3  :					\
		    ((x == (0x1 << 4 )) ? 4  :					\
		     ((x == (0x1 << 5 )) ? 5  :					\
		      ((x == (0x1 << 6 )) ? 6  :				\
		       ((x == (0x1 << 7 )) ? 7  :				\
			((x == (0x1 << 8 )) ? 8  :				\
			 ((x == (0x1 << 9 )) ? 9  :				\
			  ((x == (0x1 << 10)) ? 10 :				\
			   ((x == (0x1 << 11)) ? 11 :				\
			    ((x == (0x1 << 12)) ? 12 :				\
			     ((x == (0x1 << 13)) ? 13 :				\
			      ((x == (0x1 << 14)) ? 14 :			\
			       ((x == (0x1 << 15)) ? 15 :			\
				((x == (0x1 << 16)) ? 16 :			\
				 ((x == (0x1 << 17)) ? 17 :			\
				  ((x == (0x1 << 18)) ? 18 :			\
				   ((x == (0x1 << 19)) ? 19 :			\
				    ((x == (0x1 << 20)) ? 20 :			\
				     ((x == (0x1 << 21)) ? 21 :			\
				      ((x == (0x1 << 22)) ? 22 :		\
				       ((x == (0x1 << 23)) ? 23 :		\
					((x == (0x1 << 24)) ? 24 :		\
					 ((x == (0x1 << 25)) ? 25 :		\
					  ((x == (0x1 << 26)) ? 26 :		\
					   ((x == (0x1 << 27)) ? 27 :		\
					    ((x == (0x1 << 28)) ? 28 :		\
					     ((x == (0x1 << 29)) ? 29 :		\
					      ((x == (0x1 << 30)) ? 30 :        \
					       ((x == (0x1 << 31)) ? 31 : 1))))))))))))))))))))))))))))))))

#endif // CACHE_H
