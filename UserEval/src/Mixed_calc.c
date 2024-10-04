/* File Mixed_calc.c */
/* Date        Version  Author   Description
 * 2024/08/12  1.0      JGM      The composite metrics Set40, MixKarp, MixLar, Set88, test_calc(Set99), Tst etc.
 *
 */
#define GNU_SOURCE
#include "../include/std_hdrs_all.h"
#include "../include/UserEval_types.h"
#include "../include/UserEval_externs.h"
#include "../include/dbgprt_macros.h"
#include "../include/mmap_template.h"
#include "../include/UE_calc_protos.h"
#include "../include/UE_util_protos.h"

/* External functions -- metrics_util_subs.c , short_honors_subs.c */

 /* Forward Function Definitions */
int unkn_err(   UE_SIDESTAT_k *p_ss ) { fprintf(stderr,"Unknown Metric Not Implemented. Returning -1\n"); return -1 ; }
void showints( int *p_lo , int *p_hi, int count ) ;

/* dispatch table used by most of what follows */
typedef int (*pCALC_FUNC_k)(  UE_SIDESTAT_k *p_ss) ;
pCALC_FUNC_k p_cfunc[] = {bergen_calc,  bissell_calc, dkp_calc,      goren_calc,   kaplan_calc,  
                          karpin_calc,  karpb_calc,   knr_calc,      lar_calc,     lar_b_calc,
                          pav_calc,     sheinw_calc,  zarbas_calc,   zaradv_calc,  roth_calc, unkn_err } ;
 char cf_name[][10] = {"Bergen", "Bissel", "DKP", "Goren","Kaplan","Karpin","Karp_B", "KnR","Larsson","Lar_B",
                       "Pavlicek","Sheinwold","ZarBasic","ZarAdvced","ROTH", "!Unkn" };

                       
/* Tag 88 is a way of getting all possible evaluations done with one call. This can be useful because
 * the cache is filled and all the numbers can then be got without calling the  server so many times.
 * The downside is that only a limited number of results per metric can be obtained.
 * But in practice this is not a drawback as usually we are only interested in at most two results:
 * The total for the side if played in NT and the total for the side if played in the longest suit.
 * Since we can gete 128 results stored in the mmap, this allows the main 2 results for 64 metrics.
 * Set88 is a reduced version of set40, which returns 6 values per metric for each metric.
 */
/* the set88 array allows us to filter or choose which of the implemented metrics to do the evaluations for
 * To change the choice, either Recompile or patch the array with a hex editor or a Debugger
 */
 char FIND88[]="8888 SET 8888";  /*marker in case we want to patch next array with GDB */
  /*                     0        1       2     3      4       5       6      7    8      9     10     11       12      13      14      15        20          21  */
//enum metric_ek    { BERGEN=0, BISSEL,  DKP, GOREN, KAPLAN, KARPIN, KARP_B, KnR, LAR, LAR_B, PAV, SHEINW,  ZARBAS, ZARADV, ROTH, metricEND, MixJGM=20, MixMOR,
 //              B,b,D,  G,E, K,kb, KnR, L,lb, P,S, z,Z, R  U    1 means include in set, zero skip. -0 not implemented Future HCP Flavors
 int  set88[16]={1,1,1,  1,1, 1,1,   1,  1,1,  1,1, 1,1, 1, -0 } ;
 #define SET88_SZ sizeof(set88)/sizeof(int)
 int  metric_enabled[16]={1, 1, 1,  1, 1, 1,  1, 1, 1,  1, 1, 1,  1, 1, 1, -0 } ;
 #define m_enabled_SZ sizeof(metric_enabled)/sizeof(int)
