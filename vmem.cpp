//Author : Moinuddin K Qureshi, Prashant J Nair
//MS ECE : Georgia Institute of Technology

#include <assert.h>
#include "vmem.h"
/*
#define VMEM_MAX_THREADS     4
#define VMEM_PTE_SIZE        8
#define VMEM_PAGE_TABLE_SIZE ((1<<20)*VMEM_PTE_SIZE)
*/

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

tlb *tlb_new(int size) {
  tlb *t = (tlb *) calloc(1, sizeof(tlb));

  t->size=size;
  t->s_access = 0;
  t->s_miss = 0;
  t->entries = (tlb_entry *)calloc(size, sizeof(tlb_entry));

  for(int ii=0; ii<t->size; ii++) {
    tlb_entry *entry = &t->entries[ii];  
    entry->vpn = -1;
    entry->pfn = -1;
    entry->threadid = -1;
    entry->valid = false;
    entry->last_access_time = -1;
  }

  return t;
}



//////////////////////////////////////////////////////////////
// returns 1 if TLB hit, 0 otherwise
// If hit, the PFN field is changed to the translated PFN
//////////////////////////////////////////////////////////////

bool tlb_access (tlb *t, uint64_t vpn, int threadid, uint64_t *pfn) {
  bool found=0;

  for(int ii=0; ii<t->size; ii++){
    tlb_entry *entry = &t->entries[ii];
    if( entry->valid &&	(entry->threadid==threadid) && (entry->vpn==vpn) ) {
      found=1;
      *pfn = entry->pfn;
      entry->last_access_time = t->s_access;
      break;
    }
  }
  
  t->s_access++;

  if(found==0){ 
    t->s_miss++;
  }

  return found;
}


//////////////////////////////////////////////////////////////
// Use this function only after TLB miss and DRAM access for PTE completes
// Get the actual PFN not from PTE but from vmem_vpn_to_pfn and then install
//////////////////////////////////////////////////////////////

void tlb_install(tlb *t, uint64_t vpn, int threadid, uint64_t  pfn) {
  tlb_entry *entry = NULL;
  uint64_t min_access_time;

  //search for an invalid entry
  for(int ii=0; ii<t->size; ii++){
    if(!t->entries[ii].valid) {
      entry = &t->entries[ii];
      break;
    }
  }

  //no invalid entry, use LRU replacement 
  if(entry==NULL) {
    entry = &t->entries[0];
    min_access_time = t->entries[0].last_access_time;
    for(int ii=1; ii<t->size; ii++){
      if(t->entries[ii].last_access_time < min_access_time) {
        entry = &t->entries[ii];
        min_access_time = t->entries[ii].last_access_time;
      }
    }
  }

  //insert new TLB entry
  entry->vpn = vpn;
  entry->pfn = pfn;
  entry->threadid = threadid;
  entry->valid = true;
  entry->last_access_time = t->s_access;
  t->s_access++;

}


//////////////////////////////////////////////////////////////
// This function provides PTE address for a given VPN
// On TLB miss, use this function to get the DRAM address for PTE
// Do not change this function
//////////////////////////////////////////////////////////////

uint64_t vmem_get_pteaddr(uint64_t vpn, int threadid) {

  assert(threadid < VMEM_MAX_THREADS);
  
  uint64_t pte_base_addr = threadid * VMEM_PAGE_TABLE_SIZE;
  uint64_t pte_offset = (vpn%(1<<20))* VMEM_PTE_SIZE;

  uint64_t pte = pte_base_addr + pte_offset;

  return pte; // provides a byte address of PTE (change to lineaddr)
}

//////////////////////////////////////////////////////////////
// This function provides the actual VPN to PFN translation
// Access this function once you get the PTE from DRAM, and before TLB install
// Do not change this function
//////////////////////////////////////////////////////////////

uint64_t vmem_vpn_to_pfn(uint64_t vpn, int threadid) {

  assert(threadid < VMEM_MAX_THREADS);

  uint64_t total_os_reserved_pages = 16384;
  uint64_t pages_per_thread = (1<<20); // 4GB per thread
  uint64_t pfn_base = threadid*pages_per_thread;
  uint64_t pfn_thread = vpn % pages_per_thread; // simple function (not realistic)
  uint64_t pfn = total_os_reserved_pages + pfn_base + pfn_thread;

  return pfn;
}
