/* File UserEval_externs.h  -- globals for the various user_eval sub funcs and sub-sub_funcs
 *  Version  Date          Comment
 *    0.5    2022-11-26    First  -- must keep in sync with UserEval_globals.c
 *    1.0    2024-08-01    Included ALL_UE code
 */
#ifndef USEREVAL_EXTERNS_H
#define USEREVAL_EXTERNS_H
#include "../include/std_hdrs_all.h"  /* all the stdio.h stdlib.h string.h assert.h etc. */
#ifndef USEREVAL_TYPES_H
   #include "../include/UserEval_types.h"
#endif
#ifndef MMAP_TEMPLATE_H
   #include "../include/mmap_template.h"
#endif

/*
 * Some Globals -- Server Mainline and top Level
 */
extern int jgmDebug ;  /* where from? not UserEval_globals UserServer.c file globals?... */

   /* Next bit from UserServer_globals.c */
extern char metric_names[][16] ;
// extern char alt_HCP_names[][16] ;

/* Vars Used  by the User Functions; Setup by the Server Infrastructure */
extern struct mmap_ptrs_st ptrs ;    /* all relevant mmap pointers */
extern struct mmap_off_st  offs ;    /* relevant offsets in case they are useful in debugging. */

extern DEALDATA_k           *p_dldat ;     /* the deal and handstat area */
// extern USER_VALUES_k        *p_nsdat ;     /* results for North, South, and NS side */
// extern USER_VALUES_k        *p_ewdat ;     /* results for East,  West,  and EW side */
extern HANDSTAT_k           *hs_ptr[4] ;   /* 4 pointers; one to each handstat struct */

   /* most of what follows is from UserEval_globals.c */

extern int gen_num  ; 
extern int prod_num ;

/* prolog stuff; uses vars setup by the Server Infrastructure */
extern HANDSTAT_k *phs[2] ;         /* pointers to two related HS structs; N/S or E/W */
extern struct gbl_struct_st gbl ;	/* tag, side, compass, suit, idx from the query pkt */
extern USER_VALUES_k *p_uservals  ; /* points to the 128 int array for the side in question */
extern int seat[2] ;                /* Server is called with a side; these are the two seats(0..3) related to that side. */
extern char compass[2];             /* seat names for debug statements */
extern UE_SIDESTAT_k       UEsidestat[2] ; /* suit lens, fitlens, bestfit suit, declarer in best fit, etc. */
extern UE_SIDESTAT_k      *p_UEss  ;

/* results stuff */
extern struct FitPoints_st TFpts ;
extern struct misfit_st misfit[4] ;  /* 4 possible misfits, one in each suit */
extern struct UserEvals_st UEv   ;  /* room for 128 integers and anything else of interest returned by the xxxx_calc functions*/

// Temp vars used by most routines.
extern enum suit_ek suit ;      /* CLUBS, DIAMONDS, HEARTS, SPADES  */
extern enum metric_ek metric ;

extern int h_dummy , h_decl ;
extern int misfit_cnt ;
extern int side_nt_pts, seat_nt_pts[2], side_hldf_pts, seat_hdlf_pts[2] ; /* these are the most likely results wanted */
extern int hcp[2], hcp_adj[2], dpts[2], lpts[2], dfit_pts[2], syn_pts[2], hf_pts[2], Fn_pts[2], hand_adj[2], body_pts[2];
extern int freak[2][4];
extern int dpts_suit[2][4], lpts_suit[2][4], syn_pts_suit[2][4], hf_pts_suit[2][4] ;
 /* Some calcs will be done in float; cast to int or int*100 at the end*/
extern float_t fhcp[2], fhcp_adj[2], fhcp_suit[2][4], fhcp_adj_suit[2][4], suit_qual[2][4];

extern int set88[] ;
 // These globals no longer needed. Use global UEfitstat[h] for these
extern int sorted_slen[2][4], sorted_slen_idx[2][4] ;   /* Suit lengths and 'names' sorted longest to shortest */
extern int sorted_slen[2][4], sorted_slen_idx[2][4] ;   /* Suit lengths and 'names' sorted longest to shortest */
extern int sorted_fitlen[4] , sorted_fitlen_idx[4] ;
extern int gbl_fitlen, gbl_fitsuit;   /* set by Fill_sidestat */

#endif /* end file guard */