int set88_calc ( UE_SIDESTAT_k *p_ss) { /* returns the side total in BestFit and the side total in NT for each metric enabled in the set_88 table */
   int m;
   int m_cnt = 0 ;
   USER_VALUES_k *p_88_results;
  /* we don't need to redo prolog, or reset_UEsidestat for each metric; we only need to zero the globals. */
   int set88_HLDF_pts[SET88_SZ] = { -1 };   /* set88 returns the Suit evals in res[0] to [14] */
   int set88_NT_pts[SET88_SZ]   = { -1 };   /* set88 returns the NT evals in res[15] to res[29]*/
   int nt_offset = metricEND ;
   JGMDPRT(6, "SET88_SZ = %ld, metricEnd=%d, nt_offset=%d \n", SET88_SZ, metricEND, nt_offset );

   p_88_results   = p_uservals;
   for (m=0 ; m<metricEND ; m++ ) {
      if (set88[m] == 1 ) {
         (*p_cfunc[m])( p_ss ) ;  /* results in UEv struct. Returns number of results calculated; we only care about the side total */
         set88_HLDF_pts[m] = UEv.hldf_pts_side ; /* we need to store these temporarily bec the metrics modify the userresults area*/
         set88_NT_pts[m]   = UEv.nt_pts_side;
         if( m == ZARBAS ) {  /* return the rounded half zar pts which are on a 0 .. 40 pt scale */
            set88_NT_pts[m]   = UEv.misc_pts[0]+UEv.misc_pts[1];  // use same for both as ZarBas has no Hf, Fn, Dfit etc. pts
            set88_HLDF_pts[m] = UEv.misc_pts[0]+UEv.misc_pts[1];
         }
         if ( m == ZARADV ) { /* return the rounded half zar pts which are on a 0 .. 40 pt scale */
            set88_NT_pts[m]   = UEv.misc_pts[0]+UEv.misc_pts[1];
            set88_HLDF_pts[m] = UEv.misc_pts[2]+UEv.misc_pts[3];
         }           
         m_cnt++;
         JGMDPRT(4," Metric=%d, Enabled?=%d, name=%s HLDF_pts=%d, NT_pts=%d m_cnt=%d funcptr=%p\n",
                     m,set88[m],cf_name[m],set88_HLDF_pts[m],set88_NT_pts[m], m_cnt, p_cfunc[m]);
      }
      else { set88_HLDF_pts[m] = -1 ; set88_NT_pts[m] = -1 ; }
   }
   /* put the set88 values into the mmap user results area */
  /* one hldf val and one nt val per metric */
   for (m=0; m < metricEND ; m++ ) {
      p_88_results->u.res[m] = set88_HLDF_pts[m] ;          /* some of these will be invalid i.e. -1; USER BEWARE */
      p_88_results->u.res[m + nt_offset] = set88_NT_pts[m];
   }
   JGMDPRT(7, "Set88 did %d metrics. Showing user results at ptr=%p, from 0 to 29 for\n",m_cnt,(void *)p_88_results );
   DBGDO(7, show_user_res( "Set_88_calcs", p_88_results, 0, 29 )  ) ;
   return m_cnt*2; /* the total number of valid results in the 0 .. (NT_offset+m_cnt) slots */
}

