//Author: Prashant J Nair, Georgia Tech
//Simulation of Branch Predictors

/****** Header file for branch prediction structures ********/

#ifndef __BPRED_H
#define __BPRED_H
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>

#define HW_MAX_THREAD 4

typedef enum bpred_type_enum{
  BPRED_NOTTAKEN, 
  BPRED_TAKEN,
  BPRED_BIMODAL,
  BPRED_GSHARE 
}bpred_type;

typedef struct bpred bpred;

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

struct bpred{
  unsigned int  ghr[HW_MAX_THREAD]; // global history register
  unsigned int *pht; // pattern history table

  bpred_type type; // type of branch predictor
  unsigned int hist_len; // history length
  unsigned int pht_entries; // entries in pht table

  long int mispred; // A counter to count mispredictions
  long int okpred; // A counter to count correct predictions
};


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

bpred *bpred_new(bpred_type type, int hist_len);
int    bpred_access(bpred *b, unsigned int pc, int thread_id);
void   bpred_update(bpred *b, unsigned int pc, int pred_dir, int resolve_dir, int thread_id);

// bimodal predictor
void   bpred_bimodal_init(bpred *b);
int    bpred_bimodal_access(bpred *b, unsigned int pc);
void   bpred_bimodal_update(bpred *b, unsigned int pc, int pred_dir, int resolve_dir);


// gshare predictor
void   bpred_gshare_init(bpred *b);
int    bpred_gshare_access(bpred *b, unsigned int pc, int thread_id);
void   bpred_gshare_update(bpred *b, unsigned int pc, int pred_dir, int resolve_dir, int thread_id);

/***********************************************************/
#endif
