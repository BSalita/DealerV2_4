/* File mmap_template.h */
#ifndef MMAP_TEMPLATE_H
#define MMAP_TEMPLATE_H

/* Date      Version  Author  Description
* 2022/10/22 1.0.0    JGM     Define a memory layout for the shared memory area
* 2024/02/26 1.1.0    JGM		Increase the results area from 2x64 to 2x128
* 2024/09/11 1.2.0    JGM     Add the DDS tricks to the dealdata passed to UserServer
*/
#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif
#ifndef DEALTYPES_H
   #include "../include/dealtypes.h"
#endif

#define NO_MORE_QUERY -1
#define CACHE_OK 1
#define CACHE_STALE 0
typedef struct user_eval_cache_st {  // one cache for each side
   int dealnum[2] ;                 /* dealer sets this when calling server */
   int qtag[2]    ;                 /* ditto */
   int state[2] ;                   /* maintained by dealer; server does not care about cache */
} UEVAL_CACHE_k ;

typedef struct mmap_hdr_st {     /* some info to share between dealer(client) and usereval(server) */
   char    map_name[32];         /* file name used in the open call */
   char    q_sema_name[32];      /* name of semaphore so server can sem_open() it. */
   char    r_sema_name[32];      /* ditto */
   off_t   q_type_off;           /* offset to the type of query. offset calculated at init time by dealer.*/
   off_t   r_type_off  ;         /* offset to the type of reply. offset calculated at init time by dealer.*/
   off_t   deal_data_off ;       /* offset to the current deal, and the numbers from the analyze routine. */
   off_t   user_nsvals_off ;     /* offset to the NS results to be returned to the dealer from the user_eval*/
   off_t   user_ewvals_off ;     /*     ditto for EW results */
   off_t   mm_cache_off    ;     /* offset to the cache that can be updated by either dealer or usereval */
   int     stderr_fd       ; 
} mmap_hdr_k;
/*
 * Let the user write the code to return the results he wants and where he wants them. If he wants NT only write code for that.
 * By default return Valuation for NT side, and NT each hand; also HLDF points in longest fit for Side and each hand. 6 res in all.
 * Might also return various 'factors' such as L pts, Syn pts etc to test their effect.
 *
 */
typedef struct query_type_st {  /* this is  not the input data just a description of the query. */
   int query_tag ;               /* Usually Metric type; 0 for Bergen, 1 for CPU, 2 for DKP etc. see the enum */
   int query_side;
   int query_hflag;   /* set to 1 if the hand field is relevant */
   int query_hand ;
   int query_sflag;  /* set to 1 if the suit field is relevant */
   int query_suit ;
   int query_idx  ;  /* in case user is only interested in one value, and wants it in this location */
   int  q_vals[8]            ; /* Could use the q_vals array to further describe the type of result we want from the query_tag.*/
   char query_descr[32]      ; /* text field to help with debugging */
 } query_type_k;

typedef struct deal_data_st {
   int curr_gen, curr_prod ;
   HANDSTAT_k hs[4];             /* includes Has_cards[][] */
   DEAL52_k curdeal ;
   SIDESTAT_k ss[2];             /* values per side; mostly opc currently; future dummy suppt pts etc. */
   int dds_cache   ;             /* 0 = Stale. 1 = OK; 2=Needs an update */
   int dds_trix[4][5] ;          /* N:C,D,H,S,NT, E:C,D,H,S,NT, S:C,D,H,S,NT, W:C,D,H,S,NT,  -1 aka INV_TRIX means not present */
                        /* Future other aspects cccc etc. if needed */
} DEALDATA_k ;

typedef struct reply_type_st {
   int  reply_tag ;
   int  r_vals[8];
   char reply_descr[32]      ;
} reply_type_k;
#define NUSER_RESULTS 128
typedef struct user_eval_results_st { // this struct holds the results for one side. the usual case for this kind of query
   union res_u_ut {
      int res[128] ;             // quick easy access if user can keep track himself. syntax p_ures->u.res[i]
      struct detailed_res_st {   // suggested layout for convenience. will correspond to syntax in dealer input file
         int side_tot[32] ;        //0-31 32 metrics for side as a whole. Usual case otherwise dealer is adequate.
         int side_by_suit[4][8] ;  //32-63  8 diff metrics for the side, broken down by suit.[CDHS][wxyz]
         int hand_tot[2][16];      //64-95 hand results  16 diff metrics total for each hand
         int hand_suit[2][4][4];   //96-127 results by suit; 4 diff metrics for each hand-suit combo.
      } dr ;                       // detailed result
    } u ;                          // Syntax to access the above: uval.u.res[i] or uvalNS.u.dr.hand_suit[h][s][i]
} USER_VALUES_k;                   //   -- or -- using a pointer: ptr->u.res[i] or ptrEW->u.dr.hand_suit[h][s][i]

typedef struct mmap_template_st {         // how the shared memory is laid out and used.
   struct mmap_hdr_st      mm_hdr_dat   ; // pointers and offsets to the rest of the memory
   struct query_type_st    mm_qtype_dat ; // query tag, side, etc. spec for the UserEval server
   struct reply_type_st    mm_rtype_dat ; // passed back by the Server. currently not used much
   struct deal_data_st     mm_deal_data ; // the deal itself, and the handstat, and sidestat analysis, and the DDS tricks if available
   USER_VALUES_k           mm_user_nsvals;// 128 integers to report the Server results for the NS side.
   USER_VALUES_k           mm_user_ewvals;//     ditto for the EW side
   UEVAL_CACHE_k           mm_cache ;     // copy of the Cache in case the Server needs to update it. Not used ?

} MMAP_TEMPLATE_k ;

struct mmap_ptrs_st {                     //  Collect all the pointers to the parts of mmap in one place
   mmap_hdr_k     *header;
   query_type_k   *query ;
   reply_type_k   *reply ;
   DEALDATA_k     *dldata;
   USER_VALUES_k  *nsres, *ewres ;
   UEVAL_CACHE_k  *cache ;
   HANDSTAT_k     *phs[4]; // pointers to copy of each hand's handstat_st in the MMAP area; saves having to use dldata ptr */
   DEAL52_k       *p_deal;
} ;
struct mmap_off_st {                      // Pointer = BasePtr + Offset ; mostly for debugging.
   off_t    header;
   off_t    query ;
   off_t    reply ;
   off_t    dldata;
   off_t    nsres ;
   off_t    ewres ;
   off_t    cache ;
   off_t    hs_arr;
   off_t    curdeal;
} ;

#endif  /* file guard */