int mixed_Karpin_calc (  UE_SIDESTAT_k *p_ss ) { /* call karpin_calc then karpb_calc  tag = 50 */
   int karpin_pts[8] = { 0 };
   int karpb_pts[8] = { 0 };
   int i ;
   karpin_calc( p_ss ) ;
   for (i = 0 ; i < 6 ; i++ ) {
      karpin_pts[i] = p_uservals->u.res[i] ; /* save results since they will be overwritten by karpb_calc 2024-08-02 add t_suit, t_fitlen*/
   }
   karpin_pts[6] = UEv.hldf_suit ;  /* Global UEv filled by karpin calc still OK at this point */
   karpin_pts[7] = UEv.hldf_fitlen;
   
   karpb_calc( p_ss ) ;
   for (i = 0 ; i < 6 ; i++ ) {
      karpb_pts[i] = p_uservals->u.res[i] ; /* save results since they will be overwritten by karpin pts. */
   }
   karpb_pts[6] = UEv.hldf_suit ;  /* Global UEv filled by karpin calc still OK at this point */
   karpb_pts[7] = UEv.hldf_fitlen;
   /* now copy the saved karpin results to the next available slots in the p_uservals area */
   for (i = 0 ; i < 6 ; i++ ) {
      p_uservals->u.res[i]   = karpb_pts[i]    ;  /* karpb in lower slots since karpb is lower numbered metric */
      p_uservals->u.res[i+6] = karpin_pts[i] ;
   }
      p_uservals->u.res[12]   = karpb_pts[6]    ; /* trump suit */
      p_uservals->u.res[13]   = karpb_pts[7]    ; /* trump fitlen */
      p_uservals->u.res[14]   = karpin_pts[6]   ; /* trump suit same as karpb_pts since trump suit chosen by Analyze Side */
      p_uservals->u.res[15]   = karpin_pts[7]   ; /* trump fitlen */
   /* we should now have:
    * in slots 0 .. 5 for KARP_B: NT_side, NT_north, NT_south, HLDF_side, HLDF_north, HLDF_south
    * in slots 6 ..11 for Karpin: NT_side, NT_north, NT_south, HLDF_side, HLDF_north, HLDF_south (overwriting jgm dbg vals)
    * The misc extra debug values in slots 16 .. nn are still there, but will be ignored by Dealer
    * Added trump suit and trump fitlen to help dealer chose which dds tricks to use
    */
    return 16 ;  /* number of values we care about 6 + 2 + 6 + 2 */
} /* end mixed_Karpin_calc */

int mixed_Larsson_calc  ( UE_SIDESTAT_k *p_ss ) { /* call lar_calc    then lar_b_calc tag = 21 */

   int lar_pts[8] = { 0 };
   int lar_b_pts[8] = { 0 };
   int i ;
   lar_calc( p_ss ) ;
   for (i = 0 ; i < 6 ; i++ ) {
      lar_pts[i] = p_uservals->u.res[i] ; /* save results since they will be overwritten by lar_b_calc */
   }

   lar_b_calc( p_ss ) ;
   for (i = 0 ; i < 6 ; i++ ) {
      lar_b_pts[i] = p_uservals->u.res[i] ; /* save results since they will be overwritten by lar_calc */
   }
   lar_b_pts[6] = UEv.hldf_suit ;  /* Global UEv filled by lar_calc still OK at this point */
   lar_b_pts[7] = UEv.hldf_fitlen;
   /* now copy the saved lar results to the next available slots in the p_uservals area LAR first since it is lower metric*/
   for (i = 0 ; i < 6 ; i++ ) {
      p_uservals->u.res[i] = lar_pts[i] ;
      p_uservals->u.res[i+6]  = lar_b_pts[i] ;

   }              /* extraneous values to help with choosing which DDS tricks to pick */
      p_uservals->u.res[12]   = lar_pts[6] ;   /* trump suit */
      p_uservals->u.res[13]   = lar_pts[7] ;   /* trump fitlen */
      p_uservals->u.res[14]   = lar_b_pts[6];  /* trump suit same as lar_pts since trump suit chosen by Analyze Side */
      p_uservals->u.res[15]   = lar_b_pts[7];  /* trump fitlen */
   /* we should now have:
    * in slots 0 .. 5 for LAR_B NT_side, NT_north, NT_south, HLDF_side, HLDF_north, HLDF_south
    * in slots 6 ..11 for LAR   NT_side, NT_north, NT_south, HLDF_side, HLDF_north, HLDF_south (overwriting jgm dbg vals)
    * The misc extra debug values in slots 16 .. nn are still there, but will be ignored by Dealer
    * Added trump suit and trump fitlen to help dealer chose which dds tricks to use
    */
    return 16 ;  /* number of values we care about 16 = 6 + 2 + 6 + 2 */
} /* end mixed_LAR_calc */

/* for the tag 99, make test results, no analyze side or best fit etc. needed. Just raw write to User Results area */
int make_test_evals(struct detailed_res_st *p_ures) ;  /* prototype from metrics_utils_subs.c raw write to UserEvals area */
int test_calc ( UE_SIDESTAT_k *p_ss ) {                /* make the prototype consistent with the others even if not needed */
/* put a sequence of values in the results area so we can verify that the Dealer Input file syntax picks up the right ones */
/* prolog and analyze side have already been called,
 * so the global pointers and vars have been setup. p_uservals, and p_UEss point to the correct side
 */
   int nres = 0 ;
   int res_idx ;
   char lf = ' ';
   // HANDSTAT_k *p_hs;
   struct detailed_res_st *p_ures ;
   int side = p_ss->side;

   JGMDPRT( 7 , "test_calc done prolog for side= %d; compass[0]=%c, compass[1]=%c, phs[0]=%p, phs[1]=%p\n",
               side, compass[0],compass[1],(void *)phs[0], (void *)phs[1] ) ;

   JGMDPRT( 5 , "------- Testing Tag %d with dealnum=%d-------\n", 99 , prod_num );
   p_ures = &(p_uservals->u.dr) ;
   nres = make_test_evals( p_ures ) ;
   #ifdef JGMDBG
      if(jgmDebug >= 5 ) {
         fprintf(stderr,"%d Test Evals Created\n",nres);
         for (res_idx = 0 ; res_idx < nres ; res_idx++ ) {
            lf = ((res_idx+1) % 16 == 0 ) ? '\n' : ' ' ;
            fprintf(stderr, "%d=%d,%c",res_idx,p_uservals->u.res[res_idx], lf ) ;
         }
         fprintf(stderr, "\n");
      }
   #endif
  return nres ;
} /* end test calc */

/* Return 6 values for each of the 15 coded metrics. (Room for 21 metrics ) and also in slot 126 BestFit Suit, slot 127 BestFit fitlen
 * NT: Side_total NTpts, Horth/East NTpts, South/West NTpts
 * BestFit: Side total HLDFpts, Horth/East HLDFpts, South/West HLDFpts, 
 */ 
int set40_calc (UE_SIDESTAT_k *p_ss ) {
   int m ;
   int m_cnt = 0 ;
   #define MAX_METRICS 15
   struct EvalAll_res_st All_UE_res[MAX_METRICS] ; /* 15 sets of 6 ints. will memcpy this to the p_nsres or p_ewres area at the end. */
   size_t All_UE_res_sz = sizeof(All_UE_res) ; /* should be 360 = 15 * 6 * 4    */
   size_t All_UE_res_cnt = All_UE_res_sz/sizeof(int) ;  /* total number of results for all 15 metrics ; should be 90 */
   memset(All_UE_res, 0, All_UE_res_sz ) ;
   struct EvalAll_res_st ue40_res ;

   int side = p_ss->side ;
   int bf_suit   = p_ss->t_suit;
   int bf_fitlen = p_ss->t_fitlen ;          
   JGMDPRT(7, "All_UE_res_sz=%zd, All_UE_res_cnt=%zd side=%d Strain=%d Fitlen=%d\n",
               All_UE_res_sz, All_UE_res_cnt, side, bf_suit, bf_fitlen ) ;
                
   for (m=0 ; m < MAX_METRICS ; m++ ) {
       if (metric_enabled[m] == 1 ) {
         JGMDPRT(3," Set40 Metric=%d, Enabled=%d, name=%s\n",m,metric_enabled[m], cf_name[m] );
         (*p_cfunc[m])( p_ss ) ;                   
         All_UE_res[m].nt_pts_side    = UEv.nt_pts_side;
         All_UE_res[m].nt_pts_seat[0] = UEv.nt_pts_seat[0];
         All_UE_res[m].nt_pts_seat[1] = UEv.nt_pts_seat[1];
         All_UE_res[m].bf_pts_side    = UEv.hldf_pts_side;
         All_UE_res[m].bf_pts_seat[0] = UEv.hldf_pts_seat[0];
         All_UE_res[m].bf_pts_seat[1] = UEv.hldf_pts_seat[1];
         if ( ZARBAS == m || ZARADV == m ) { /* for zar points in this set use the scaled values in the first 4 slots of the misc area */
            All_UE_res[m].nt_pts_seat[0] = UEv.misc_pts[0] ;
            All_UE_res[m].nt_pts_seat[1] = UEv.misc_pts[1] ;
            All_UE_res[m].nt_pts_side    = UEv.misc_pts[0] + UEv.misc_pts[1]  ;
            All_UE_res[m].bf_pts_side    = UEv.misc_pts[2] + UEv.misc_pts[3]  ;
            All_UE_res[m].bf_pts_seat[0] = UEv.misc_pts[2];
            All_UE_res[m].bf_pts_seat[1] = UEv.misc_pts[3];
         } /* end zar pts if */
              
         m_cnt++;
         ue40_res = All_UE_res[m] ; 
         JGMDPRT(7,"%s_calc done ; NT: %d = %d + %d ; HLDF: %d = %d + %d\n",cf_name[m],
      ue40_res.nt_pts_side, ue40_res.nt_pts_seat[0], ue40_res.nt_pts_seat[1],
      ue40_res.bf_pts_side, ue40_res.bf_pts_seat[0], ue40_res.bf_pts_seat[1]) ;
      } /* end if metric enabled */
   } /* end for m 0 .. MAX_METRICS */

 
   // we now have 6 values for each of the 15 metrics in our array of structures. 90 of the 128 allowed
   // memcpy this entire array to the User Results area of 128 ints.
   memcpy(p_uservals, All_UE_res ,  All_UE_res_sz );
   p_uservals->u.res[126] = bf_suit ;
   p_uservals->u.res[127] = bf_fitlen ; /* return the BestFit strain and fitlen in a known location in case user needs them */
   DBGDO(7, show_user_res("Set40_calc", p_uservals, 0, All_UE_res_cnt ) ) ;  
   return m_cnt; /* the number of metrics evaluated in slots 0..89 */
}

/* Calc 6 values for each of the 8 Alt_HCP metrics;  the raw_vals*100 and the Pav_round vals
 *  8 sets of six values ... 
 *  
 */
extern char alt_HCP_names[][16];
extern int alt_HCP_calc( UE_SIDESTAT_k *p_ss, int metric ) ;
int set41_calc (UE_SIDESTAT_k *p_ss ) {
   struct EvalALT_res_st {
      int nt_pts_side;  /*  The float vals Pav Rounded to Ints */
      int nt_pts_seat[2];
      int raw_pts_side;  /* The float vals *100 converted to Ints */
      int raw_pts_seat[2];
   } ;
   int m , metric;
   int m_val_cnt = 0 ; 
   #define MAX_ALTCNTS 8
   struct EvalALT_res_st All_ALT_res[MAX_ALTCNTS] ; /* sets of 6 ints. will memcpy this to the p_nsres or p_ewres area at the end. */
   size_t All_ALT_res_sz = sizeof(All_ALT_res) ; 
   size_t All_ALT_res_cnt = All_ALT_res_sz/sizeof(int) ;  /* total number of results for all 8 metrics ; should be 48 */
   memset(All_ALT_res, 0, All_ALT_res_sz ) ;
   struct EvalALT_res_st ue41_res ;

   int side = p_ss->side ;
   JGMDPRT(3, "All_ALT_res_sz=%zd, All_ALT_res_cnt=%zd side=%d\n",
               All_ALT_res_sz, All_ALT_res_cnt, side ) ;
                
   for (m=0 ; m < MAX_ALTCNTS ; m++ ) {
         metric = 20 + m;
         JGMDPRT(3," Set41 Metric=%d, name=%s\n",metric, alt_HCP_names[m] );
         alt_HCP_calc( p_ss, metric ) ;
         All_ALT_res[m].nt_pts_side    = UEv.nt_pts_side;  /* nt pts are the Pav_round() ones */
         All_ALT_res[m].nt_pts_seat[0] = UEv.nt_pts_seat[0];
         All_ALT_res[m].nt_pts_seat[1] = UEv.nt_pts_seat[1];
         All_ALT_res[m].raw_pts_side    = UEv.hldf_pts_side; /* bf pts are the Raw*100 ones */
         All_ALT_res[m].raw_pts_seat[0] = UEv.hldf_pts_seat[0];
         All_ALT_res[m].raw_pts_seat[1] = UEv.hldf_pts_seat[1];
         m_val_cnt += 6 ; 
         ue41_res = All_ALT_res[m] ;
         showints(&All_ALT_res[m].nt_pts_side, NULL, 6 ) ; 
         JGMDPRT(3,"Curr_Val_cnt=%d, %s_calc done ; NT: %d = %d + %d ; RAWx100: %d = %d + %d\n",m_val_cnt, alt_HCP_names[m],
            ue41_res.nt_pts_side,  ue41_res.nt_pts_seat[0],  ue41_res.nt_pts_seat[1],
            ue41_res.raw_pts_side, ue41_res.raw_pts_seat[0], ue41_res.raw_pts_seat[1]) ;
   } /* end for m 0 .. MAX_ALTCNTS */
  
   // we now have 6 values for each of the 8 metrics in our array of structures. 48 of the 128 allowed
   // memcpy this entire array to the User Results area of 128 ints.
   memcpy(p_uservals, All_ALT_res ,  All_ALT_res_sz );
   DBGDO(3, show_user_res("Set41_calc", p_uservals, 0, (m_val_cnt-1) ) ) ;  
   return m_val_cnt; /* the number of metrics evaluated in slots 0..47 */
}

/* Raw write to the UserEval area in mmap. Tests Dealer to UserServer IP communication -- only uses first 64 values leaves gaps now */
/* side stuff -ve or 0 - 999 ; suits clubs=100+, diam=200+ etc. hand[0]= 1000+, hand[1] = 2000+ */
int make_test_evals(struct detailed_res_st *p_ures) {
   int idx, s, pts ;
   for (idx=0 ; idx < 32 ; idx++ ) { p_ures->side_tot[idx] = -(idx+1) ; } // side x 32
   pts = 0 ;
   for (s= 0 ; s<4 ; s++ ) {                                              // side-suitx4 x 8
      pts += 100 ;
      for (idx = 0 ; idx < 8 ; idx ++ ) {
         p_ures->side_by_suit[s][idx] = pts + idx ;
      }
   }
   for (idx = 0 ; idx < 16; idx++ ) {                                      // side-handx2 x 16
      p_ures->hand_tot[0][idx] = 1000 + idx ;
      p_ures->hand_tot[1][idx] = 2000 + idx ;
   }
   pts = 0 ;
   for (s = 0 ; s< 4 ; s++ ) {                                             // side-handx2-suitx4 x4                                           
      pts += 100 ;
      p_ures->hand_suit[0][s][0] = 1000 + pts;
      p_ures->hand_suit[0][s][1] = 1001 + pts;
      p_ures->hand_suit[0][s][2] = 1002 + pts;
      p_ures->hand_suit[0][s][3] = 1003 + pts;
      p_ures->hand_suit[1][s][0] = 2000 + pts;
      p_ures->hand_suit[1][s][1] = 2001 + pts;
      p_ures->hand_suit[1][s][2] = 2002 + pts;
      p_ures->hand_suit[1][s][3] = 2003 + pts;
   }
   return 64 ;
}

void showints( int *p_lo , int *p_hi, int count ) { // for showing arbitrary RAM areas with ints.
   int *pi = p_lo;
   int *ph = p_hi;
   int  i = 0;
   char ch ;  
   if (count > 0 ) ph = p_lo + (count - 1) ;
   while ( pi <= ph ) {
     if( ((i+1) % 10 ) == 0 ) ch = '\n';
     else ch = ',';    
     fprintf(stderr, "[%d]=%d%c",i,*pi, ch ) ;
     pi++ ; i++ ;
   }
   fprintf(stderr,"\n");
   return ;
} /* end show ints */











